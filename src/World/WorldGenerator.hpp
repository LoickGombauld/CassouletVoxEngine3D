#pragma once

#include <cstdint>


namespace Voxel
{
	class Chunk;
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

	private:

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