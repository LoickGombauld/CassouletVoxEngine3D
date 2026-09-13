#pragma once

#include <array>
#include <cstdint>

namespace Voxel
{
    class Noise
    {
    public:

        explicit Noise(
            std::uint32_t seed
        );

        float noise2D(
            float x,
            float z
        ) const;

        float fractalNoise2D(
            float x,
            float z,
            int octaves = 5,
            float persistence = 0.5f,
            float lacunarity = 2.0f
        ) const;

        std::uint32_t getSeed() const
        {
            return m_seed;
        }

    private:

        std::uint32_t m_seed;

        std::array<int, 512> m_permutation;

        static float fade(
            float t
        );

        static float lerp(
            float a,
            float b,
            float t
        );

        static float gradient(
            int hash,
            float x,
            float z
        );
    };
}