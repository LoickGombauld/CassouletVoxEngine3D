#include "../World/WorldGenerator.hpp"
#include "../Voxel/Voxel.hpp"
#include "../Voxel/Chunk.hpp"
#include <glm/common.hpp>

namespace Voxel {


    WorldGenerator::WorldGenerator(
        std::uint32_t seed
    )
        :
        m_seed(seed),
		m_noise(std::make_unique<Noise>(seed))
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

    float WorldGenerator::getTemperature(
        int worldX,
        int worldZ
    ) const
    {
        const float temperature =
            m_noise->fractalNoise2D(
                static_cast<float>(worldX) * 0.0012f,
                static_cast<float>(worldZ) * 0.0012f,
                4,
                0.5f,
                2.0f
            );

        return temperature * 0.5f + 0.5f;
    }

    float WorldGenerator::getHumidity(
        int worldX,
        int worldZ
    ) const
    {
        constexpr float OFFSET_X = 1000.0f;
        constexpr float OFFSET_Z = 1000.0f;

        const float humidity =
            m_noise->fractalNoise2D(
                (static_cast<float>(worldX) + OFFSET_X) * 0.0015f,
                (static_cast<float>(worldZ) + OFFSET_Z) * 0.0015f,
                4,
                0.5f,
                2.0f
            );

        return humidity * 0.5f + 0.5f;
    }

    Biome WorldGenerator::getBiome(
        int worldX,
        int worldZ
    ) const
    {
        const float temperature =
            getTemperature(
                worldX,
                worldZ
            );

        const float humidity =
            getHumidity(
                worldX,
                worldZ
            );


        /*
         * Très froid
         */
        if (temperature < 0.38f)
        {
            return Biome::Taiga;
        }


        /*
         * Très chaud
         */
        if (temperature > 0.62f)
        {
            if (humidity < 0.48f)
            {
                return Biome::Desert;
            }

            return Biome::Savanna;
        }


        /*
         * Température intermédiaire
         */
        if (humidity > 0.55f)
        {
            return Biome::Forest;
        }

        return Biome::Plains;
    }

    int WorldGenerator::getTerrainHeight(
        int worldX,
        int worldZ
    ) const
    {
        /*
         * ------------------------------------------------------------
         * 1. GRANDES FORMES DU MONDE
         * ------------------------------------------------------------
         *
         * Très basse fréquence.
         *
         * Sert principalement à déterminer où se trouvent les
         * régions montagneuses.
         */
        const float continental =
            m_noise->fractalNoise2D(
                static_cast<float>(worldX) * 0.0015f,
                static_cast<float>(worldZ) * 0.0015f,
                4,
                0.5f,
                2.0f
            );

        /*
         * Conversion [-1, 1] -> [0, 1]
         */
        const float continental01 =
            continental * 0.5f + 0.5f;


        /*
         * ------------------------------------------------------------
         * 2. RELIEF PRINCIPAL
         * ------------------------------------------------------------
         *
         * Donne les collines et variations générales.
         */
        const float hills =
            m_noise->fractalNoise2D(
                static_cast<float>(worldX) * 0.004f,
                static_cast<float>(worldZ) * 0.004f,
                5,
                0.5f,
                2.0f
            );

        const float hills01 =
            hills * 0.5f + 0.5f;


        /*
         * ------------------------------------------------------------
         * 3. PETITS DETAILS
         * ------------------------------------------------------------
         *
         * Fréquence plus élevée.
         *
         * On garde une influence faible afin d'éviter un terrain
         * trop bruité.
         */
        const float detail =
            m_noise->fractalNoise2D(
                static_cast<float>(worldX) * 0.015f,
                static_cast<float>(worldZ) * 0.015f,
                3,
                0.5f,
                2.0f
            );

        const float detail01 =
            detail * 0.5f + 0.5f;


        /*
         * ------------------------------------------------------------
         * 4. MASQUE DE MONTAGNE
         * ------------------------------------------------------------
         *
         * Les montagnes n'apparaissent que dans certaines régions.
         *
         * smoothstep permet d'éviter une transition brutale entre
         * plaine et montagne.
         */
        const float mountainMask =
            glm::smoothstep(
                0.55f,
                0.75f,
                continental01
            );


        /*
         * ------------------------------------------------------------
         * 5. HAUTEUR DE BASE
         * ------------------------------------------------------------
         */

        constexpr float BASE_HEIGHT = 28.0f;

        /*
         * Variation générale des collines.
         */
        const float hillHeight =
            hills01 * 20.0f;

        /*
         * Petits détails.
         */
        const float detailHeight =
            detail01 * 4.0f;


        /*
         * ------------------------------------------------------------
         * 6. MONTAGNES
         * ------------------------------------------------------------
         *
         * On amplifie fortement le relief dans les régions
         * sélectionnées par mountainMask.
         */
        const float mountainHeight =
            mountainMask * continental01 * 45.0f;


        /*
         * ------------------------------------------------------------
         * 7. HAUTEUR FINALE
         * ------------------------------------------------------------
         */

        const float height =
            BASE_HEIGHT
            + hillHeight
            + detailHeight
            + mountainHeight;


        return static_cast<int>(height);
    }

    VoxelID WorldGenerator::generateVoxel(
        int worldX,
        int worldY,
        int worldZ
    ) const
    {
        const int terrainHeight =
            getTerrainHeight(
                worldX,
                worldZ
            );

        /*
         * Au-dessus du terrain
         */

        if (worldY > terrainHeight)
        {
            return Block::Air;
        }

        /*
         * Biome de cette colonne
         */

        const Biome biome =
            getBiome(
                worldX,
                worldZ
            );


        /*
         * ------------------------------------------------------------
         * DÉSERT
         * ------------------------------------------------------------
         */

        if (biome == Biome::Desert)
        {
            if (worldY >= terrainHeight - 3)
            {
                return Block::Sand;
            }

            return Block::Stone;
        }


        /*
         * ------------------------------------------------------------
         * SAVANE
         * ------------------------------------------------------------
         */

        if (biome == Biome::Savanna)
        {
            if (worldY == terrainHeight)
            {
                return Block::Grass;
            }

            if (worldY >= terrainHeight - 2)
            {
                return Block::Dirt;
            }

            return Block::Stone;
        }


        /*
         * ------------------------------------------------------------
         * FORÊT
         * ------------------------------------------------------------
         */

        if (biome == Biome::Forest)
        {
            if (worldY == terrainHeight)
            {
                return Block::Grass;
            }

            if (worldY >= terrainHeight - 3)
            {
                return Block::Dirt;
            }

            return Block::Stone;
        }


        /*
         * ------------------------------------------------------------
         * TAÏGA
         * ------------------------------------------------------------
         */

        if (biome == Biome::Taiga)
        {
            if (worldY == terrainHeight)
            {
                return Block::Grass;
            }

            if (worldY >= terrainHeight - 3)
            {
                return Block::Dirt;
            }

            return Block::Stone;
        }


        /*
         * ------------------------------------------------------------
         * PLAINES
         * ------------------------------------------------------------
         */

        if (worldY == terrainHeight)
        {
            return Block::Grass;
        }

        if (worldY >= terrainHeight - 3)
        {
            return Block::Dirt;
        }

        return Block::Stone;
    }
}