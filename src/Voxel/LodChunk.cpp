#include "../Voxel/LodChunk.hpp"
#include "../Voxel/Chunk.hpp"
#include "../World/WorldGenerator.hpp"
#include "../Renderer/Mesh.hpp"
#include <algorithm>
#include <glm/geometric.hpp>

namespace Voxel
{
    LodChunk::LodChunk(
        int chunkX,
        int chunkZ,
        int lodStep
    )
        : m_chunkX(chunkX),
        m_chunkZ(chunkZ),
        m_lodStep(lodStep),
        m_heights(GRID_SIZE * GRID_SIZE, 0.0f)
    {
    }

    void LodChunk::generateHeights(
        const WorldGenerator& generator
    )
    {
        const int originX = m_chunkX * Chunk::WIDTH;
        const int originZ = m_chunkZ * Chunk::DEPTH;

        // La grille couvre le chunk avec (GRID_SIZE - 1) segments,
        // chaque segment faisant m_lodStep blocs de large.
        const int sampleSpacing =
            (Chunk::WIDTH * m_lodStep) / (GRID_SIZE - 1);

        for (int sampleX = 0; sampleX < GRID_SIZE; ++sampleX)
        {
            for (int sampleZ = 0; sampleZ < GRID_SIZE; ++sampleZ)
            {
                const int worldX = originX + sampleX * sampleSpacing;
                const int worldZ = originZ + sampleZ * sampleSpacing;

                m_heights[sampleX * GRID_SIZE + sampleZ] =
                    static_cast<float>(
                        generator.getTerrainHeight(worldX, worldZ)
                    );
            }
        }
    }

    float LodChunk::getHeight(
        int sampleX,
        int sampleZ
    ) const
    {
        const int clampedX =
            std::max(0, std::min(GRID_SIZE - 1, sampleX));
        const int clampedZ =
            std::max(0, std::min(GRID_SIZE - 1, sampleZ));

        return m_heights[clampedX * GRID_SIZE + clampedZ];
    }

    LodChunk::LodMeshData LodChunk::buildMeshData() const
    {
        LodMeshData data;

        const int sampleSpacing =
            (Chunk::WIDTH * m_lodStep) / (GRID_SIZE - 1);

        data.vertices.reserve(GRID_SIZE * GRID_SIZE);

        for (int sampleX = 0; sampleX < GRID_SIZE; ++sampleX)
        {
            for (int sampleZ = 0; sampleZ < GRID_SIZE; ++sampleZ)
            {
                const float localX =
                    static_cast<float>(sampleX * sampleSpacing);
                const float localZ =
                    static_cast<float>(sampleZ * sampleSpacing);

                const float height = getHeight(sampleX, sampleZ);

                // Normale approchée par différences finies.
                const float heightRight =
                    getHeight(sampleX + 1, sampleZ);
                const float heightUp =
                    getHeight(sampleX, sampleZ + 1);

                const glm::vec3 dx(
                    static_cast<float>(sampleSpacing),
                    heightRight - height,
                    0.0f
                );

                const glm::vec3 dz(
                    0.0f,
                    heightUp - height,
                    static_cast<float>(sampleSpacing)
                );

                const glm::vec3 normal =
                    glm::normalize(glm::cross(dz, dx));

                data.vertices.push_back(
                    LodVertex{
                        glm::vec3(localX, height, localZ),
                        normal
                    }
                );
            }
        }

        data.indices.reserve(
            (GRID_SIZE - 1) * (GRID_SIZE - 1) * 6
        );

        for (int x = 0; x < GRID_SIZE - 1; ++x)
        {
            for (int z = 0; z < GRID_SIZE - 1; ++z)
            {
                const unsigned int topLeft =
                    static_cast<unsigned int>(x * GRID_SIZE + z);
                const unsigned int topRight =
                    static_cast<unsigned int>((x + 1) * GRID_SIZE + z);
                const unsigned int bottomLeft =
                    static_cast<unsigned int>(x * GRID_SIZE + z + 1);
                const unsigned int bottomRight =
                    static_cast<unsigned int>((x + 1) * GRID_SIZE + z + 1);

                data.indices.push_back(topLeft);
                data.indices.push_back(bottomLeft);
                data.indices.push_back(topRight);

                data.indices.push_back(topRight);
                data.indices.push_back(bottomLeft);
                data.indices.push_back(bottomRight);
            }
        }

        return data;
    }

    void LodChunk::setMesh(
        std::unique_ptr<Mesh> mesh
    )
    {
        m_mesh = std::move(mesh);
    }
}
