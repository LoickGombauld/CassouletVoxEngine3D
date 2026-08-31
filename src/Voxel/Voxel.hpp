
#pragma once

#include <cstdint>

namespace Voxel
{
    using VoxelID = std::uint16_t;

    namespace Block
    {
        constexpr VoxelID Air = 0;
        constexpr VoxelID Grass = 1;
        constexpr VoxelID Dirt = 2;
        constexpr VoxelID Stone = 3;
        constexpr VoxelID Sand = 4;
        constexpr VoxelID Water = 5;
    }

    inline bool isAir(VoxelID voxel)
    {
        return voxel == Block::Air;
    }

    inline bool isSolid(VoxelID voxel)
    {
        return voxel != Block::Air &&
            voxel != Block::Water;
    }
}