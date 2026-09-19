#include <glm/gtc/matrix_transform.hpp>
#include "../World/World.hpp"
#include "../World/WorldGenerationSettings.hpp"
#include <chrono>
#include <iostream>
#include <algorithm>
#include <atomic>
#include <thread>
#include <vector>
#include <cmath>
#include "../Renderer/Shader.hpp"
#include "../Voxel/Chunk.hpp"
#include "../Renderer/Mesh.hpp"
#include "../Voxel/Voxel.hpp"
#include "../World/World.hpp"



namespace Voxel
{
    World::World(
        std::uint32_t seed
    )
    {
        m_generator =
            std::make_unique<WorldGenerator>(
                seed
            );

		m_generator->testNoiseCPUvsGPU();

        const unsigned int hardwareThreads =
            std::thread::hardware_concurrency();

        const unsigned int workerCount =
            std::max(
                1u,
                hardwareThreads > 2 ? hardwareThreads - 2 : 1u
            );

        m_generationWorkers.reserve(workerCount);

        for (unsigned int index = 0; index < workerCount; ++index)
        {
            m_generationWorkers.emplace_back(
                &World::generationWorker,
                this
            );
        }

        m_lodGenerationWorker = std::thread(
            &World::lodGenerationWorker,
            this
        );
    }

    World::~World()
    {
        {
            std::lock_guard<std::mutex> lock(m_generationMutex);
            m_stopGeneration = true;
        }

        m_generationCondition.notify_all();

        for (std::thread& worker : m_generationWorkers)
        {
            if (worker.joinable())
            {
                worker.join();
            }
        }

        {
            std::lock_guard<std::mutex> lock(m_lodGenerationMutex);
            m_stopLodGeneration = true;
        }

        m_lodGenerationCondition.notify_all();

        if (m_lodGenerationWorker.joinable())
        {
            m_lodGenerationWorker.join();
        }
    }
    
    void World::generateChunk(
        int chunkX,
        int chunkZ
    )
    {
        auto chunk =
            std::make_unique<Chunk>(
                chunkX,
                chunkZ
            );

        m_generator->generateChunk(
            *chunk
        );


		m_chunks[makeChunkKey(chunkX, chunkZ)] =
            std::move(chunk);

        rebuildChunkAndNeighbors(
            chunkX,
            chunkZ
        );
    }

