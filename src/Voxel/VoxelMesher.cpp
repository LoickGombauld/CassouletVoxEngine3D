#include "../Voxel/VoxelMesher.hpp"

#include "../World/World.hpp"
#include "../Renderer/Mesh.hpp"
#include "../Voxel/Voxel.hpp"
#include "../Renderer/VertexArray.hpp"
#include "../Renderer/VertexBuffer.hpp"
#include "../Renderer/IndexBuffer.hpp"
#include "../Renderer/Shader.hpp"
#include "../Voxel/Chunk.hpp"

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

#include <algorithm>
#include <array>
#include <cstdint>
#include <vector>

namespace Voxel
{
    namespace
    {
        struct Axis
        {
            int x;
            int y;
            int z;
        };

        Axis makeAxis(
            int axis,
            int value
        )
        {
            Axis result{ 0, 0, 0 };

            if (axis == 0)
                result.x = value;

            if (axis == 1)
                result.y = value;

            if (axis == 2)
                result.z = value;

            return result;
        }

        glm::vec3 axisVector(
            int axis,
            float value
        )
        {
            if (axis == 0)
                return { value, 0.0f, 0.0f };

            if (axis == 1)
                return { 0.0f, value, 0.0f };

            return { 0.0f, 0.0f, value };
        }

        int getWorldAxisOffset(
            int axis,
            int chunkOriginX,
            int chunkOriginZ
        )
        {
            if (axis == 0)
                return chunkOriginX;

            if (axis == 2)
                return chunkOriginZ;

            return 0;
        }

        int getComponent(
            const Axis& value,
            int axis
        )
        {
            if (axis == 0)
                return value.x;

            if (axis == 1)
                return value.y;

            return value.z;
        }
        bool sameFace(
            const VoxelMesher::Face& a,
            const VoxelMesher::Face& b
        )
        {
            if (a.voxel != b.voxel)
                return false;

            if (a.normal != b.normal)
                return false;

            if (
                a.textureIndex !=
                b.textureIndex
                )
            {
                return false;
            }

            for (int i = 0; i < 4; ++i)
            {
                if (a.ao[i] != b.ao[i])
                    return false;
            }

            return true;
        }
        int getFaceIndex(
            const glm::vec3& normal
        )
        {
            if (normal.x > 0.0f)
                return 0;

            if (normal.x < 0.0f)
                return 1;

            if (normal.y > 0.0f)
                return 2;

            if (normal.y < 0.0f)
                return 3;

            if (normal.z > 0.0f)
                return 4;

            return 5;
        }
    }



    std::unique_ptr<Mesh>
        VoxelMesher::build( const World& world,const Chunk& chunk)

