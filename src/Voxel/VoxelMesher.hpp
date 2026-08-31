#pragma once

#include <memory>
#include <iostream>
#include <vector>
#include <glm/vec3.hpp>

namespace Voxel
{
    class Chunk;
    struct Vertex;
    class Mesh;

    class VoxelMesher
    {
    public:

        static std::unique_ptr<Mesh> build(
            const Chunk& chunk
        );

    private:

        static void addFace(
            std::vector<Vertex>& vertices,
            std::vector<unsigned int>& indices,

            const glm::vec3& v0,
            const glm::vec3& v1,
            const glm::vec3& v2,
            const glm::vec3& v3,

            const glm::vec3& normal
        );
    };
}