    void World::updateStreaming(
        const glm::vec3& playerPosition
    )
    {
        const int playerWorldX =
            static_cast<int>(std::floor(playerPosition.x));

        const int playerWorldZ =
            static_cast<int>(std::floor(playerPosition.z));

        const int playerChunkX =
            floorDiv(playerWorldX, Chunk::WIDTH);

        const int playerChunkZ =
            floorDiv(playerWorldZ, Chunk::DEPTH);

        constexpr int LOAD_RADIUS =
            WorldGenerationSettings::SIMULATION_DISTANCE;

        constexpr int UNLOAD_RADIUS =
            WorldGenerationSettings::SIMULATION_DISTANCE +
            WorldGenerationSettings::STREAMING_UNLOAD_MARGIN;

        const auto isInsideZone = [](int offsetX, int offsetZ, int radius)
        {
            if constexpr (WorldGenerationSettings::USE_CIRCULAR_CHUNK_ZONE)
            {
                return offsetX * offsetX + offsetZ * offsetZ <=
                    radius * radius;
            }

            return std::abs(offsetX) <= radius &&
                std::abs(offsetZ) <= radius;
        };

        StreamingTimings timings;
        processCompletedChunks(timings);

        // Le scan complet des zones de chargement/déchargement (jusqu'à
        // (2*LOAD_RADIUS+1)² itérations) n'est utile que lorsque le
        // joueur change de chunk. Sans cette protection, cette fonction
        // tournerait à pleine charge à chaque frame pendant tout
        // déplacement, causant une chute de FPS continue.
        const bool playerChangedChunk =
            !m_hasStreamedChunk ||
            playerChunkX != m_lastStreamedChunkX ||
            playerChunkZ != m_lastStreamedChunkZ;

        std::vector<std::pair<int, int>> chunksToUnload;

        if (playerChangedChunk)
        {
            m_hasStreamedChunk = true;
            m_lastStreamedChunkX = playerChunkX;
            m_lastStreamedChunkZ = playerChunkZ;

            std::vector<std::pair<int, int>> chunksToQueue;

            for (int offsetX = -LOAD_RADIUS; offsetX <= LOAD_RADIUS; ++offsetX)
            {
                for (int offsetZ = -LOAD_RADIUS; offsetZ <= LOAD_RADIUS; ++offsetZ)
                {
                    const int chunkX = playerChunkX + offsetX;
                    const int chunkZ = playerChunkZ + offsetZ;

                    if (isInsideZone(offsetX, offsetZ, LOAD_RADIUS) &&
                        !getChunk(chunkX, chunkZ))
                    {
                        chunksToQueue.emplace_back(chunkX, chunkZ);
                    }
                }
            }

            std::sort(
                chunksToQueue.begin(),
                chunksToQueue.end(),
                [playerChunkX, playerChunkZ](const auto& a, const auto& b)
                {
                    const int distanceA =
                        std::abs(a.first - playerChunkX) +
                        std::abs(a.second - playerChunkZ);

                    const int distanceB =
                        std::abs(b.first - playerChunkX) +
                        std::abs(b.second - playerChunkZ);

                    return distanceA < distanceB;
                }
            );

            for (const auto& [chunkX, chunkZ] : chunksToQueue)
            {
                queueChunkGeneration(chunkX, chunkZ);
            }

            for (const auto& [key, chunk] : m_chunks)
            {
                const int chunkX =
                    static_cast<int>(key >> 32);

                const int chunkZ =
                    static_cast<int>(static_cast<unsigned int>(key));

                if (!isInsideZone(
                        chunkX - playerChunkX,
                        chunkZ - playerChunkZ,
                        UNLOAD_RADIUS
                    ))
                {
                    chunksToUnload.emplace_back(chunkX, chunkZ);
                }
            }

            for (const auto& [chunkX, chunkZ] : chunksToUnload)
            {
                {
                    std::lock_guard<std::mutex> lock(m_meshWorldMutex);
                    m_chunks.erase(makeChunkKey(chunkX, chunkZ));
                }

                queueMeshRebuild(chunkX - 1, chunkZ);
                queueMeshRebuild(chunkX + 1, chunkZ);
                queueMeshRebuild(chunkX, chunkZ - 1);
                queueMeshRebuild(chunkX, chunkZ + 1);
            }
        }

        // Ces opérations restent bornées (quelques éléments par frame au
        // maximum) et doivent continuer à s'exécuter à chaque frame,
        // même si le joueur reste dans le même chunk.
        processDirtyMeshes(timings);

        const long long generatedNanoseconds =
            m_generationNanoseconds.exchange(0);
        const long long generationPeakNanoseconds =
            m_generationPeakNanoseconds.exchange(0);
        const int generatedChunks = m_generatedChunks.exchange(0);

        if (generatedChunks > 0 || timings.integratedChunks > 0 ||
            !chunksToUnload.empty())
        {
            const auto toMilliseconds = [](std::chrono::nanoseconds value)
            {
                return std::chrono::duration<double, std::milli>(value).count();
            };

            std::cout
                << "\nStreaming | generation workers: "
                << generatedChunks << " chunks, total "
                << toMilliseconds(std::chrono::nanoseconds(generatedNanoseconds))
                << " ms, pic "
                << toMilliseconds(std::chrono::nanoseconds(generationPeakNanoseconds))
                << " ms | integration: "
                << toMilliseconds(timings.integration)
                << " ms (" << timings.integratedChunks << " chunks)"
                << " | meshes/OpenGL: "
                << toMilliseconds(timings.meshes)
                << " ms\n";
        }
    }

    void World::queueChunkGeneration(
        int chunkX,
        int chunkZ
    )
    {
        const long long key = makeChunkKey(chunkX, chunkZ);

        std::lock_guard<std::mutex> lock(m_generationMutex);

        if (m_pendingChunks.contains(key))
        {
            return;
        }

        m_pendingChunks.insert(key);
        m_generationQueue.emplace_back(chunkX, chunkZ);
        m_generationCondition.notify_one();
    }

    void World::queueMeshRebuild(
        int chunkX,
        int chunkZ
    )
    {
        const long long key = makeChunkKey(chunkX, chunkZ);

            std::lock_guard<std::mutex> worldLock(m_meshWorldMutex);

            if (!m_chunks.contains(key))
        {
            return;
        }

        if (m_dirtyMeshKeys.insert(key).second)
        {
            m_dirtyMeshQueue.emplace_back(chunkX, chunkZ);
        }
    }

