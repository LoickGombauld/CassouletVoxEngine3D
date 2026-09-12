#include "../World/WorldGenerator.hpp"
#include "../Voxel/Voxel.hpp"
#include "../Voxel/Chunk.hpp"
#include "../Math/Noise.hpp"

namespace Voxel {


    WorldGenerator::WorldGenerator(
        std::uint32_t seed
    )
        :
        m_seed(seed)
    {
    }

    void WorldGenerator::generateChunk(
        Chunk& chunk
    )
    {
        const int originX =
            chunk.getChunkX() *
            Chunk::WIDTH;

        const int originZ =
            chunk.getChunkZ() *
            Chunk::DEPTH;

        for (
            int x = 0;
            x < Chunk::WIDTH;
            ++x
            )
        {
            for (
                int z = 0;
                z < Chunk::DEPTH;
                ++z
                )
            {
                for (
                    int y = 0;
                    y < Chunk::HEIGHT;
                    ++y
                    )
                {
                    const int worldX =
                        originX + x;

                    const int worldZ =
                        originZ + z;



                    chunk.set(
                        x,
                        y,
                        z,
                        generateVoxel(
                            worldX,
                            y,
                            worldZ
                        )
                    );
                }
            }
        }
    }

    int WorldGenerator::getTerrainHeight(
        int worldX,
        int worldZ
    ) const
    {
        float noiseValue =
            fractalNoise(
                worldX * 0.005f,
                worldZ * 0.005f,
                5,
                0.5f,
                2.0f
            );

        float normalized =
            noiseValue * 0.5f + 0.5f;

        int height =
            static_cast<int>(
                20.0f +
                normalized * 50.0f
                );
        return height;
    }

    VoxelID WorldGenerator::generateVoxel(
        int worldX,
        int worldY,
        int worldZ
    ) const
    {
        const int height =
            getTerrainHeight(
                worldX,
                worldZ
            );

        if (worldY > height)
            return Block::Air;

        if (worldY == height)
            return Block::Grass;

        if (worldY > height - 4)
            return Block::Dirt;

        return Block::Stone;
    }
}