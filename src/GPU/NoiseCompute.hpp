#pragma once

#include <cstdint>
#include <vector>
#include <glm/vec2.hpp>

namespace Voxel
{
    class NoiseCompute
    {
    public:

        NoiseCompute(
            std::uint32_t seed
        );

        ~NoiseCompute();

        // Calcule une grille de hauteurs sur le GPU
        void computeHeightmap(
            int chunkX,
            int chunkZ,
            int sampleStep,
            std::vector<int>& output
        );

        // Calcule le bruit 3D des cavernes sur le GPU
        void computeCaveNoise(
            int chunkX,
            int chunkZ,
            std::vector<float>& output
        );

    private:

        std::uint32_t m_seed;
        unsigned int m_computeProgram;
        unsigned int m_caveProgram;

        unsigned int m_permutationTexture;
        unsigned int m_readSSBO;
        unsigned int m_writeSSBO;

        void initPermutationTexture();
        void initComputeShader();
        void initCaveShader();
    };
}
