#include "../World/WorldGenerator.hpp"
#include "../Voxel/Voxel.hpp"
#include "../Voxel/Chunk.hpp"
#include "../World/WorldGenerationSettings.hpp"
#include <glm/common.hpp>

namespace Voxel {

	namespace
	{
		std::uint32_t hashCoordinates(
			std::uint32_t seed,
			int x,
			int z
		)
		{
			std::uint32_t h = seed;

			h ^= static_cast<std::uint32_t>(x) + 0x9e3779b9u
				+ (h << 6)
				+ (h >> 2);

			h ^= static_cast<std::uint32_t>(z) + 0x9e3779b9u
				+ (h << 6)
				+ (h >> 2);

			h ^= h >> 16;
			h *= 0x85ebca6bu;
			h ^= h >> 13;
			h *= 0xc2b2ae35u;
			h ^= h >> 16;

			return h;
		}

		void setVoxelIfInsideChunk(
			Chunk& chunk,
			int worldX,
			int worldY,
			int worldZ,
			VoxelID voxel
		)
		{
			const int originX =
				chunk.getChunkX() * Chunk::WIDTH;

			const int originZ =
				chunk.getChunkZ() * Chunk::DEPTH;


			const int localX =
				worldX - originX;

			const int localZ =
				worldZ - originZ;


			if (
				localX < 0 ||
				localX >= Chunk::WIDTH ||
				localZ < 0 ||
				localZ >= Chunk::DEPTH
				)
			{
				return;
			}


			if (
				worldY < 0 ||
				worldY >= Chunk::HEIGHT
				)
			{
				return;
			}


			chunk.set(
				localX,
				worldY,
				localZ,
				voxel
			);
		}

		void setVoxelIfAir(
			Chunk& chunk,
			int worldX,
			int worldY,
			int worldZ,
			VoxelID voxel
		)
		{
			const int originX =
				chunk.getChunkX() * Chunk::WIDTH;

			const int originZ =
				chunk.getChunkZ() * Chunk::DEPTH;

			const int localX =
				worldX - originX;

			const int localZ =
				worldZ - originZ;


			if (
				localX < 0 ||
				localX >= Chunk::WIDTH ||
				localZ < 0 ||
				localZ >= Chunk::DEPTH ||
				worldY < 0 ||
				worldY >= Chunk::HEIGHT
				)
			{
				return;
			}


			if (
				chunk.get(
					localX,
					worldY,
					localZ
				) == Block::Air
				)
			{
				chunk.set(
					localX,
					worldY,
					localZ,
					voxel
				);
			}
		}
	}

	WorldGenerator::WorldGenerator(
		std::uint32_t seed
	)
		:
		m_seed(seed),
		m_noise(std::make_unique<Noise>(seed))
	{
	}

	void WorldGenerator::resetTimings()
	{
		m_voxelGenerationTime = std::chrono::nanoseconds::zero();
		m_vegetationGenerationTime = std::chrono::nanoseconds::zero();
		m_noise->resetElapsedTime();
	}

	GenerationTimings WorldGenerator::getTimings() const
	{
		return {
			m_voxelGenerationTime,
			m_vegetationGenerationTime,
			m_noise->getElapsedTime()
		};
	}

	void WorldGenerator::generateChunk(
		Chunk& chunk
	)
	{
     const auto voxelStart = std::chrono::steady_clock::now();
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
			/*
			 * ============================================================
			 * 2. VÉGÉTATION
			 * ============================================================
			 */

		}
     m_voxelGenerationTime += std::chrono::steady_clock::now() - voxelStart;

