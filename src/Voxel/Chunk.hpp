#pragma once

#include <glm/vec3.hpp>
#include "../Voxel/Voxel.hpp"
#include "../Renderer/Mesh.hpp"
#include <memory>
#include <vector>

namespace Voxel
{
    class Chunk
    {
    public:

        static constexpr int WIDTH = 32;
        static constexpr int HEIGHT = 128;
        static constexpr int DEPTH = 32;

        static constexpr int VOLUME =
            WIDTH * HEIGHT * DEPTH;

    public:

        Chunk(
            int chunkX,
            int chunkZ
        );

        ~Chunk() = default;

        Chunk(const Chunk&) = delete;
        Chunk& operator=(const Chunk&) = delete;

        VoxelID get(
            int x,
            int y,
            int z
        ) const;

        void set(
            int x,
            int y,
            int z,
            VoxelID voxel
        );

        bool isInside(
            int x,
            int y,
            int z
        ) const;

        int getChunkX() const;
        int getChunkZ() const;

        glm::vec3 getWorldPosition() const;

        void rebuildMesh();

        void generateTestTerrain();

        void render() const;

    private:

        int getIndex(
            int x,
            int y,
            int z
        ) const;

    private:

        int m_chunkX;
        int m_chunkZ;

        std::vector<VoxelID> m_voxels;

        std::unique_ptr<Mesh> m_mesh;
    };
}