    void World::processDirtyMeshes(
        StreamingTimings& timings
    )
    {
        if (m_meshFuture.valid())
        {
            if (m_meshFuture.wait_for(std::chrono::seconds(0)) !=
                std::future_status::ready)
            {
                return;
            }

            const auto meshStart = std::chrono::steady_clock::now();
            VoxelMesher::MeshData data = m_meshFuture.get();

            Chunk* chunk = getChunk(
                m_meshJobChunkX,
                m_meshJobChunkZ
            );

            if (chunk && !data.empty())
            {
                chunk->setMesh(
                    std::make_unique<Mesh>(
                        data.vertices,
                        data.indices
                    )
                );
                ++timings.integratedChunks;
            }

            timings.meshes +=
                std::chrono::steady_clock::now() - meshStart;
        }

        if (!m_meshFuture.valid() && !m_dirtyMeshQueue.empty())
        {
            const auto [chunkX, chunkZ] = m_dirtyMeshQueue.front();
            m_dirtyMeshQueue.pop_front();
            m_dirtyMeshKeys.erase(makeChunkKey(chunkX, chunkZ));

            if (!getChunk(chunkX, chunkZ))
            {
                return;
            }

            m_meshJobChunkX = chunkX;
            m_meshJobChunkZ = chunkZ;
            m_meshFuture = std::async(
                std::launch::async,
                [this, chunkX, chunkZ]()
                {
                    std::lock_guard<std::mutex> lock(m_meshWorldMutex);
                    Chunk* chunk = getChunk(chunkX, chunkZ);

                    if (!chunk)
                    {
                        return VoxelMesher::MeshData{};
                    }

                    return VoxelMesher::buildData(*this, *chunk);
                }
            );
        }
    }

    void World::processCompletedChunks(
        StreamingTimings& timings
    )
    {
        for (int count = 0;
             count < WorldGenerationSettings::STREAMING_MAX_COMPLETIONS_PER_FRAME;
             ++count)
        {
            std::unique_ptr<Chunk> completedChunk;

            {
                std::lock_guard<std::mutex> lock(m_generationMutex);

                if (m_completedChunks.empty())
                {
                    break;
                }

                completedChunk = std::move(m_completedChunks.front());
                m_completedChunks.pop_front();
                m_pendingChunks.erase(
                    makeChunkKey(
                        completedChunk->getChunkX(),
                        completedChunk->getChunkZ()
                    )
                );
            }

            const int chunkX = completedChunk->getChunkX();
            const int chunkZ = completedChunk->getChunkZ();
            const long long key = makeChunkKey(chunkX, chunkZ);

            if (!m_chunks.contains(key))
            {
                const auto integrationStart = std::chrono::steady_clock::now();
                m_chunks.emplace(key, std::move(completedChunk));

                timings.integration +=
                    std::chrono::steady_clock::now() - integrationStart;
                ++timings.integratedChunks;

                queueMeshRebuild(chunkX, chunkZ);
                queueMeshRebuild(chunkX - 1, chunkZ);
                queueMeshRebuild(chunkX + 1, chunkZ);
                queueMeshRebuild(chunkX, chunkZ - 1);
                queueMeshRebuild(chunkX, chunkZ + 1);
            }
        }
    }

    void World::generationWorker()
    {
            WorldGenerator generator(m_generator->getSeed(), false);

        while (true)
        {
            std::pair<int, int> coordinates;

            {
                std::unique_lock<std::mutex> lock(m_generationMutex);

                m_generationCondition.wait(
                    lock,
                    [this]
                    {
                        return m_stopGeneration ||
                            !m_generationQueue.empty();
                    }
                );

                if (m_stopGeneration && m_generationQueue.empty())
                {
                    return;
                }

                coordinates = m_generationQueue.front();
                m_generationQueue.pop_front();
            }

            auto chunk = std::make_unique<Chunk>(
                coordinates.first,
                coordinates.second
            );

            const auto generationStart = std::chrono::steady_clock::now();
            generator.generateChunk(*chunk);
            const auto generationTime =
                std::chrono::steady_clock::now() - generationStart;

            const long long generationNanoseconds =
                std::chrono::duration_cast<std::chrono::nanoseconds>(
                    generationTime
                ).count();

            m_generationNanoseconds.fetch_add(generationNanoseconds);
            m_generatedChunks.fetch_add(1);

            long long previousPeak = m_generationPeakNanoseconds.load();
            while (previousPeak < generationNanoseconds &&
                   !m_generationPeakNanoseconds.compare_exchange_weak(
                       previousPeak,
                       generationNanoseconds
                   ))
            {
            }

            generator.resetTimings();

            {
                std::lock_guard<std::mutex> lock(m_generationMutex);
                m_completedChunks.emplace_back(std::move(chunk));
            }
        }
    }

