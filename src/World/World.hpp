#pragma once

#include <memory>
#include <unordered_map>
#include <glm/ext/matrix_float4x4.hpp>

namespace Voxel
{
    class Shader;
	class Chunk;

    class World
    {
    public:

        World();

        void generate();

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

    private:

        std::unordered_map<
            long long,
            std::unique_ptr<Chunk>
        > m_chunks;
    };
}