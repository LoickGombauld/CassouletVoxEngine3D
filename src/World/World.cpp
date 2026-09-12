#include <glm/gtc/matrix_transform.hpp>
#include "../World/World.hpp"
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

        m_chunks[makeChunkKey(chunkX, chunkZ)]->rebuildMesh(
            *this
        );
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
        constexpr int RADIUS = 2;

        for (
            int x = -RADIUS;
            x <= RADIUS;
            ++x
            )
        {
            for (
                int z = -RADIUS;
                z <= RADIUS;
                ++z
                )
            {
                auto chunk =
                    std::make_unique<Chunk>(
                        x,
                        z
                    );

                chunk->generateTestTerrain(*this);

                const long long key =
                    makeChunkKey(
                        x,
                        z
                    );

                m_chunks.emplace(
                    key,
                    std::move(chunk)
                );
            }
        }

        // Maintenant que TOUS les chunks existent,
        // nous pouvons construire leurs meshes.
        for (auto& [key, chunk] : m_chunks)
        {
            chunk->rebuildMesh(*this);
        }
    }

    void World::render(
        const glm::mat4& view,
        const glm::mat4& projection,
        Shader& shader
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

        for (auto& [key, chunk] : m_chunks)
        {
            const glm::mat4 model =
                glm::translate(
                    glm::mat4(1.0f),
                    chunk->getWorldPosition()
                );

            shader.setMat4(
                "u_Model",
                model
            );

            chunk->render();
        }
    }
}
