#pragma once

#include "../Voxel/Chunk.hpp"

#include <memory>
#include <unordered_map>
#include <glm/ext/matrix_float4x4.hpp>

namespace Voxel
{
	class Chunk;
	class Shader;

    class World
    {
    public:

        World();

        void generate();

        void render(
            const glm::mat4& view,
            const glm::mat4& projection,
            Shader& shader
        );

        Chunk* getChunk(
            int chunkX,
            int chunkZ
        );

    private:

        static long long makeChunkKey(
            int x,
            int z
        );

    private:

        std::unordered_map<
            long long,
            std::unique_ptr<Chunk>
        > m_chunks;
    };
}
