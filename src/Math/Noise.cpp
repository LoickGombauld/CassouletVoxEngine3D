#include "Noise.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <numeric>
#include <random>

namespace Voxel
{
    Noise::Noise(
        std::uint32_t seed
    )
        :
        m_seed(seed)
    {

        std::array<int, 256> values{};

        std::iota(
            values.begin(),
            values.end(),
            0
        );


        std::mt19937 generator(
            m_seed
        );

        std::shuffle(
            values.begin(),
            values.end(),
            generator
        );


        for (int i = 0; i < 512; ++i)
        {
            m_permutation[i] =
                values[i & 255];
        }
    }

    float Noise::gradient3D(
        int hash,
        float x,
        float y,
        float z
    )
    {
        switch (hash % 12)
        {
        case 0:  return  x + y;
        case 1:  return -x + y;
        case 2:  return  x - y;
        case 3:  return -x - y;
        case 4:  return  x + z;
        case 5:  return -x + z;
        case 6:  return  x - z;
        case 7:  return -x - z;
        case 8:  return  y + z;
        case 9:  return -y + z;
        case 10: return  y - z;
        default: return -y - z;
        }
    }


    float Noise::fade(
        float t
    )
    {
        return
            t * t * t *
            (
                t *
                (
                    t * 6.0f
                    - 15.0f
                    )
                + 10.0f
                );
    }


    float Noise::lerp(
        float a,
        float b,
        float t
    )
    {
        return a + t * (b - a);
    }


    float Noise::gradient(
        int hash,
        float x,
        float z
    )
    {
        switch (hash & 7)
        {
        case 0:
            return  x + z;

        case 1:
            return -x + z;

        case 2:
            return  x - z;

        case 3:
            return -x - z;

        case 4:
            return  x;

        case 5:
            return -x;

        case 6:
            return  z;

        default:
            return -z;
        }
    }


    float Noise::noise2D(
        float x,
        float z
    ) const
    {
        const int cellX =
            static_cast<int>(
                std::floor(x)
                );

        const int cellZ =
            static_cast<int>(
                std::floor(z)
                );


        const float localX =
            x - std::floor(x);

        const float localZ =
            z - std::floor(z);



        const float u =
            fade(localX);

        const float v =
            fade(localZ);

        const int x0 =
            cellX & 255;

        const int z0 =
            cellZ & 255;

        const int x1 =
            (cellX + 1) & 255;

        const int z1 =
            (cellZ + 1) & 255;


        const int aa =
            m_permutation[
                m_permutation[x0] + z0
            ];

        const int ab =
            m_permutation[
                m_permutation[x0] + z1
            ];

        const int ba =
            m_permutation[
                m_permutation[x1] + z0
            ];

        const int bb =
            m_permutation[
                m_permutation[x1] + z1
            ];

        const float gradientAA =
            gradient(
                aa,
                localX,
                localZ
            );

        const float gradientBA =
            gradient(
                ba,
                localX - 1.0f,
                localZ
            );

        const float gradientAB =
            gradient(
                ab,
                localX,
                localZ - 1.0f
            );

        const float gradientBB =
            gradient(
                bb,
                localX - 1.0f,
                localZ - 1.0f
            );


        const float bottom =
            lerp(
                gradientAA,
                gradientBA,
                u
            );

        const float top =
            lerp(
                gradientAB,
                gradientBB,
                u
            );

        const float result =
            lerp(
                bottom,
                top,
                v
            );

        return std::clamp(
            result,
            -1.0f,
            1.0f
        );
    }

    float Noise::noise3D(
        float x,
        float y,
        float z
    ) const
    {
        const int cellX = static_cast<int>(std::floor(x));
        const int cellY = static_cast<int>(std::floor(y));
        const int cellZ = static_cast<int>(std::floor(z));

        const float localX = x - std::floor(x);
        const float localY = y - std::floor(y);
        const float localZ = z - std::floor(z);

        const float u = fade(localX);
        const float v = fade(localY);
        const float w = fade(localZ);

        const int x0 = cellX & 255;
        const int y0 = cellY & 255;
        const int z0 = cellZ & 255;
        const int x1 = (cellX + 1) & 255;
        const int y1 = (cellY + 1) & 255;
        const int z1 = (cellZ + 1) & 255;

        const auto hash = [this](int px, int py, int pz)
        {
            return m_permutation[
                m_permutation[
                    m_permutation[px] + py
                ] + pz
            ];
        };

        const float c000 = gradient3D(hash(x0, y0, z0), localX, localY, localZ);
        const float c100 = gradient3D(hash(x1, y0, z0), localX - 1.0f, localY, localZ);
        const float c010 = gradient3D(hash(x0, y1, z0), localX, localY - 1.0f, localZ);
        const float c110 = gradient3D(hash(x1, y1, z0), localX - 1.0f, localY - 1.0f, localZ);
        const float c001 = gradient3D(hash(x0, y0, z1), localX, localY, localZ - 1.0f);
        const float c101 = gradient3D(hash(x1, y0, z1), localX - 1.0f, localY, localZ - 1.0f);
        const float c011 = gradient3D(hash(x0, y1, z1), localX, localY - 1.0f, localZ - 1.0f);
        const float c111 = gradient3D(hash(x1, y1, z1), localX - 1.0f, localY - 1.0f, localZ - 1.0f);

        const float x00 = lerp(c000, c100, u);
        const float x10 = lerp(c010, c110, u);
        const float x01 = lerp(c001, c101, u);
        const float x11 = lerp(c011, c111, u);
        const float y0Value = lerp(x00, x10, v);
        const float y1Value = lerp(x01, x11, v);

        return std::clamp(
            lerp(y0Value, y1Value, w),
            -1.0f,
            1.0f
        );
    }


    float Noise::fractalNoise2D(
        float x,
        float z,
        int octaves,
        float persistence,
        float lacunarity
    ) const
    {
        const auto start = std::chrono::steady_clock::now();

        if (octaves <= 0)
        {
            return 0.0f;
        }


        float total = 0.0f;

        float amplitude = 1.0f;

        float frequency = 1.0f;

        float amplitudeSum = 0.0f;


        for (
            int octave = 0;
            octave < octaves;
            ++octave
            )
        {
            total +=
                noise2D(
                    x * frequency,
                    z * frequency
                )
                * amplitude;


            amplitudeSum +=
                amplitude;


            amplitude *=
                persistence;

            frequency *=
                lacunarity;
        }

        if (amplitudeSum == 0.0f)
        {
            return 0.0f;
        }


        const float result = total / amplitudeSum;
        m_elapsedTime += std::chrono::steady_clock::now() - start;
        return result;
    }

    float Noise::fractalNoise3D(
        float x,
        float y,
        float z,
        int octaves,
        float persistence,
        float lacunarity
    ) const
    {
        const auto start = std::chrono::steady_clock::now();

        if (octaves <= 0)
        {
            return 0.0f;
        }

        float total = 0.0f;
        float amplitude = 1.0f;
        float frequency = 1.0f;
        float amplitudeSum = 0.0f;

        for (int octave = 0; octave < octaves; ++octave)
        {
            total += noise3D(
                x * frequency,
                y * frequency,
                z * frequency
            ) * amplitude;

            amplitudeSum += amplitude;
            amplitude *= persistence;
            frequency *= lacunarity;
        }

        if (amplitudeSum == 0.0f)
        {
            return 0.0f;
        }

        const float result = total / amplitudeSum;
        m_elapsedTime += std::chrono::steady_clock::now() - start;
        return result;
    }
}