#include "NoiseCompute.hpp"
#include <glad/glad.h>
#include <array>
#include <cmath>
#include <numeric>
#include <random>
#include <iostream>
#include <filesystem>
#include "../Engine/Profiler.hpp"

namespace Voxel
{
    NoiseCompute::NoiseCompute(
        std::uint32_t seed
    )
        : m_seed(seed),
        m_computeProgram(0),
        m_caveProgram(0),
        m_permutationTexture(0),
        m_readSSBO(0),
        m_writeSSBO(0)
    {
        initPermutationTexture();
        initComputeShader();
        initCaveShader();
    }

    NoiseCompute::~NoiseCompute()
    {
        if (m_permutationTexture)
            glDeleteTextures(1, &m_permutationTexture);
        if (m_computeProgram)
            glDeleteProgram(m_computeProgram);
        if (m_caveProgram)
            glDeleteProgram(m_caveProgram);
        if (m_readSSBO)
            glDeleteBuffers(1, &m_readSSBO);
        if (m_writeSSBO)
            glDeleteBuffers(1, &m_writeSSBO);
    }

    void NoiseCompute::initPermutationTexture()
    {
        std::array<int, 256> values{};
        std::iota(values.begin(), values.end(), 0);
        std::mt19937 generator(m_seed);
        std::shuffle(values.begin(), values.end(), generator);

        glGenTextures(1, &m_permutationTexture);
        glBindTexture(GL_TEXTURE_1D, m_permutationTexture);
        glTexImage1D(
            GL_TEXTURE_1D,
            0,
            GL_R32I,
            256,
            0,
            GL_RED_INTEGER,
            GL_INT,
            values.data()
        );
        glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    }

    void NoiseCompute::initComputeShader()
    {
        const char* computeShaderSource = R"(
#version 430 core

layout(local_size_x = 16, local_size_y = 16) in;

uniform int uChunkX;
uniform int uChunkZ;
uniform int uSampleStep;

uniform isampler1D uPermutation;

layout(std430, binding = 0) buffer OutputBuffer
{
    int heights[];
};


// ============================================================
// Utilitaires
// ============================================================

float fade(float t)
{
    return t * t * t * (t * (t * 6.0f - 15.0f) + 10.0f);
}


float lerp(float a, float b, float t)
{
    return a + t * (b - a);
}


// ============================================================
// Permutation
// ============================================================

int permutation(int index)
{
    index = index & 255;

    return texelFetch(
        uPermutation,
        index,
        0
    ).r;
}


// ============================================================
// Gradient 2D
// ============================================================

float gradient2D(
    int hash,
    float x,
    float z
)
{
    int h = hash & 3;

    if (h == 0)
        return x + z;

    if (h == 1)
        return -x + z;

    if (h == 2)
        return x - z;

    return -x - z;
}


// ============================================================
// Perlin 2D
// ============================================================

float noise2D(
    float x,
    float z
)
{
    int x0 = int(floor(x));
    int z0 = int(floor(z));

    float xf = x - float(x0);
    float zf = z - float(z0);

    int xi = x0 & 255;
    int zi = z0 & 255;

    int aa = permutation(
        permutation(xi) + zi
    );

    int ab = permutation(
        permutation(xi) + zi + 1
    );

    int ba = permutation(
        permutation(xi + 1) + zi
    );

    int bb = permutation(
        permutation(xi + 1) + zi + 1
    );

    float u = fade(xf);
    float v = fade(zf);

    float x1 = lerp(
        gradient2D(
            aa,
            xf,
            zf
        ),
        gradient2D(
            ba,
            xf - 1.0f,
            zf
        ),
        u
    );

    float x2 = lerp(
        gradient2D(
            ab,
            xf,
            zf - 1.0f
        ),
        gradient2D(
            bb,
            xf - 1.0f,
            zf - 1.0f
        ),
        u
    );

    return lerp(
        x1,
        x2,
        v
    );
}


// ============================================================
// FBM / Fractal Brownian Motion
// ============================================================

float fractalNoise2D(
    float x,
    float z,
    int octaves,
    float persistence,
    float lacunarity
)
{
    float total = 0.0f;

    float amplitude = 1.0f;
    float frequency = 1.0f;

    float amplitudeSum = 0.0f;

    for (int i = 0; i < octaves; ++i)
    {
        total += noise2D(
            x * frequency,
            z * frequency
        ) * amplitude;

        amplitudeSum += amplitude;

        amplitude *= persistence;
        frequency *= lacunarity;
    }

    // Même principe de normalisation :
    // permet de conserver une plage stable
    // indépendamment du nombre d'octaves.
    if (amplitudeSum > 0.0f)
        total /= amplitudeSum;

    return total;
}


// ============================================================
// Compute
// ============================================================

void main()
{
    uvec2 coord =
        gl_GlobalInvocationID.xy;

    int x = int(coord.x);
    int z = int(coord.y);

    // Heightmap 16x16 actuelle.
    if (x >= 16 || z >= 16)
        return;


    float worldX =
        float(
            uChunkX * 16 +
            x * uSampleStep
        );

    float worldZ =
        float(
            uChunkZ * 16 +
            z * uSampleStep
        );


    // ========================================================
    // FBM
    // ========================================================

    float noise =
        fractalNoise2D(
            worldX * 0.01f,
            worldZ * 0.01f,
            5,
            0.5f,
            2.0f
        );


    // ========================================================
    // Conversion en hauteur
    // ========================================================

    int height =
        int(
            32.0f +
            noise * 20.0f
        );


    int outputIndex =
        x * 16 + z;


    if (outputIndex < heights.length())
    {
        heights[outputIndex] = height;
    }
}
)";