    void World::queueLodChunkGeneration(
        int chunkX,
        int chunkZ
    )
    {
        const long long key = makeChunkKey(chunkX, chunkZ);

        std::lock_guard<std::mutex> lock(m_lodGenerationMutex);

        if (m_pendingLodChunks.contains(key))
        {
            return;
        }

        m_pendingLodChunks.insert(key);
        m_lodGenerationQueue.emplace_back(chunkX, chunkZ);
        m_lodGenerationCondition.notify_one();
    }

    void World::processCompletedLodChunks()
    {
        for (int count = 0;
             count < WorldGenerationSettings::STREAMING_MAX_COMPLETIONS_PER_FRAME;
             ++count)
        {
            std::unique_ptr<LodChunk> completedLodChunk;

            {
                std::lock_guard<std::mutex> lock(m_lodGenerationMutex);

                if (m_completedLodChunks.empty())
                {
                    break;
                }

                completedLodChunk = std::move(m_completedLodChunks.front());
                m_completedLodChunks.pop_front();
                m_pendingLodChunks.erase(
                    makeChunkKey(
                        completedLodChunk->getChunkX(),
                        completedLodChunk->getChunkZ()
                    )
                );
            }

            const int chunkX = completedLodChunk->getChunkX();
            const int chunkZ = completedLodChunk->getChunkZ();
            const long long key = makeChunkKey(chunkX, chunkZ);

            if (!m_lodChunks.contains(key))
            {
                m_lodChunks.emplace(key, std::move(completedLodChunk));

                std::lock_guard<std::mutex> meshLock(m_lodMeshMutex);

                if (m_dirtyLodMeshKeys.insert(key).second)
                {
                    m_dirtyLodMeshQueue.emplace_back(chunkX, chunkZ);
                }
            }
        }
    }

    void World::processDirtyLodMeshes()
    {
        if (m_lodMeshFuture.valid())
        {
            if (m_lodMeshFuture.wait_for(std::chrono::seconds(0)) !=
                std::future_status::ready)
            {
                return;
            }

            LodChunk::LodMeshData data = m_lodMeshFuture.get();

            const long long key =
                makeChunkKey(m_lodMeshJobChunkX, m_lodMeshJobChunkZ);

            auto iterator = m_lodChunks.find(key);

            if (iterator != m_lodChunks.end() && !data.empty())
            {
                std::vector<Vertex> vertices;
                vertices.reserve(data.vertices.size());

                for (const auto& lodVertex : data.vertices)
                {
                    Vertex vertex{};
                    vertex.position = lodVertex.position;
                    vertex.normal = lodVertex.normal;
                    vertex.uv = glm::vec2(0.0f, 0.0f);
                    vertex.ao = 1.0f;
                    vertex.textureIndex = 0.0f;

                    vertices.push_back(vertex);
                }

                iterator->second->setMesh(
                    std::make_unique<Mesh>(
                        vertices,
                        data.indices
                    )
                );
            }
        }

        if (!m_lodMeshFuture.valid() && !m_dirtyLodMeshQueue.empty())
        {
            const auto [chunkX, chunkZ] = m_dirtyLodMeshQueue.front();
            m_dirtyLodMeshQueue.pop_front();
            m_dirtyLodMeshKeys.erase(makeChunkKey(chunkX, chunkZ));

            const auto iterator =
                m_lodChunks.find(makeChunkKey(chunkX, chunkZ));

            if (iterator == m_lodChunks.end())
            {
                return;
            }

            m_lodMeshJobChunkX = chunkX;
            m_lodMeshJobChunkZ = chunkZ;

            LodChunk* lodChunk = iterator->second.get();

            m_lodMeshFuture = std::async(
                std::launch::async,
                [lodChunk]()
                {
                    return lodChunk->buildMeshData();
                }
            );
        }
    }

    void World::lodGenerationWorker()
    {
        WorldGenerator generator(m_generator->getSeed(), false);

        constexpr int LOD_STEP = 4;

        while (true)
        {
            std::pair<int, int> coordinates;

            {
                std::unique_lock<std::mutex> lock(m_lodGenerationMutex);

                m_lodGenerationCondition.wait(
                    lock,
                    [this]
                    {
                        return m_stopLodGeneration ||
                            !m_lodGenerationQueue.empty();
                    }
                );

                if (m_stopLodGeneration && m_lodGenerationQueue.empty())
                {
                    return;
                }

                coordinates = m_lodGenerationQueue.front();
                m_lodGenerationQueue.pop_front();
            }

            auto lodChunk = std::make_unique<LodChunk>(
                coordinates.first,
                coordinates.second,
                LOD_STEP
            );

            lodChunk->generateHeights(generator);

            {
                std::lock_guard<std::mutex> lock(m_lodGenerationMutex);
                m_completedLodChunks.emplace_back(std::move(lodChunk));
            }
        }
    }

