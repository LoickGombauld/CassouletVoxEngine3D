#include "../World/WorldGenerator.hpp"
#include "../Voxel/Voxel.hpp"
#include "../Voxel/Chunk.hpp"
#include "../World/WorldGenerationSettings.hpp"
#include <algorithm>
#include <cmath>
#include <vector>
#include <glm/common.hpp>
#include <iostream>

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
		m_noise(std::make_unique<Noise>(seed)),
		m_noiseCompute(std::make_unique<NoiseCompute>(seed))
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

        constexpr int SAMPLE_STEP =
			WorldGenerationSettings::TERRAIN_SAMPLE_STEP;

		const int sampleCountX =
			((Chunk::WIDTH + SAMPLE_STEP - 1) / SAMPLE_STEP) + 1;
		const int sampleCountZ =
			((Chunk::DEPTH + SAMPLE_STEP - 1) / SAMPLE_STEP) + 1;

		std::vector<int> terrainSamples(
			sampleCountX * sampleCountZ
		);

		for (int sampleX = 0; sampleX < sampleCountX; ++sampleX)
		{
			for (int sampleZ = 0; sampleZ < sampleCountZ; ++sampleZ)
			{
				terrainSamples[sampleX * sampleCountZ + sampleZ] =
					getTerrainHeight(
						originX + sampleX * SAMPLE_STEP,
						originZ + sampleZ * SAMPLE_STEP
					);
			}
		}

		std::vector<int> terrainHeights(
			Chunk::WIDTH * Chunk::DEPTH
		);

		std::vector<Biome> biomeCache(
			Chunk::WIDTH * Chunk::DEPTH
		);

		for (int x = 0; x < Chunk::WIDTH; ++x)
		{
			for (int z = 0; z < Chunk::DEPTH; ++z)
			{
				biomeCache[x * Chunk::DEPTH + z] =
					getBiome(
						originX + x,
						originZ + z
					);
			}
		}

		for (int x = 0; x < Chunk::WIDTH; ++x)
		{
			for (int z = 0; z < Chunk::DEPTH; ++z)
			{
				const int sampleX = x / SAMPLE_STEP;
				const int sampleZ = z / SAMPLE_STEP;
				const float interpolationX =
					static_cast<float>(x % SAMPLE_STEP) /
					static_cast<float>(SAMPLE_STEP);
				const float interpolationZ =
					static_cast<float>(z % SAMPLE_STEP) /
					static_cast<float>(SAMPLE_STEP);

				const float height00 = static_cast<float>(
					terrainSamples[sampleX * sampleCountZ + sampleZ]
				);
				const float height10 = static_cast<float>(
					terrainSamples[(sampleX + 1) * sampleCountZ + sampleZ]
				);
				const float height01 = static_cast<float>(
					terrainSamples[sampleX * sampleCountZ + sampleZ + 1]
				);
				const float height11 = static_cast<float>(
					terrainSamples[(sampleX + 1) * sampleCountZ + sampleZ + 1]
				);

				const float heightX0 =
					height00 + (height10 - height00) * interpolationX;
				const float heightX1 =
					height01 + (height11 - height01) * interpolationX;
				const float interpolatedHeight =
					heightX0 + (heightX1 - heightX0) * interpolationZ;

				terrainHeights[x * Chunk::DEPTH + z] =
					static_cast<int>(interpolatedHeight);
			}
		}

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
               const int terrainHeight =
					terrainHeights[x * Chunk::DEPTH + z];
				const Biome biome =
					biomeCache[x * Chunk::DEPTH + z];

				const int generationTop =
					std::min(
						Chunk::HEIGHT - 1,
						std::max(
							terrainHeight,
							WorldGenerationSettings::SEA_LEVEL
						)
					);

				for (
					int y = 0;
                  y <= generationTop;
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
                          worldZ,
                           terrainHeight,
							biome
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
      generateVegetation(chunk, biomeCache);
		m_vegetationGenerationTime += std::chrono::steady_clock::now() - vegetationStart;

		generateLake(chunk);
       removeFloatingLeaves(chunk);
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

	void WorldGenerator::testNoiseCPUvsGPU()
	{
		std::cout << std::endl;
		std::cout << "========================================" << std::endl;
		std::cout << "       NOISE CPU vs GPU TEST" << std::endl;
		std::cout << "========================================" << std::endl;

		constexpr int chunkX = 0;
		constexpr int chunkZ = 0;
		constexpr int sampleStep = 1;

		constexpr int gridSize = 16;

		std::vector<int> gpuHeights(
			gridSize * gridSize
		);

		// --------------------------------------------------------
		// GPU
		// --------------------------------------------------------

		m_noiseCompute->computeHeightmap(
			chunkX,
			chunkZ,
			sampleStep,
			gpuHeights
		);


		// --------------------------------------------------------
		// CPU
		// --------------------------------------------------------

		std::cout << std::endl;
		std::cout << "Comparaison des hauteurs :" << std::endl;
		std::cout << std::endl;

		for (int z = 0; z < gridSize; ++z)
		{
			for (int x = 0; x < gridSize; ++x)
			{
				const int worldX =
					chunkX * 16 + x * sampleStep;

				const int worldZ =
					chunkZ * 16 + z * sampleStep;


				/*
				 * Pour l'instant, le CPU utilise directement
				 * fractalNoise2D().
				 *
				 * Les paramètres doivent être les mêmes
				 * que ceux du Compute Shader.
				 */
				const float cpuNoise =
					m_noise->fractalNoise2D(
						static_cast<float>(worldX) * 0.01f,
						static_cast<float>(worldZ) * 0.01f,
						5,
						0.5f,
						2.0f
					);


				const int cpuHeight =
					static_cast<int>(
						32.0f +
						cpuNoise * 20.0f
						);


				const int gpuHeight =
					gpuHeights[x * gridSize + z];


				const int difference =
					std::abs(cpuHeight - gpuHeight);


				std::cout
					<< "("
					<< worldX
					<< ", "
					<< worldZ
					<< ") "
					<< "CPU="
					<< cpuHeight
					<< " GPU="
					<< gpuHeight
					<< " DIFF="
					<< difference
					<< std::endl;
			}
		}

		std::cout << std::endl;
		std::cout << "========================================" << std::endl;
		std::cout << "       FIN DU TEST CPU vs GPU" << std::endl;
		std::cout << "========================================" << std::endl;
		std::cout << std::endl;
	}

	void WorldGenerator::generateVegetation(
        Chunk& chunk,
		const std::vector<Biome>& biomeCache
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
                      worldZ,
						chunk,
						biomeCache
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
                      worldZ,
						chunk,
						biomeCache
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
      int worldZ,
		const Chunk& chunk,
		const std::vector<Biome>& biomeCache
	) const
	{
        const int localX =
			worldX - chunk.getChunkX() * Chunk::WIDTH;
		const int localZ =
			worldZ - chunk.getChunkZ() * Chunk::DEPTH;

		const Biome biome =
			localX >= 0 && localX < Chunk::WIDTH &&
			localZ >= 0 && localZ < Chunk::DEPTH
			? biomeCache[localX * Chunk::DEPTH + localZ]
			: getBiome(worldX, worldZ);

		if (
           biome != Biome::Desert
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
      int worldZ,
		const Chunk& chunk,
		const std::vector<Biome>& biomeCache
	) const
	{
     for (int dx = -2; dx <= 2; ++dx)
		{
           for (int dz = -2; dz <= 2; ++dz)
			{
				if (isInRiverZone(worldX + dx, worldZ + dz))
				{
					return false;
				}
			}
		}

     const int localX =
			worldX - chunk.getChunkX() * Chunk::WIDTH;
		const int localZ =
			worldZ - chunk.getChunkZ() * Chunk::DEPTH;

		const Biome biome =
			localX >= 0 && localX < Chunk::WIDTH &&
			localZ >= 0 && localZ < Chunk::DEPTH
			? biomeCache[localX * Chunk::DEPTH + localZ]
			: getBiome(worldX, worldZ);

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
          int worldZ,
       int terrainHeight,
		Biome biome
	) const
	{
		if (isInRiverZone(worldX, worldZ))
		{
            const int riverSurface =
				terrainHeight - WorldGenerationSettings::RIVER_BANK_OFFSET;

			if (worldY == WorldGenerationSettings::RIVER_BOTTOM - 1)
			{
				return Block::Sand;
			}

			if (worldY >= WorldGenerationSettings::RIVER_BOTTOM &&
               worldY <= riverSurface)
			{
				return Block::Water;
			}

            if (worldY > riverSurface)
			{
				return Block::Air;
			}
		}

	  /*
		 * Espace au-dessus du terrain.
		 *
		 * Les dépressions situées sous le niveau marin
		 * sont remplies d'eau jusqu'à SEA_LEVEL.
		 */

		if (worldY > terrainHeight)
		{
           int waterLevel = WorldGenerationSettings::SEA_LEVEL;

          for (int offsetX = -1; offsetX <= 1; ++offsetX)
			{
              for (int offsetZ = -1; offsetZ <= 1; ++offsetZ)
				{
                  if (offsetX == 0 && offsetZ == 0)
					{
						continue;
					}

					const int neighborWorldX = worldX + offsetX;
					const int neighborWorldZ = worldZ + offsetZ;
					const Biome neighborBiome =
						getBiome(neighborWorldX, neighborWorldZ);

					const bool hasGrassSurface =
						neighborBiome != Biome::Desert &&
						!isInRiverZone(neighborWorldX, neighborWorldZ) &&
						!isInLakeZone(neighborWorldX, neighborWorldZ) &&
						!isInDesertZone(neighborWorldX, neighborWorldZ);

                   if (hasGrassSurface)
					{
                        const int grassY =
							getTerrainHeight(
								neighborWorldX,
								neighborWorldZ
							);

						if (waterLevel >= grassY)
						{
							waterLevel = grassY - 1;
						}
					}
				}
			}

			if (worldY <= waterLevel)
			{
				return Block::Water;
			}

			return Block::Air;
		}

       const bool isMountain =
			terrainHeight >=
			WorldGenerationSettings::MOUNTAIN_CAVE_ENTRY_HEIGHT;

		const int caveSurfaceDepth =
			isMountain
			? 0
			: WorldGenerationSettings::CAVE_SURFACE_DEPTH;

		if (WorldGenerationSettings::GENERATE_CAVE_TUNNELS &&
			worldY >= WorldGenerationSettings::CAVE_MIN_Y &&
			worldY <= terrainHeight - caveSurfaceDepth &&
			!isInLakeZone(worldX, worldZ) &&
			!isInRiverZone(worldX, worldZ))
		{
			const float caveNoise =
				m_noise->fractalNoise3D(
					static_cast<float>(worldX) *
						WorldGenerationSettings::CAVE_TUNNEL_FREQUENCY,
					static_cast<float>(worldY) *
						WorldGenerationSettings::CAVE_VERTICAL_FREQUENCY,
					static_cast<float>(worldZ) *
						WorldGenerationSettings::CAVE_TUNNEL_FREQUENCY,
					3,
					0.5f,
					2.0f
				);

			if (caveNoise > WorldGenerationSettings::CAVE_TUNNEL_THRESHOLD)
			{
				return Block::Air;
			}
		}

		/*
		 * Biome de cette colonne
		 */

		if (isInDesertZone(worldX, worldZ))
		{
			if (worldY >= terrainHeight - 3)
			{
				return Block::Sand;
			}

			return Block::Stone;
		}


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

	bool WorldGenerator::isInLakeZone(
		int worldX,
		int worldZ
	) const
	{
		if (!WorldGenerationSettings::GUARANTEE_LAKES)
		{
			return false;
		}

		const int lakeWorldX =
           (WorldGenerationSettings::LAKE_SPAWN_CHUNK_X * Chunk::WIDTH) +
			WorldGenerationSettings::LAKE_CENTER_OFFSET_X;

		const int lakeWorldZ =
           (WorldGenerationSettings::LAKE_SPAWN_CHUNK_Z * Chunk::DEPTH) +
			WorldGenerationSettings::LAKE_CENTER_OFFSET_Z;

		const int distX = worldX - lakeWorldX;
		const int distZ = worldZ - lakeWorldZ;
		const int distSquared = distX * distX + distZ * distZ;
		const int radiusSquared =
			WorldGenerationSettings::LAKE_RADIUS *
			WorldGenerationSettings::LAKE_RADIUS;

		return distSquared <= radiusSquared;
	}

	void WorldGenerator::generateLake(
		Chunk& chunk
	)
	{
		if (!WorldGenerationSettings::GUARANTEE_LAKES)
		{
			return;
		}

		const int chunkX = chunk.getChunkX();
		const int chunkZ = chunk.getChunkZ();

		const int originX = chunkX * Chunk::WIDTH;
		const int originZ = chunkZ * Chunk::DEPTH;

		const int lakeWorldX =
			(WorldGenerationSettings::LAKE_SPAWN_CHUNK_X * Chunk::WIDTH) +
			WorldGenerationSettings::LAKE_CENTER_OFFSET_X;

		const int lakeWorldZ =
			(WorldGenerationSettings::LAKE_SPAWN_CHUNK_Z * Chunk::DEPTH) +
			WorldGenerationSettings::LAKE_CENTER_OFFSET_Z;

       int lakeSurface = 1000000;

		for (int sample = 0; sample < 32; ++sample)
		{
			const float angle =
				static_cast<float>(sample) * 6.2831853f / 32.0f;
			const int sampleX =
				lakeWorldX + static_cast<int>(
					std::cos(angle) *
					(WorldGenerationSettings::LAKE_RADIUS + 2)
				);
			const int sampleZ =
				lakeWorldZ + static_cast<int>(
					std::sin(angle) *
					(WorldGenerationSettings::LAKE_RADIUS + 2)
				);

			const int rimHeight =
				getTerrainHeight(sampleX, sampleZ) - 1;

			if (rimHeight < lakeSurface)
			{
				lakeSurface = rimHeight;
			}
		}

		for (int x = 0; x < Chunk::WIDTH; ++x)
		{
			for (int z = 0; z < Chunk::DEPTH; ++z)
			{
				const int worldX = originX + x;
				const int worldZ = originZ + z;

				const int distX = worldX - lakeWorldX;
				const int distZ = worldZ - lakeWorldZ;
              const float shapeVariation =
					std::sin(static_cast<float>(worldX) * 0.071f) * 0.6f +
					std::cos(static_cast<float>(worldZ) * 0.053f) * 0.4f;
				const float radius =
					static_cast<float>(WorldGenerationSettings::LAKE_RADIUS) *
					(1.0f + shapeVariation *
					WorldGenerationSettings::LAKE_SHAPE_VARIATION);
				const float distance = std::sqrt(
					static_cast<float>(distX * distX + distZ * distZ)
				);

				if (distance <= radius)
				{
                 const float normalizedDistance = distance / radius;
					const int craterBottom =
						std::max(
							WorldGenerationSettings::LAKE_BOTTOM,
							lakeSurface - static_cast<int>(
								(1.0f - normalizedDistance * normalizedDistance) *
								WorldGenerationSettings::LAKE_DEPTH
							)
						);

					for (int y = 0; y < Chunk::HEIGHT; ++y)
					{
                      if (y < craterBottom)
						{
                            continue;
						}

                      if (y == craterBottom)
						{
							chunk.set(x, y, z, Block::Sand);
                        }
						else if (y <= lakeSurface)
						{
							chunk.set(x, y, z, Block::Water);
						}
						else
						{
							chunk.set(x, y, z, Block::Air);
						}
					}
				}
			}
		}
	}

	void WorldGenerator::removeFloatingLeaves(
		Chunk& chunk
	)
	{
		constexpr int TREE_MAX_HEIGHT =
			WorldGenerationSettings::TREE_MIN_HEIGHT +
			WorldGenerationSettings::TREE_HEIGHT_VARIATION;

		constexpr int LEAF_RADIUS = 2;
		constexpr int CHUNK_BORDER_MARGIN = LEAF_RADIUS;

		for (int x = 0; x < Chunk::WIDTH; ++x)
		{
			for (int z = 0; z < Chunk::DEPTH; ++z)
			{
				for (int y = 0; y < Chunk::HEIGHT; ++y)
				{
					if (chunk.get(x, y, z) != Block::Leaves)
					{
						continue;
					}

					if (x < CHUNK_BORDER_MARGIN ||
						x >= Chunk::WIDTH - CHUNK_BORDER_MARGIN ||
						z < CHUNK_BORDER_MARGIN ||
						z >= Chunk::DEPTH - CHUNK_BORDER_MARGIN)
					{
						continue;
					}

					bool hasTrunk = false;

					for (int dx = -LEAF_RADIUS; dx <= LEAF_RADIUS && !hasTrunk; ++dx)
					{
						for (int dz = -LEAF_RADIUS; dz <= LEAF_RADIUS && !hasTrunk; ++dz)
						{
							const int minY =
								std::max(0, y - TREE_MAX_HEIGHT);

							for (int trunkY = y - 1; trunkY >= minY; --trunkY)
							{
								if (chunk.get(x + dx, trunkY, z + dz) == Block::Log)
								{
									hasTrunk = true;
									break;
								}
							}
						}
					}

					if (!hasTrunk)
					{
						chunk.set(x, y, z, Block::Air);
					}
				}
			}
		}
	}

	bool WorldGenerator::isInDesertZone(
		int worldX,
		int worldZ
	) const
	{
		if (!WorldGenerationSettings::GUARANTEE_DESERT)
		{
			return false;
		}

		const int centerX =
			WorldGenerationSettings::DESERT_SPAWN_CHUNK_X * Chunk::WIDTH +
			WorldGenerationSettings::DESERT_CENTER_OFFSET_X;
		const int centerZ =
			WorldGenerationSettings::DESERT_SPAWN_CHUNK_Z * Chunk::DEPTH +
			WorldGenerationSettings::DESERT_CENTER_OFFSET_Z;
		const int dx = worldX - centerX;
		const int dz = worldZ - centerZ;
		const int radius = WorldGenerationSettings::DESERT_RADIUS;

		return dx * dx + dz * dz <= radius * radius;
	}

	bool WorldGenerator::isInRiverZone(
		int worldX,
		int worldZ
	) const
	{
		if (!WorldGenerationSettings::GUARANTEE_RIVER)
		{
			return false;
		}

		const float riverCenterZ =
			std::sin(
				static_cast<float>(worldX) *
				WorldGenerationSettings::RIVER_FREQUENCY
			) * WorldGenerationSettings::RIVER_AMPLITUDE;

		return std::abs(
			static_cast<float>(worldZ) - riverCenterZ
		) <= static_cast<float>(WorldGenerationSettings::RIVER_WIDTH);
	}
}