		const auto vegetationStart = std::chrono::steady_clock::now();
		generateVegetation(chunk);
		m_vegetationGenerationTime += std::chrono::steady_clock::now() - vegetationStart;
	}

	float WorldGenerator::getTemperature(
		int worldX,
		int worldZ
	) const
	{
		const float temperature =
			m_noise->fractalNoise2D(
               static_cast<float>(worldX) * WorldGenerationSettings::TEMPERATURE_FREQUENCY,
				static_cast<float>(worldZ) * WorldGenerationSettings::TEMPERATURE_FREQUENCY,
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
              (static_cast<float>(worldX) + OFFSET_X) * WorldGenerationSettings::HUMIDITY_FREQUENCY,
				(static_cast<float>(worldZ) + OFFSET_Z) * WorldGenerationSettings::HUMIDITY_FREQUENCY,
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

	void WorldGenerator::generateVegetation(
		Chunk& chunk
	)
	{
		const int originX =
			chunk.getChunkX() * Chunk::WIDTH;

		const int originZ =
			chunk.getChunkZ() * Chunk::DEPTH;


		/*
		 * Marge nécessaire pour les structures.
		 *
		 * Notre arbre peut s'étendre de 2 blocs autour
		 * de son origine.
		 */
       constexpr int MARGIN =
			WorldGenerationSettings::TREE_GENERATION_MARGIN;


		for (
			int worldX = originX - MARGIN;
			worldX < originX + Chunk::WIDTH + MARGIN;
			++worldX
			)
		{
			for (
				int worldZ = originZ - MARGIN;
				worldZ < originZ + Chunk::DEPTH + MARGIN;
				++worldZ
				)
			{
				if (
					shouldGenerateTree(
						worldX,
						worldZ
					)
					)
				{
					generateTree(
						chunk,
						worldX,
						worldZ
					);
				}
				if (
					shouldGenerateCactus(
						worldX,
						worldZ
					)
					)
				{
					generateCactus(
						chunk,
						worldX,
						worldZ
					);
				}
			}
		}
	}

	void WorldGenerator::generateTree(
		Chunk& chunk,
		int worldX,
		int worldZ
	)
	{
		const int groundY =
			getTerrainHeight(
				worldX,
				worldZ
			);

		const int trunkHeight =
         WorldGenerationSettings::TREE_MIN_HEIGHT +
			static_cast<int>(
				hashCoordinates(
					m_seed + 17u,
					worldX,
					worldZ
               ) % WorldGenerationSettings::TREE_HEIGHT_VARIATION
				);

		const int topY =
			groundY + trunkHeight;


		/*
		 * ------------------------------------------------------------
		 * TRONC
		 * ------------------------------------------------------------
		 */

		for (int y = 1; y <= trunkHeight; ++y)
		{
			setVoxelIfInsideChunk(
				chunk,
				worldX,
				groundY + y,
				worldZ,
				Block::Log
			);
		}


		/*
		 * ------------------------------------------------------------
		 * FEUILLAGE
		 * ------------------------------------------------------------
		 */

		for (int y = topY - 2; y <= topY + 1; ++y)
		{
			const int layer =
				y - (topY - 2);

			int radius = 2;

			if (layer == 0 || layer == 3)
			{
				radius = 1;
			}

			for (
				int dx = -radius;
				dx <= radius;
				++dx
				)
			{
				for (
					int dz = -radius;
					dz <= radius;
					++dz
					)
				{
					/*
					 * Évite les coins extrêmes.
					 */
					if (
						std::abs(dx) == radius &&
						std::abs(dz) == radius
						)
					{
						continue;
					}

					setVoxelIfAir(
						chunk,
						worldX + dx,
						y,
						worldZ + dz,
						Block::Leaves
					);
				}
			}
		}
	}

	void WorldGenerator::generateCactus(
		Chunk& chunk,
		int worldX,
		int worldZ
	)
	{
		const int groundY =
			getTerrainHeight(
				worldX,
				worldZ
			);


      const int height =
			WorldGenerationSettings::CACTUS_MIN_HEIGHT +
			static_cast<int>(
				hashCoordinates(
					m_seed + 100u,
					worldX,
					worldZ
               ) % WorldGenerationSettings::CACTUS_HEIGHT_VARIATION
				);


		for (
			int y = 1;
			y <= height;
			++y
			)
		{
			setVoxelIfInsideChunk(
				chunk,
				worldX,
				groundY + y,
				worldZ,
				Block::Cactus
			);
		}
	}

	bool WorldGenerator::shouldGenerateCactus(
		int worldX,
		int worldZ
	) const
	{
		if (
			getBiome(
				worldX,
				worldZ
			) != Biome::Desert
			)
		{
			return false;
		}


		const std::uint32_t hash =
			hashCoordinates(
				m_seed + 42u,
				worldX,
				worldZ
			);


		const float value =
			static_cast<float>(hash)
			/
			static_cast<float>(UINT32_MAX);


       return value < WorldGenerationSettings::DESERT_CACTUS_DENSITY;
	}

	bool WorldGenerator::shouldGenerateTree(
		int worldX,
		int worldZ
	) const
	{
		const Biome biome =
			getBiome(
				worldX,
				worldZ
			);

		const std::uint32_t hash =
			hashCoordinates(
				m_seed,
				worldX,
				worldZ
			);

		/*
		 * Valeur entre 0 et 1.
		 */
		const float value =
			static_cast<float>(
				hash
				) / static_cast<float>(
					UINT32_MAX
					);


		switch (biome)
		{
		case Biome::Forest:
           return value < WorldGenerationSettings::FOREST_TREE_DENSITY;

		case Biome::Plains:
          return value < WorldGenerationSettings::PLAINS_TREE_DENSITY;

		case Biome::Savanna:
          return value < WorldGenerationSettings::SAVANNA_TREE_DENSITY;

		default:
			return false;
		}
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

        constexpr float BASE_HEIGHT =
			WorldGenerationSettings::BASE_TERRAIN_HEIGHT;

		/*
		 * Variation générale des collines.
		 */
		const float hillHeight =
             hills01 * WorldGenerationSettings::HILL_HEIGHT;

		/*
		 * Petits détails.
		 */
		const float detailHeight =
             detail01 * WorldGenerationSettings::DETAIL_HEIGHT;


		/*
		 * ------------------------------------------------------------
		 * 6. MONTAGNES
		 * ------------------------------------------------------------
		 *
		 * On amplifie fortement le relief dans les régions
		 * sélectionnées par mountainMask.
		 */
		const float mountainHeight =
            mountainMask * continental01 *
			 WorldGenerationSettings::MOUNTAIN_HEIGHT;


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