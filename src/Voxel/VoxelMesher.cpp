#include "../Voxel/VoxelMesher.hpp"

#include "../Voxel/Chunk.hpp"
#include "../Renderer/Mesh.hpp"
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

namespace Voxel
{
    void VoxelMesher::addFace(
        std::vector<Vertex>& vertices,
        std::vector<unsigned int>& indices,

        const glm::vec3& v0,
        const glm::vec3& v1,
        const glm::vec3& v2,
        const glm::vec3& v3,

        const glm::vec3& normal
    )
    {
        const unsigned int start =
            static_cast<unsigned int>(
                vertices.size()
                );

        vertices.push_back({
            {
                v0.x,
                v0.y,
                v0.z
            },
            {
                normal.x,
                normal.y,
                normal.z
            },
            {
                0.0f,
                0.0f
            }
            });

        vertices.push_back({
            {
                v1.x,
                v1.y,
                v1.z
            },
            {
                normal.x,
                normal.y,
                normal.z
            },
            {
                1.0f,
                0.0f
            }
            });

        vertices.push_back({
            {
                v2.x,
                v2.y,
                v2.z
            },
            {
                normal.x,
                normal.y,
                normal.z
            },
            {
                1.0f,
                1.0f
            }
            });

        vertices.push_back({
            {
                v3.x,
                v3.y,
                v3.z
            },
            {
                normal.x,
                normal.y,
                normal.z
            },
            {
                0.0f,
                1.0f
            }
            });

        indices.push_back(start + 0);
        indices.push_back(start + 1);
        indices.push_back(start + 2);

        indices.push_back(start + 2);
        indices.push_back(start + 3);
        indices.push_back(start + 0);
    }

    std::unique_ptr<Mesh>
        VoxelMesher::build(
            const Chunk& chunk
        )
    {
        std::vector<Vertex> vertices;
        std::vector<unsigned int> indices;

        // Réserve approximativement de la mémoire
        // pour éviter de nombreuses reallocations.
        vertices.reserve(4096);
        indices.reserve(6144);

        for (
            int y = 0;
            y < Chunk::HEIGHT;
            ++y
            )
        {
            for (
                int z = 0;
                z < Chunk::DEPTH;
                ++z
                )
            {
                for (
                    int x = 0;
                    x < Chunk::WIDTH;
                    ++x
                    )
                {
                    const VoxelID voxel =
                        chunk.get(x, y, z);

                    if (isAir(voxel))
                        continue;

                    const float fx =
                        static_cast<float>(x);

                    const float fy =
                        static_cast<float>(y);

                    const float fz =
                        static_cast<float>(z);

                    // ----------------------------------------
                    // TOP
                    // ----------------------------------------

                    if (
                        chunk.get(
                            x,
                            y + 1,
                            z
                        ) == Block::Air
                        )
                    {
                        addFace(
                            vertices,
                            indices,

                            { fx,     fy + 1, fz },
                            { fx + 1, fy + 1, fz },
                            { fx + 1, fy + 1, fz + 1 },
                            { fx,     fy + 1, fz + 1 },

                            { 0.0f, 1.0f, 0.0f }
                        );
                    }

                    // ----------------------------------------
                    // BOTTOM
                    // ----------------------------------------

                    if (
                        chunk.get(
                            x,
                            y - 1,
                            z
                        ) == Block::Air
                        )
                    {
                        addFace(
                            vertices,
                            indices,

                            { fx,     fy, fz },
                            { fx,     fy, fz + 1 },
                            { fx + 1, fy, fz + 1 },
                            { fx + 1, fy, fz },

                            { 0.0f, -1.0f, 0.0f }
                        );
                    }

                    // ----------------------------------------
                    // FRONT
                    // ----------------------------------------

                    if (
                        chunk.get(
                            x,
                            y,
                            z + 1
                        ) == Block::Air
                        )
                    {
                        addFace(
                            vertices,
                            indices,

                            { fx,     fy,     fz + 1 },
                            { fx,     fy + 1, fz + 1 },
                            { fx + 1, fy + 1, fz + 1 },
                            { fx + 1, fy,     fz + 1 },

                            { 0.0f, 0.0f, 1.0f }
                        );
                    }

                    // ----------------------------------------
                    // BACK
                    // ----------------------------------------

                    if (
                        chunk.get(
                            x,
                            y,
                            z - 1
                        ) == Block::Air
                        )
                    {
                        addFace(
                            vertices,
                            indices,

                            { fx,     fy,     fz },
                            { fx + 1, fy,     fz },
                            { fx + 1, fy + 1, fz },
                            { fx,     fy + 1, fz },

                            { 0.0f, 0.0f, -1.0f }
                        );
                    }

                    // ----------------------------------------
                    // RIGHT
                    // ----------------------------------------

                    if (
                        chunk.get(
                            x + 1,
                            y,
                            z
                        ) == Block::Air
                        )
                    {
                        addFace(
                            vertices,
                            indices,

                            { fx + 1, fy,     fz },
                            { fx + 1, fy,     fz + 1 },
                            { fx + 1, fy + 1, fz + 1 },
                            { fx + 1, fy + 1, fz },

                            { 1.0f, 0.0f, 0.0f }
                        );
                    }

                    // ----------------------------------------
                    // LEFT
                    // ----------------------------------------

                    if (
                        chunk.get(
                            x - 1,
                            y,
                            z
                        ) == Block::Air
                        )
                    {
                        addFace(
                            vertices,
                            indices,

                            { fx, fy,     fz },
                            { fx, fy + 1, fz },
                            { fx, fy + 1, fz + 1 },
                            { fx, fy,     fz + 1 },

                            { -1.0f, 0.0f, 0.0f }
                        );
                    }
                }
            }
        }

        if (vertices.empty())
            return nullptr;

        return std::make_unique<Mesh>(
            vertices,
            indices
        );
    }
}