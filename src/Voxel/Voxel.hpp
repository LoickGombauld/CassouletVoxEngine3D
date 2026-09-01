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

    struct BlockInfo
    {
        int topTexture;
        int bottomTexture;
        int sideTexture;
    };
    inline const BlockInfo& getBlockInfo(
        VoxelID id
    )
    {
        static const BlockInfo air{
            -1,
            -1,
            -1
        };

        static const BlockInfo grass{
            0,
            1,
            2
        };

        static const BlockInfo dirt{
            1,
            1,
            1
        };

        static const BlockInfo stone{
            3,
            3,
            3
        };

        static const BlockInfo sand{
            4,
            4,
            4
        };

        static const BlockInfo water{
            5,
            5,
            5
        };

        switch (id)
        {
        case Block::Grass:
            return grass;

        case Block::Dirt:
            return dirt;

        case Block::Stone:
            return stone;

        case Block::Sand:
            return sand;

        case Block::Water:
            return water;

        default:
            return air;
        }
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