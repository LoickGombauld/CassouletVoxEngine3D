#pragma once

#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <deque>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <vector>
#include <atomic>
#include <chrono>
#include <glm/ext/matrix_float4x4.hpp>
#include <glm/vec3.hpp>
#include "../World/WorldGenerator.hpp"
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
            Shader& shader
        );

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

        void queueChunkGeneration(
            int chunkX,
            int chunkZ
        );

        void generationWorker();

    private:

        std::unordered_map<
            long long,
            std::unique_ptr<Chunk>
        > m_chunks;
        std::unique_ptr<WorldGenerator> m_generator;

        std::mutex m_generationMutex;
        std::condition_variable m_generationCondition;
        std::deque<std::pair<int, int>> m_generationQueue;
        std::deque<std::unique_ptr<Chunk>> m_completedChunks;
        std::unordered_set<long long> m_pendingChunks;
        std::vector<std::thread> m_generationWorkers;
        bool m_stopGeneration = false;
        std::atomic<long long> m_generationNanoseconds{ 0 };
        std::atomic<long long> m_generationPeakNanoseconds{ 0 };
        std::atomic<int> m_generatedChunks{ 0 };
    };
}