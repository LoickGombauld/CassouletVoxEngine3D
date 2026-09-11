#pragma once

#include <memory>
#include <iostream>
#include <vector>
#include <glm/vec3.hpp>


namespace Voxel
{
    class Chunk;
	class Mesh;
	class Voxel;
    class World;
	struct Vertex;

    class VoxelMesher
    {

    public:

        struct Face
        {
            std::uint16_t voxel = 0;

            int normal = 0;

			int textureIndex = 0;

            int uAxis;
            int vAxis;
            int uSign;
            int vSign;

            std::uint8_t ao[4] = {
                3,
                3,
                3,
                3
            };

            bool valid() const
            {
                return voxel != 0;
            }
        };

        static std::unique_ptr<Mesh> build(
            const World& world,
            const Chunk& chunk
        );

    private:

        static void addQuad(
            std::vector<Vertex>& vertices,
            std::vector<unsigned int>& indices,

            const glm::vec3& v0,
            const glm::vec3& v1,
            const glm::vec3& v2,
            const glm::vec3& v3,

            const glm::vec3& normal,

            const Face& face,

            int width,
            int height,

            int faceIndex,

            int UWorld,
            int VWorld
        );

        static std::uint8_t calculateAO(
            const World& world,

            int x,
            int y,
            int z,

            int axis,
            int uAxis,
            int vAxis,

            int uSign,
            int vSign
        );

        static Face createFace(
            const World& world,

            int x,
            int y,
            int z,

            int axis,
            int normal
        );
    };
}