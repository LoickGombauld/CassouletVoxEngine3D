#include "NoiseCompute.hpp"
#include <glad/glad.h>
#include <array>
#include <cmath>
#include <numeric>
#include <random>
#include <iostream>

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

// Simplifié : utilise sin() à la place du bruit réel pour prototype
float pseudoNoise2D(float x, float z)
{
    return sin(x * 0.03f) * cos(z * 0.03f);
}

void main()
{
    uvec2 coord = gl_GlobalInvocationID.xy;
    int x = int(coord.x);
    int z = int(coord.y);

    float worldX = float(uChunkX * 16 + x * uSampleStep);
    float worldZ = float(uChunkZ * 16 + z * uSampleStep);

    float noise1 = pseudoNoise2D(worldX * 0.0015f, worldZ * 0.0015f) * 64.0f;
    float noise2 = pseudoNoise2D(worldX * 0.004f, worldZ * 0.004f) * 40.0f;

    int height = int(28.0f + noise1 + noise2);

    int outputIndex = x * 16 + z;
    if (outputIndex < heights.length())
        heights[outputIndex] = height;
}
)";

        unsigned int shader = glCreateShader(GL_COMPUTE_SHADER);
        glShaderSource(shader, 1, &computeShaderSource, nullptr);
        glCompileShader(shader);

        int success;
        char infoLog[512];
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(shader, 512, nullptr, infoLog);
            std::cerr << "Compute shader compilation failed: " << infoLog << std::endl;
        }

        m_computeProgram = glCreateProgram();
        glAttachShader(m_computeProgram, shader);
        glLinkProgram(m_computeProgram);

        glGetProgramiv(m_computeProgram, GL_LINK_STATUS, &success);
        if (!success)
        {
            glGetProgramInfoLog(m_computeProgram, 512, nullptr, infoLog);
            std::cerr << "Compute program linking failed: " << infoLog << std::endl;
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
