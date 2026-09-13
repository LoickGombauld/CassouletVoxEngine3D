#pragma once
#include <cstdint>
#include <memory>
#include <chrono>
#include "../Math/Noise.hpp"
#include "Biome.hpp"


namespace Voxel
{
	class Chunk;
	class Noise;

	using VoxelID = std::uint16_t;

	struct GenerationTimings
	{
		std::chrono::nanoseconds voxel{};
		std::chrono::nanoseconds vegetation{};
		std::chrono::nanoseconds noise{};
	};

	class WorldGenerator
	{
	public:

		explicit WorldGenerator(
			std::uint32_t seed
		);

		void generateChunk(
			Chunk& chunk
		);

		std::chrono::nanoseconds getVoxelGenerationTime() const { return m_voxelGenerationTime; }
		std::chrono::nanoseconds getVegetationGenerationTime() const { return m_vegetationGenerationTime; }
		std::chrono::nanoseconds getNoiseGenerationTime() const { return m_noise->getElapsedTime(); }
		void resetTimings();
		GenerationTimings getTimings() const;

		std::uint32_t getSeed() const
		{
			return m_seed;
		}

		float getTemperature(
			int worldX,
			int worldZ
		) const;

		float getHumidity(
			int worldX,
			int worldZ
		) const;

		Biome getBiome(
			int worldX,
			int worldZ
		) const;

	private:

		std::unique_ptr<Noise> m_noise;
		std::uint32_t m_seed;
		std::chrono::nanoseconds m_voxelGenerationTime{};
		std::chrono::nanoseconds m_vegetationGenerationTime{};

		void generateVegetation(
			Chunk& chunk
		);

		void generateTree(
			Chunk& chunk,
			int worldX,
			int worldZ
		);

		void generateCactus(
			Chunk& chunk,
			int worldX,
			int worldZ
		);

		bool shouldGenerateTree(
			int worldX,
			int worldZ
		) const;

		bool shouldGenerateCactus(
			int worldX,
			int worldZ
		) const;

		int getTerrainHeight(
			int worldX,
			int worldZ
		) const;

		VoxelID generateVoxel(
			int worldX,
			int worldY,
			int worldZ
		) const;
	};
}