    void World::updateLodStreaming(
        const glm::vec3& playerPosition
    )
    {
        const int playerWorldX =
            static_cast<int>(std::floor(playerPosition.x));

        const int playerWorldZ =
            static_cast<int>(std::floor(playerPosition.z));

        const int playerChunkX =
            floorDiv(playerWorldX, Chunk::WIDTH);

        const int playerChunkZ =
            floorDiv(playerWorldZ, Chunk::DEPTH);

        const int LOD_INNER_RADIUS =
            WorldGenerationSettings::LOD_START_DISTANCE -
            WorldGenerationSettings::LOD_FADE_MARGIN;

        const int LOD_OUTER_RADIUS = m_lodRenderDistance;

        processCompletedLodChunks();

        // Comme pour updateStreaming, le scan complet de la zone LOD
        // (jusqu'à (2*LOD_OUTER_RADIUS+1)² itérations, potentiellement
        // plusieurs milliers) ne doit s'exécuter que lorsque le joueur
        // change de chunk, pas à chaque frame.
        const bool playerChangedLodChunk =
            !m_hasLodStreamedChunk ||
            playerChunkX != m_lastLodStreamedChunkX ||
            playerChunkZ != m_lastLodStreamedChunkZ;

        if (playerChangedLodChunk)
        {
            m_hasLodStreamedChunk = true;
            m_lastLodStreamedChunkX = playerChunkX;
            m_lastLodStreamedChunkZ = playerChunkZ;

            for (int offsetX = -LOD_OUTER_RADIUS; offsetX <= LOD_OUTER_RADIUS; ++offsetX)
            {
                for (int offsetZ = -LOD_OUTER_RADIUS; offsetZ <= LOD_OUTER_RADIUS; ++offsetZ)
                {
                    const int distanceSquared = offsetX * offsetX + offsetZ * offsetZ;

                    if (distanceSquared > LOD_OUTER_RADIUS * LOD_OUTER_RADIUS ||
                        distanceSquared <= LOD_INNER_RADIUS * LOD_INNER_RADIUS)
                    {
                        continue;
                    }

                    const int chunkX = playerChunkX + offsetX;
                    const int chunkZ = playerChunkZ + offsetZ;

                    if (!m_lodChunks.contains(makeChunkKey(chunkX, chunkZ)))
                    {
                        queueLodChunkGeneration(chunkX, chunkZ);
                    }
                }
            }

            std::vector<std::pair<int, int>> lodChunksToUnload;

            for (const auto& [key, lodChunk] : m_lodChunks)
            {
                const int chunkX = static_cast<int>(key >> 32);
                const int chunkZ = static_cast<int>(static_cast<unsigned int>(key));

                const int offsetX = chunkX - playerChunkX;
                const int offsetZ = chunkZ - playerChunkZ;
                const int distanceSquared = offsetX * offsetX + offsetZ * offsetZ;

                if (distanceSquared > LOD_OUTER_RADIUS * LOD_OUTER_RADIUS ||
                    distanceSquared <= LOD_INNER_RADIUS * LOD_INNER_RADIUS)
                {
                    lodChunksToUnload.emplace_back(chunkX, chunkZ);
                }
            }

            for (const auto& [chunkX, chunkZ] : lodChunksToUnload)
            {
                m_lodChunks.erase(makeChunkKey(chunkX, chunkZ));
            }
        }

        processDirtyLodMeshes();
    }

    void World::setLodRenderDistance(
        int distance
    )
    {
        m_lodRenderDistance = std::max(0, distance);

        // Force un nouveau scan de streaming LOD pour appliquer
        // immédiatement la nouvelle distance.
        m_hasLodStreamedChunk = false;
    }

    void World::setMaxRenderedLodChunks(
        int count
    )
    {
        m_maxRenderedLodChunks = count;
    }

    long long World::makeChunkKey(
        int x,
        int z
    )
    {
        return
            (static_cast<long long>(x) << 32) ^
            static_cast<unsigned int>(z);
    }

    int World::floorDiv(
        int value,
        int divisor
    )
    {
        int result = value / divisor;
        int remainder = value % divisor;

        if (remainder != 0 && remainder < 0)
            --result;

        return result;
    }

    int World::positiveModulo(
        int value,
        int divisor
    )
    {
        int result = value % divisor;

        if (result < 0)
            result += divisor;

        return result;
    }

