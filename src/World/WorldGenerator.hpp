#pragma once
#include <cstdint>
#include <memory>
#include "../Math/Noise.hpp"
#include "Biome.hpp"


namespace Voxel
{
	class Chunk;
	class Noise;

	using VoxelID = std::uint16_t;

	class WorldGenerator
	{
	public:

		explicit WorldGenerator(
			std::uint32_t seed
		);

		void generateChunk(
			Chunk& chunk
		);

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