    {
        std::vector<Vertex> vertices;
        std::vector<unsigned int> indices;

        constexpr int CHUNK_WIDTH =
            Chunk::WIDTH;

        constexpr int CHUNK_HEIGHT =
            Chunk::HEIGHT;

        constexpr int CHUNK_DEPTH =
            Chunk::DEPTH;

        const int dimensions[3] = {
            CHUNK_WIDTH,
            CHUNK_HEIGHT,
            CHUNK_DEPTH
        };

        vertices.reserve(8192);
        indices.reserve(12288);

        const int originX =
            chunk.getChunkX() *
            Chunk::WIDTH;

        const int originZ =
            chunk.getChunkZ() *
            Chunk::DEPTH;

        /*
            Sweep sur les trois axes.

            d = 0 → X
            d = 1 → Y
            d = 2 → Z
        */

        for (int d = 0; d < 3; ++d)
        {
            const int u =
                (d + 1) % 3;

            const int v =
                (d + 2) % 3;

            const int du =
                dimensions[u];

            const int dv =
                dimensions[v];

            /*
                Le masque contient les faces
                visibles entre deux voxels.
            */

            std::vector<Face> mask(
                du * dv
            );

            for (
                int slice = -1;
                slice < dimensions[d];
                ++slice
                )
            {
                /*
                    Construction du masque.
                */

                for (int j = 0; j < dv; ++j)
                {
                    for (int i = 0; i < du; ++i)
                    {
                        int posA[3] = {
                            0,
                            0,
                            0
                        };

                        int posB[3] = {
                            0,
                            0,
                            0
                        };

                        posA[d] = slice;
                        posB[d] = slice + 1;

                        posA[u] = i;
                        posB[u] = i;

                        posA[v] = j;
                        posB[v] = j;

                        const int worldAX =
                            originX + posA[0];

                        const int worldAY =
                            posA[1];

                        const int worldAZ =
                            originZ + posA[2];

                        const int worldBX =
                            originX + posB[0];

                        const int worldBY =
                            posB[1];

                        const int worldBZ =
                            originZ + posB[2];

                        const VoxelID voxelA =
                            world.getVoxel(
                                worldAX,
                                worldAY,
                                worldAZ
                            );

                        const VoxelID voxelB =
                            world.getVoxel(
                                worldBX,
                                worldBY,
                                worldBZ
                            );

                        Face face;



                        if (
                            isSolid(voxelA) &&
                            isAir(voxelB)
                            )
                        {
                            face =
                                createFace(
                                    world,
                                    worldAX,
                                    worldAY,
                                    worldAZ,
                                    d,
                                    +1
                                );
                        }
                        else if (
                            isAir(voxelA) &&
                            isSolid(voxelB)
                            )
                        {
                            face =
                                createFace(
                                    world,
                                    worldBX,
                                    worldBY,
                                    worldBZ,
                                    d,
                                    -1
                                );
                        }

                        mask[
                            i + j * du
                        ] = face;
                    }
                }
                /*
    Greedy rectangle extraction.
*/

                for (int j = 0; j < dv; ++j)
                {
                    for (int i = 0; i < du; )
                    {
                        Face& current =
                            mask[
                                i + j * du
                            ];

                        if (!current.valid())
                        {
                            ++i;
                            continue;
                        }

                        /*
                            Trouve la largeur maximale.
                        */

                        int width = 1;

                        while (
                            i + width < du &&
                            sameFace(
                                current,
                                mask[
                                    i + width +
                                        j * du
                                ]
                            )
                            )
                        {
                            ++width;
                        }

                        /*
                            Trouve la hauteur maximale.
                        */

                        int height = 1;

                        bool stop = false;

                        while (
                            j + height < dv &&
                            !stop
                            )
                        {
                            for (
                                int k = 0;
                                k < width;
                                ++k
                                )
                            {
                                if (
                                    !sameFace(
                                        current,
                                        mask[
                                            i + k +
                                                (j + height) * du
                                        ]
                                    )
                                    )
                                {
                                    stop = true;
                                    break;
                                }
                            }

                            if (!stop)
                                ++height;
                        }                    /*
                        Position de départ.
                    */

                        int p[3] = {
                            0,
                            0,
                            0
                        };

                        p[d] = slice + 1;

                        p[u] = i;
                        p[v] = j;

                        /*
                            Deux vecteurs correspondant
                            aux dimensions du rectangle.
                        */

                        glm::vec3 origin(
                            static_cast<float>(p[0]),
                            static_cast<float>(p[1]),
                            static_cast<float>(p[2])
                        );

                        glm::vec3 duVector =
                            axisVector(
                                u,
                                static_cast<float>(width)
                            );

                        glm::vec3 dvVector =
                            axisVector(
                                v,
                                static_cast<float>(height)
                            );

                        glm::vec3 normal =
                            axisVector(
                                d,
                                static_cast<float>(current.normal)
                            );

                        glm::vec3 v0 = origin;
                        glm::vec3 v1 = origin + duVector;
                        glm::vec3 v2 = origin + duVector + dvVector;
                        glm::vec3 v3 = origin + dvVector;
                        const int worldU =
                            getWorldAxisOffset(
                                u,
                                originX,
                                originZ
                            ) + i;

                        const int worldV =
                            getWorldAxisOffset(
                                v,
                                originX,
                                originZ
                            ) + j;
                        addQuad(
                            vertices,
                            indices,

                            v0,
                            v1,
                            v2,
                            v3,

                            normal,

                            current,

                            width,
                            height,
							getFaceIndex(normal),
                            worldU,
							worldV
                        );

                        /*
                            Supprime le rectangle
                            du masque.
                        */

                        for (int y = 0; y < height; ++y)
                        {
                            for (int x = 0; x < width; ++x)
                            {
                                mask[
                                    i + x +
                                        (j + y) * du
                                ] = Face{};
                            }
                        }

                        i += width;
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


//

void VoxelMesher::addQuad(
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
    int worldUOffset,
    int worldVOffset
)
{
    const unsigned int start =
        static_cast<unsigned int>(
            vertices.size()
            );

    const float w =
        static_cast<float>(width);

    const float h =
        static_cast<float>(height);

    const float textureIndex =
        static_cast<float>(
            face.textureIndex
            );

    const UVOrientation orientation =
        UV_ORIENTATIONS[faceIndex];

    /*
        Coordonnées du quad dans le repère
        géométrique utilisé par le Greedy Mesher.
    */

    float aU = static_cast<float>(worldUOffset);
    float bU = static_cast<float>(
            worldUOffset + width
            );

    float aV = static_cast<float>(worldVOffset);
    float bV = static_cast<float>(
        worldVOffset + height
        );;

    /*
        Permutation U <-> V.
    */

    float uv00;
    float uv10;
    float uv01;
    float uv11;

    if (orientation.swapUV)
    {
        uv00 = aV;
        uv10 = bV;

        uv01 = aU;
        uv11 = bU;
    }
    else
    {
        uv00 = aU;
        uv10 = bU;

        uv01 = aV;
        uv11 = bV;
    }

    /*
        Flip U.
    */

    if (orientation.flipU)
    {
        std::swap(uv00, uv10);
        std::swap(uv01, uv11);
    }

    /*
        Flip V.
    */

    if (orientation.flipV)
    {
        std::swap(uv00, uv01);
        std::swap(uv10, uv11);
    }

    vertices.push_back({
        v0,
        normal,
        { uv00, uv01 },
        static_cast<float>(
            face.ao[0]
        ) / 3.0f,
        textureIndex
        });

    vertices.push_back({
        v1,
        normal,
        { uv10, uv01 },
        static_cast<float>(
            face.ao[1]
        ) / 3.0f,
        textureIndex
        });

    vertices.push_back({
        v2,
        normal,
        { uv10, uv11 },
        static_cast<float>(
            face.ao[2]
        ) / 3.0f,
        textureIndex
        });

    vertices.push_back({
        v3,
        normal,
        { uv00, uv11 },
        static_cast<float>(
            face.ao[3]
        ) / 3.0f,
        textureIndex
        });

    const bool negativeFace =
        (faceIndex % 2) == 1;

    if (face.ao[0] + face.ao[2] >
        face.ao[1] + face.ao[3])
    {
        if (!negativeFace)
        {
            indices.push_back(start + 0);
            indices.push_back(start + 1);
            indices.push_back(start + 3);

            indices.push_back(start + 1);
            indices.push_back(start + 2);
            indices.push_back(start + 3);
        }
        else
        {
            indices.push_back(start + 0);
            indices.push_back(start + 3);
            indices.push_back(start + 1);

            indices.push_back(start + 1);
            indices.push_back(start + 3);
            indices.push_back(start + 2);
        }
    }
    else
    {
        if (!negativeFace)
        {
            indices.push_back(start + 0);
            indices.push_back(start + 1);
            indices.push_back(start + 2);

            indices.push_back(start + 2);
            indices.push_back(start + 3);
            indices.push_back(start + 0);
        }
        else
        {
            indices.push_back(start + 0);
            indices.push_back(start + 2);
            indices.push_back(start + 1);

            indices.push_back(start + 2);
            indices.push_back(start + 0);
            indices.push_back(start + 3);
        }
    }

}
    std::uint8_t
    VoxelMesher::calculateAO(
        const World& world,

        int x,
        int y,
        int z,

        int axis,
        int uAxis,
        int vAxis,

        int uSign,
        int vSign
    )
    {
        Axis side1Offset =
            makeAxis(
                uAxis,
                uSign
            );

        Axis side2Offset =
            makeAxis(
                vAxis,
                vSign
            );

        Axis cornerOffset{
            side1Offset.x + side2Offset.x,
            side1Offset.y + side2Offset.y,
            side1Offset.z + side2Offset.z
        };

        const bool side1 =
            isSolid(
                world.getVoxel(
                    x + side1Offset.x,
                    y + side1Offset.y,
                    z + side1Offset.z
                )
            );

        const bool side2 =
            isSolid(
                world.getVoxel(
                    x + side2Offset.x,
                    y + side2Offset.y,
                    z + side2Offset.z
                )
            );

        const bool corner =
            isSolid(
                world.getVoxel(
                    x + cornerOffset.x,
                    y + cornerOffset.y,
                    z + cornerOffset.z
                )
            );

        if (side1 && side2)
        {
            return 0;
        }

        return static_cast<std::uint8_t>(
            3 -
            static_cast<int>(side1) -
            static_cast<int>(side2) -
            static_cast<int>(corner)
        );
    }

    VoxelMesher::Face
    VoxelMesher::createFace(
        const World& world,

        int x,
        int y,
        int z,

        int axis,
        int normal
    )
    {
        Face face;

        face.voxel =
            world.getVoxel(
                x,
                y,
                z
            );

        face.normal = normal;


        const BlockInfo& info =
            getBlockInfo(
                face.voxel
            );
        const int faceIndex =
            axis * 2 + (normal < 0 ? 1 : 0);

        face.textureIndex =
            info.texture[faceIndex];

        const int uAxis =
            (axis + 1) % 3;

        const int vAxis =
            (axis + 2) % 3;

        face.ao[0] =
            calculateAO(
                world,
                x,
                y,
                z,
                axis,
                uAxis,
                vAxis,
                -1,
                -1
            );

        face.ao[1] =
            calculateAO(
                world,
                x,
                y,
                z,
                axis,
                uAxis,
                vAxis,
                +1,
                -1
            );

        face.ao[2] =
            calculateAO(
                world,
                x,
                y,
                z,
                axis,
                uAxis,
                vAxis,
                +1,
                +1
            );

        face.ao[3] =
            calculateAO(
                world,
                x,
                y,
                z,
                axis,
                uAxis,
                vAxis,
                -1,
                +1
            );

        return face;
    }
}