    Chunk* World::getChunk(
        int chunkX,
        int chunkZ
    )
    {
        const long long key =
            makeChunkKey(
                chunkX,
                chunkZ
            );

        auto iterator =
            m_chunks.find(key);

        if (iterator == m_chunks.end())
            return nullptr;

        return iterator->second.get();
    }

    const Chunk* World::getChunk(
        int chunkX,
        int chunkZ
    ) const
    {
        const long long key =
            makeChunkKey(
                chunkX,
                chunkZ
            );

        auto iterator =
            m_chunks.find(key);

        if (iterator == m_chunks.end())
            return nullptr;

        return iterator->second.get();
    }

    VoxelID World::getVoxel(
        int worldX,
        int worldY,
        int worldZ
    ) const
    {
        if (
            worldY < 0 ||
            worldY >= Chunk::HEIGHT
            )
        {
            return Block::Air;
        }

        const int chunkX =
            floorDiv(
                worldX,
                Chunk::WIDTH
            );

        const int chunkZ =
            floorDiv(
                worldZ,
                Chunk::DEPTH
            );

        const int localX =
            positiveModulo(
                worldX,
                Chunk::WIDTH
            );

        const int localZ =
            positiveModulo(
                worldZ,
                Chunk::DEPTH
            );

        const Chunk* chunk =
            getChunk(
                chunkX,
                chunkZ
            );

        if (!chunk)
            return Block::Air;

        return chunk->get(
            localX,
            worldY,
            localZ
        );
    }

    void World::setVoxel(
        int worldX,
        int worldY,
        int worldZ,
        std::uint16_t voxel
    )
    {
        if (
            worldY < 0 ||
            worldY >= Chunk::HEIGHT
            )
        {
            return;
        }

        const int chunkX =
            floorDiv(
                worldX,
                Chunk::WIDTH
            );

        const int chunkZ =
            floorDiv(
                worldZ,
                Chunk::DEPTH
            );

        const int localX =
            positiveModulo(
                worldX,
                Chunk::WIDTH
            );

        const int localZ =
            positiveModulo(
                worldZ,
                Chunk::DEPTH
            );

        Chunk* chunk =
            getChunk(
                chunkX,
                chunkZ
            );

        if (!chunk)
            return;

        chunk->set(
            localX,
            worldY,
            localZ,
            voxel
        );

        rebuildChunkAndNeighbors(
            chunkX,
            chunkZ
        );
    }
    void World::rebuildChunk(
        int chunkX,
        int chunkZ
    )
    {
        Chunk* chunk =
            getChunk(
                chunkX,
                chunkZ
            );

        if (!chunk)
            return;

        chunk->rebuildMesh(*this);
    }

