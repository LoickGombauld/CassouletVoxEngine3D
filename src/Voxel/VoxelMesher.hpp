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

        struct FaceBasis
        {
            int uAxis;
            int uSign;

            int vAxis;
            int vSign;
        };

       static FaceBasis getFaceBasis(int axis, int normal)
        {
            /*
                Convention :

                +X : U = +Z, V = +Y
                -X : U = -Z, V = +Y

                +Y : U = +X, V = +Z
                -Y : U = +X, V = -Z

                +Z : U = -X, V = +Y
                -Z : U = +X, V = +Y
            */

            if (axis == 0)
            {
                if (normal > 0)
                    return { 2, +1, 1, +1 };

                return { 2, -1, 1, +1 };
            }

            if (axis == 1)
            {
                if (normal > 0)
                    return { 0, +1, 2, +1 };

                return { 0, +1, 2, -1 };
            }

            if (normal > 0)
                return { 0, -1, 1, +1 };

            return { 0, +1, 1, +1 };
        }


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
            int height
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