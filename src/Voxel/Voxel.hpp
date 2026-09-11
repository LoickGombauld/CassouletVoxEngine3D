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
		constexpr VoxelID Blank = 6;
	}

	struct BlockInfo
	{
		int texture[6];
		/*
		0 = +X SIDE RIGHT
		1 = -X SIDE LEFT
		2 = +Y TOP
		3 = -Y BOTTOM
		4 = +Z SIDE FRONT
		5 = -Z SIDE BACK
		*/
	};
	inline const BlockInfo& getBlockInfo(VoxelID id)
	{
		static const BlockInfo air{
			{
				-1, // +X
				-1, // -X
				-1, // +Y
				-1, // -Y
				-1, // +Z
				-1  // -Z
			}
		};

		static const BlockInfo grass{
			{
				1, // +X
				1, // -X
				0, // +Y
				1, // -Y
				1, // +Z
				1  // -Z
			}
		};

		static const BlockInfo dirt{
			{
				2, // +X
				2, // -X
				2, // +Y
				2, // -Y
				2, // +Z
				2  // -Z
			}
		};

		static const BlockInfo stone{
			{
				3, // +X
				3, // -X
				3, // +Y
				3, // -Y
				3, // +Z
				3  // -Z
			}
		};

		static const BlockInfo sand{
			{
				4, // +X
				4, // -X
				4, // +Y
				4, // -Y
				4, // +Z
				4  // -Z
			}
		};

		static const BlockInfo water{
			{
				5, // +X
				5, // -X
				5, // +Y
				5, // -Y
				5, // +Z
				5  // -Z
			}
		};

		static const BlockInfo blank{
			{
				6, // +X
				6, // -X
				6, // +Y
				6, // -Y
				6, // +Z
				6  // -Z
			}
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

		case Block::Blank:
			return blank;

		default:
			return air;
		}
	}

	enum class FaceDirection
	{
		PosX = 0,
		NegX,
		PosY,
		NegY,
		PosZ,
		NegZ
	};

	enum class CubeFace
	{
		Right = 0, // +X
		Left,      // -X
		Top,       // +Y
		Bottom,    // -Y
		Front,     // +Z
		Back       // -Z
	};

	struct UVOrientation
	{
		float aU;
		float aV;

		float bU;
		float bV;

		float cU;
		float cV;

		float dU;
		float dV;

		CubeFace face;
	};

	constexpr UVOrientation UV_ORIENTATIONS[6] =
	{
		// +X RIGHT
		{
			0.0f, 0.0f,
			1.0f, 0.0f,
			1.0f, 1.0f,
			0.0f, 1.0f, CubeFace::Right
		},

		// -X LEFT
		{
			1.0f, 1.0f,
			0.0f, 1.0f,
			0.0f, 0.0f,
			1.0f, 0.0f, CubeFace::Left
		},

		// +Y TOP
		// Correction : l'axe V n'est plus inversé par rapport aux autres faces.
		{
			0.0f, 1.0f,
			1.0f, 1.0f,
			1.0f, 0.0f,
			0.0f, 0.0f, CubeFace::Top
		},

		// -Y BOTTOM
		{
			0.0f, 1.0f,
			1.0f, 1.0f,
			1.0f, 0.0f,
			0.0f, 0.0f, CubeFace::Bottom
		},

		// +Z FRONT
		{
			0.0f, 1.0f,
			1.0f, 1.0f,
			1.0f, 0.0f,
			0.0f, 0.0f, CubeFace::Front
		},

		// -Z BACK
		{
			1.0f, 1.0f,
			0.0f, 1.0f,
			0.0f, 0.0f,
			1.0f, 0.0f, CubeFace::Back
		}
	};

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