    void World::rebuildChunkAndNeighbors(int chunkX,int chunkZ )
    {
        rebuildChunk(
            chunkX,
            chunkZ
        );

        rebuildChunk(
            chunkX - 1,
            chunkZ
        );

        rebuildChunk(
            chunkX + 1,
            chunkZ
        );

        rebuildChunk(
            chunkX,
            chunkZ - 1
        );

        rebuildChunk(
            chunkX,
            chunkZ + 1
        );
    }
    void World::generate()
    {
        constexpr int RADIUS = WorldGenerationSettings::WORLD_RADIUS;
        const auto start = std::chrono::steady_clock::now();

        struct PendingChunk
        {
            int x;
            int z;
            std::unique_ptr<Chunk> chunk;
            GenerationTimings timings;
        };

        std::vector<PendingChunk> pendingChunks;
        pendingChunks.reserve((2 * RADIUS + 1) * (2 * RADIUS + 1));

        for (int x = -RADIUS; x <= RADIUS; ++x)
        {
            for (int z = -RADIUS; z <= RADIUS; ++z)
            {
                pendingChunks.push_back({ x, z, nullptr, {} });
            }
        }

        std::atomic_size_t nextChunk = 0;
        const unsigned int hardwareThreads = std::thread::hardware_concurrency();
        const unsigned int workerCount = std::max(
            1u,
            std::min(
                hardwareThreads == 0 ? 1u : hardwareThreads,
                static_cast<unsigned int>(pendingChunks.size())
            )
        );
        std::vector<std::thread> workers;
        workers.reserve(workerCount);

        for (unsigned int worker = 0; worker < workerCount; ++worker)
        {
            workers.emplace_back([&]()
            {
                WorldGenerator generator(m_generator->getSeed(), false);

                while (true)
                {
                    const std::size_t index = nextChunk.fetch_add(1);
                    if (index >= pendingChunks.size())
                        break;

                    auto& pending = pendingChunks[index];
                    pending.chunk = std::make_unique<Chunk>(pending.x, pending.z);
                    generator.generateChunk(*pending.chunk);
                    pending.timings = generator.getTimings();
                    generator.resetTimings();
                }
            });
        }

        for (auto& worker : workers)
            worker.join();

        GenerationTimings timings;

        for (auto& pending : pendingChunks)
        {
            timings.voxel += pending.timings.voxel;
            timings.vegetation += pending.timings.vegetation;
            timings.noise += pending.timings.noise;

            m_chunks.emplace(
                makeChunkKey(pending.x, pending.z),
                std::move(pending.chunk)
            );
        }

        // Maintenant que TOUS les chunks existent,
        // nous pouvons construire leurs meshes.
        const auto meshStart = std::chrono::steady_clock::now();
        for (auto& [key, chunk] : m_chunks)
        {
            chunk->rebuildMesh(*this);
        }

        const auto toMilliseconds = [](std::chrono::nanoseconds value)
        {
            return std::chrono::duration_cast<std::chrono::milliseconds>(value).count();
        };
        std::cout
            << "World loading: "
            << std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::steady_clock::now() - start).count()
            << " ms (voxels: " << toMilliseconds(timings.voxel)
            << " ms, bruit: " << toMilliseconds(timings.noise)
            << " ms, vegetation: " << toMilliseconds(timings.vegetation)
            << " ms, meshes: " << toMilliseconds(std::chrono::steady_clock::now() - meshStart)
            << " ms)\n";
    }

    void World::render(
        const glm::mat4& view,
        const glm::mat4& projection,
        Shader& shader,
        const glm::vec3& playerPosition
    )
    {
        shader.setMat4(
            "u_View",
            view
        );

        shader.setMat4(
            "u_Projection",
            projection
        );

        const int playerChunkX = floorDiv(
            static_cast<int>(std::floor(playerPosition.x)),
            Chunk::WIDTH
        );
        const int playerChunkZ = floorDiv(
            static_cast<int>(std::floor(playerPosition.z)),
            Chunk::DEPTH
        );

        const auto isInsideRenderZone = [](int offsetX, int offsetZ)
        {
            // Les chunks complets sont rendus jusqu'à LOD_START_DISTANCE,
            // plus une marge de fondu pour se chevaucher légèrement avec
            // les LodChunk (transition en fondu croisé, pas de coupure nette).
            constexpr int FADE_RADIUS =
                WorldGenerationSettings::LOD_START_DISTANCE +
                WorldGenerationSettings::LOD_FADE_MARGIN;

            if constexpr (WorldGenerationSettings::USE_CIRCULAR_CHUNK_ZONE)
            {
                return offsetX * offsetX + offsetZ * offsetZ <=
                    FADE_RADIUS * FADE_RADIUS;
            }

            return std::abs(offsetX) <= FADE_RADIUS &&
                std::abs(offsetZ) <= FADE_RADIUS;
        };

        // Alpha de fondu pour un chunk complet : 1.0 avant la zone de
        // transition, puis décroît linéairement jusqu'à 0.0 à la limite
        // de fondu.
        const auto computeChunkFadeAlpha = [](int offsetX, int offsetZ)
        {
            const float distance = std::sqrt(
                static_cast<float>(offsetX * offsetX + offsetZ * offsetZ)
            );

            constexpr float fadeStart =
                static_cast<float>(WorldGenerationSettings::LOD_START_DISTANCE) -
                static_cast<float>(WorldGenerationSettings::LOD_FADE_MARGIN);

            constexpr float fadeEnd =
                static_cast<float>(WorldGenerationSettings::LOD_START_DISTANCE) +
                static_cast<float>(WorldGenerationSettings::LOD_FADE_MARGIN);

            if (distance <= fadeStart)
            {
                return 1.0f;
            }

            if (distance >= fadeEnd)
            {
                return 0.0f;
            }

            return 1.0f - (distance - fadeStart) / (fadeEnd - fadeStart);
        };

        for (auto& [key, chunk] : m_chunks)
        {
            const int chunkX = static_cast<int>(key >> 32);
            const int chunkZ = static_cast<int>(
                static_cast<unsigned int>(key)
            );

            const int offsetX = chunkX - playerChunkX;
            const int offsetZ = chunkZ - playerChunkZ;

            if (!isInsideRenderZone(offsetX, offsetZ))
            {
                continue;
            }

            const float fadeAlpha = computeChunkFadeAlpha(offsetX, offsetZ);

            if (fadeAlpha <= 0.0f)
            {
                continue;
            }

            const glm::mat4 model =
                glm::translate(
                    glm::mat4(1.0f),
                    chunk->getWorldPosition()
                );

            shader.setMat4(
                "u_Model",
                model
            );

            shader.setFloat(
                "u_FadeAlpha",
                fadeAlpha
            );

            chunk->render();
        }

        const int lodRenderDistance = m_lodRenderDistance;

        const auto isInsideLodZone = [lodRenderDistance](int offsetX, int offsetZ)
        {
            const int distanceSquared = offsetX * offsetX + offsetZ * offsetZ;

            const int LOD_INNER_RADIUS =
                WorldGenerationSettings::LOD_START_DISTANCE -
                WorldGenerationSettings::LOD_FADE_MARGIN;

            const int LOD_OUTER_RADIUS = lodRenderDistance;

            return distanceSquared > LOD_INNER_RADIUS * LOD_INNER_RADIUS &&
                distanceSquared <= LOD_OUTER_RADIUS * LOD_OUTER_RADIUS;
        };

        // Alpha de fondu pour un LodChunk : 0.0 avant la zone de
        // transition, puis croît linéairement jusqu'à 1.0.
        const auto computeLodFadeAlpha = [](int offsetX, int offsetZ)
        {
            const float distance = std::sqrt(
                static_cast<float>(offsetX * offsetX + offsetZ * offsetZ)
            );

            constexpr float fadeStart =
                static_cast<float>(WorldGenerationSettings::LOD_START_DISTANCE) -
                static_cast<float>(WorldGenerationSettings::LOD_FADE_MARGIN);

            constexpr float fadeEnd =
                static_cast<float>(WorldGenerationSettings::LOD_START_DISTANCE) +
                static_cast<float>(WorldGenerationSettings::LOD_FADE_MARGIN);

            if (distance <= fadeStart)
            {
                return 0.0f;
            }

            if (distance >= fadeEnd)
            {
                return 1.0f;
            }

            return (distance - fadeStart) / (fadeEnd - fadeStart);
        };

        struct VisibleLodChunk
        {
            int chunkX;
            int chunkZ;
            int distanceSquared;
            Mesh* mesh;
            float fadeAlpha;
        };

        std::vector<VisibleLodChunk> visibleLodChunks;
        visibleLodChunks.reserve(m_lodChunks.size());

        for (auto& [key, lodChunk] : m_lodChunks)
        {
            const int chunkX = static_cast<int>(key >> 32);
            const int chunkZ = static_cast<int>(
                static_cast<unsigned int>(key)
            );

            const int offsetX = chunkX - playerChunkX;
            const int offsetZ = chunkZ - playerChunkZ;

            if (!isInsideLodZone(offsetX, offsetZ))
            {
                continue;
            }

            Mesh* mesh = lodChunk->getMesh();

            if (!mesh)
            {
                continue;
            }

            const float fadeAlpha = computeLodFadeAlpha(offsetX, offsetZ);

            if (fadeAlpha <= 0.0f)
            {
                continue;
            }

            visibleLodChunks.push_back({
                chunkX,
                chunkZ,
                offsetX * offsetX + offsetZ * offsetZ,
                mesh,
                fadeAlpha
            });
        }

        // Limite le nombre de LodChunk rendus par frame (les plus
        // proches du joueur en priorité) pour maîtriser le coût GPU.
        if (m_maxRenderedLodChunks >= 0 &&
            static_cast<int>(visibleLodChunks.size()) > m_maxRenderedLodChunks)
        {
            std::partial_sort(
                visibleLodChunks.begin(),
                visibleLodChunks.begin() + m_maxRenderedLodChunks,
                visibleLodChunks.end(),
                [](const VisibleLodChunk& a, const VisibleLodChunk& b)
                {
                    return a.distanceSquared < b.distanceSquared;
                }
            );

            visibleLodChunks.resize(m_maxRenderedLodChunks);
        }

        for (const VisibleLodChunk& visible : visibleLodChunks)
        {
            const glm::mat4 model =
                glm::translate(
                    glm::mat4(1.0f),
                    glm::vec3(
                        static_cast<float>(visible.chunkX * Chunk::WIDTH),
                        0.0f,
                        static_cast<float>(visible.chunkZ * Chunk::DEPTH)
                    )
                );

            shader.setMat4(
                "u_Model",
                model
            );

            shader.setFloat(
                "u_FadeAlpha",
                visible.fadeAlpha
            );

            visible.mesh->draw();
        }

        // Réinitialise l'alpha de fondu pour les prochains appels de
        // rendu qui n'en tiennent pas compte.
        shader.setFloat(
            "u_FadeAlpha",
            1.0f
        );
    }
}
