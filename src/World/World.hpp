#pragma once

#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <deque>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <vector>
#include <future>
#include <atomic>
#include <chrono>
#include <glm/ext/matrix_float4x4.hpp>
#include <glm/vec3.hpp>
#include "../World/WorldGenerator.hpp"
#include "../World/WorldGenerationSettings.hpp"
#include "../Voxel/VoxelMesher.hpp"
#include "../Voxel/LodChunk.hpp"
namespace Voxel
{
	class Shader;
	class Chunk;



    class World
    {
    public:

        World(std::uint32_t seed);
        ~World();

        void generate();

        void updateStreaming(
            const glm::vec3& playerPosition
        );

        void generateChunk(
            int chunkX,
            int chunkZ
        );
        std::uint16_t getVoxel(
            int worldX,
            int worldY,
            int worldZ
        ) const;

        void setVoxel(
            int worldX,
            int worldY,
            int worldZ,
            std::uint16_t voxel
        );

        Chunk* getChunk(
            int chunkX,
            int chunkZ
        );

        const Chunk* getChunk(
            int chunkX,
            int chunkZ
        ) const;

        void rebuildChunk(
            int chunkX,
            int chunkZ
        );

        void rebuildChunkAndNeighbors(
            int chunkX,
            int chunkZ
        );

        void render(
            const glm::mat4& view,
            const glm::mat4& projection,
            Shader& shader,
            const glm::vec3& playerPosition
        );

        void updateLodStreaming(
            const glm::vec3& playerPosition
        );

        // Distance (en chunks) au-delà de laquelle les LodChunk sont
        // générés/rendus à la place des chunks complets. Permet de
        // réduire le coût CPU/GPU en abaissant cette distance.
        void setLodRenderDistance(
            int distance
        );

        int getLodRenderDistance() const
        {
            return m_lodRenderDistance;
        }

        // Nombre maximum de LodChunk rendus par frame (triés du plus
        // proche au plus loin). Une valeur négative signifie qu'aucune
        // limite n'est appliquée.
        void setMaxRenderedLodChunks(
            int count
        );

        int getMaxRenderedLodChunks() const
        {
            return m_maxRenderedLodChunks;
        }

    private:

        struct StreamingTimings
        {
            std::chrono::nanoseconds integration{};
            std::chrono::nanoseconds meshes{};
            int integratedChunks = 0;
        };

        static long long makeChunkKey(
            int x,
            int z
        );

        static int floorDiv(
            int value,
            int divisor
        );

        static int positiveModulo(
            int value,
            int divisor
        );

        void processCompletedChunks(
            StreamingTimings& timings
        );

        void processDirtyMeshes(
            StreamingTimings& timings
        );

        void queueMeshRebuild(
            int chunkX,
            int chunkZ
        );

        void queueChunkGeneration(
            int chunkX,
            int chunkZ
        );

        void generationWorker();

        void queueLodChunkGeneration(
            int chunkX,
            int chunkZ
        );

        void processCompletedLodChunks();

        void processDirtyLodMeshes();

        void lodGenerationWorker();

    private:

        std::unordered_map<
            long long,
            std::unique_ptr<Chunk>
        > m_chunks;
        std::unique_ptr<WorldGenerator> m_generator;

        std::mutex m_generationMutex;
        std::mutex m_meshWorldMutex;
        std::condition_variable m_generationCondition;
        std::deque<std::pair<int, int>> m_generationQueue;
        std::deque<std::unique_ptr<Chunk>> m_completedChunks;
        std::unordered_set<long long> m_pendingChunks;
        std::deque<std::pair<int, int>> m_dirtyMeshQueue;
        std::unordered_set<long long> m_dirtyMeshKeys;
        std::future<VoxelMesher::MeshData> m_meshFuture;
        int m_meshJobChunkX = 0;
        int m_meshJobChunkZ = 0;
        std::vector<std::thread> m_generationWorkers;
        bool m_stopGeneration = false;
        std::atomic<long long> m_generationNanoseconds{ 0 };
        std::atomic<long long> m_generationPeakNanoseconds{ 0 };
        std::atomic<int> m_generatedChunks{ 0 };

        // File de génération LOD, séparée de la file des chunks complets.
        std::unordered_map<
            long long,
            std::unique_ptr<LodChunk>
        > m_lodChunks;

        std::mutex m_lodGenerationMutex;
        std::mutex m_lodMeshMutex;
        std::condition_variable m_lodGenerationCondition;
        std::deque<std::pair<int, int>> m_lodGenerationQueue;
        std::deque<std::unique_ptr<LodChunk>> m_completedLodChunks;
        std::unordered_set<long long> m_pendingLodChunks;
        std::deque<std::pair<int, int>> m_dirtyLodMeshQueue;
        std::unordered_set<long long> m_dirtyLodMeshKeys;
        std::future<LodChunk::LodMeshData> m_lodMeshFuture;
        int m_lodMeshJobChunkX = 0;
        int m_lodMeshJobChunkZ = 0;
        std::thread m_lodGenerationWorker;
        bool m_stopLodGeneration = false;

        // Dernier chunk du joueur pour lequel le scan complet des zones
        // de streaming (chunks complets et LOD) a été effectué. Évite de
        // refaire ce scan coûteux à chaque frame quand le joueur reste
        // dans le même chunk.
        bool m_hasStreamedChunk = false;
        int m_lastStreamedChunkX = 0;
        int m_lastStreamedChunkZ = 0;

        bool m_hasLodStreamedChunk = false;
        int m_lastLodStreamedChunkX = 0;
        int m_lastLodStreamedChunkZ = 0;

        // Options configurables à l'exécution pour gérer la distance et
        // le nombre de rendu des chunks non chargés (LodChunk).
        int m_lodRenderDistance = WorldGenerationSettings::LOD_END_DISTANCE;
        int m_maxRenderedLodChunks = -1;
    };
}