        unsigned int shader =
            glCreateShader(GL_COMPUTE_SHADER);

        glShaderSource(
            shader,
            1,
            &computeShaderSource,
            nullptr
        );

        glCompileShader(shader);


        int success;
        char infoLog[512];

        glGetShaderiv(
            shader,
            GL_COMPILE_STATUS,
            &success
        );

        if (!success)
        {
            glGetShaderInfoLog(
                shader,
                512,
                nullptr,
                infoLog
            );

            std::cerr
                << "Compute shader compilation failed: "
                << infoLog
                << std::endl;
        }


        m_computeProgram =
            glCreateProgram();

        glAttachShader(
            m_computeProgram,
            shader
        );

        glLinkProgram(
            m_computeProgram
        );


        glGetProgramiv(
            m_computeProgram,
            GL_LINK_STATUS,
            &success
        );

        if (!success)
        {
            glGetProgramInfoLog(
                m_computeProgram,
                512,
                nullptr,
                infoLog
            );

            std::cerr
                << "Compute program linking failed: "
                << infoLog
                << std::endl;
        }


        glDeleteShader(shader);
    }

    void NoiseCompute::initCaveShader()
    {
        // Placeholder pour cave shader
        m_caveProgram = m_computeProgram;
    }

    void NoiseCompute::computeHeightmap(
        int chunkX,
        int chunkZ,
        int sampleStep,
        std::vector<int>& output
    )
    {
        if (!m_computeProgram || output.empty())
            return;

        PROFILE_SCOPE("GPU Mesh Upload");

        // Créer ou réutiliser le SSBO
        if (!m_writeSSBO)
        {
            glGenBuffers(1, &m_writeSSBO);
        }

        size_t bufferSize = output.size() * sizeof(int);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_writeSSBO);
        glBufferData(GL_SHADER_STORAGE_BUFFER, bufferSize, nullptr, GL_DYNAMIC_COPY);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_writeSSBO);

        // Lancer le compute shader
        glUseProgram(m_computeProgram);
        glUniform1i(glGetUniformLocation(m_computeProgram, "uChunkX"), chunkX);
        glUniform1i(glGetUniformLocation(m_computeProgram, "uChunkZ"), chunkZ);
        glUniform1i(glGetUniformLocation(m_computeProgram, "uSampleStep"), sampleStep);
        glUniform1i(glGetUniformLocation(m_computeProgram, "uPermutation"), 0);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_1D, m_permutationTexture);

        glDispatchCompute((16 + 15) / 16, (16 + 15) / 16, 1);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

        // Lire le résultat
        glBindBuffer(GL_COPY_READ_BUFFER, m_writeSSBO);
        glGetBufferSubData(GL_COPY_READ_BUFFER, 0, bufferSize, output.data());

        std::cout << '\r' 
            << "GPU Heightmap [0] = "
            << output[0]
            << std::endl;
    }

    void NoiseCompute::computeCaveNoise(
        int chunkX,
        int chunkZ,
        std::vector<float>& output
    )
    {
        // Placeholder : à implémenter similairement
    }
}
