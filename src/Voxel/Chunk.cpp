
#include "../Voxel/Chunk.hpp"

#include "../Voxel/VoxelMesher.hpp"

#include "../Renderer/Mesh.hpp"

#include <glm/vec3.hpp>

namespace Voxel
{
	Chunk::Chunk(
		int chunkX,
		int chunkZ
	)
		: m_chunkX(chunkX),
		m_chunkZ(chunkZ),
		m_voxels(VOLUME, Block::Air)
	{
	}

	int Chunk::getIndex(
		int x,
		int y,
		int z
	) const
	{
		return x +
			WIDTH *
			(z +
				DEPTH * y);
	}

	bool Chunk::isInside(
		int x,
		int y,
		int z
	) const
	{
		return
			x >= 0 &&
			x < WIDTH &&
			y >= 0 &&
			y < HEIGHT &&
			z >= 0 &&
			z < DEPTH;
	}

	VoxelID Chunk::get(
		int x,
		int y,
		int z
	) const
	{
		if (!isInside(x, y, z))
			return Block::Air;

		return m_voxels[
			getIndex(x, y, z)
		];
	}

	void Chunk::set(
		int x,
		int y,
		int z,
		VoxelID voxel
	)
	{
		if (!isInside(x, y, z))
			return;

		m_voxels[
			getIndex(x, y, z)
		] = voxel;
	}

	int Chunk::getChunkX() const
	{
		return m_chunkX;
	}

	int Chunk::getChunkZ() const
	{
		return m_chunkZ;
	}

	glm::vec3 Chunk::getWorldPosition() const
	{
		return glm::vec3(
			static_cast<float>(
				m_chunkX * WIDTH
				),
			0.0f,
			static_cast<float>(
				m_chunkZ * DEPTH
				)
		);
	}

	void Chunk::rebuildMesh()
	{
		m_mesh = VoxelMesher::build(*this);
	}

	void Chunk::generateTestTerrain()
	{
		for (
			int x = 0;
			x < WIDTH;
			++x
			)
		{
			for (
				int z = 0;
				z < DEPTH;
				++z
				)
			{
				for (
					int y = 0;
					y < HEIGHT;
					++y
					)
				{
					if (y == 0)
					{
						set(
							x,
							y,
							z,
							Block::Stone
						);
					}
					else if (y < 3)
					{
						set(
							x,
							y,
							z,
							Block::Dirt
						);
					}
					else if (y == 3)
					{
						set(
							x,
							y,
							z,
							Block::Grass
						);
					}
				}
			}
		}

		rebuildMesh();
	}



	void Chunk::render() const { if (!m_mesh) return; m_mesh->draw(); }
}