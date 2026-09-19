#pragma once

#include <cstdint>

namespace Voxel
{
	using VoxelID = std::uint16_t;

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
	namespace Block
	{
		enum VoxelType
		{
			Air = 0,
			Grass ,
			Dirt ,
			Stone ,
			Sand ,
			Water ,
			Blank ,
			Log ,
			Leaves ,
			Cactus ,


			Count = 1024
		};

		enum class BlockCollision
		{
			Solid,
			None
		};

		struct BlockProperties
		{
			bool isBreakable = true;
			bool isSolid = true;
			bool isTransparent = false;
			bool isInteractable = false;

			BlockCollision collision = BlockCollision::Solid;

			int hardness = 0;
			float resistance = 1.0f;

			float lightEmission = 0.0f;
			float lightOpacity = 1.0f;

			float transparency = 0.0f;
		};

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

			BlockProperties properties;
		};

		class BlockRegistry
		{
		public:
			static void initialize();

			static BlockInfo& get(Block::VoxelType type);
			static BlockInfo& get(VoxelID type);

			static bool isAir(Block::VoxelType type);
			static bool isSolid(Block::VoxelType type);
			static bool isTransparent(Block::VoxelType type);
			static bool isBreakable(Block::VoxelType type);
			static bool isInteractable(Block::VoxelType type);
			static bool canBreak(Block::VoxelType type, int force);
			static BlockCollision getCollision(Block::VoxelType type);
			static int getHardness(Block::VoxelType type);
			static float getResistance(Block::VoxelType type);
			static float getLightEmission(Block::VoxelType type);
			static float getLightOpacity(Block::VoxelType type);
			static float getTransparency(Block::VoxelType type);
			static int getTexture(Block::VoxelType type, CubeFace face);
			static bool isAir(VoxelID type);
			static bool isSolid(VoxelID type);
			static bool isTransparent(VoxelID type);
			static bool isBreakable(VoxelID type);
			static bool isInteractable(VoxelID type);
			static bool canBreak(VoxelID type, int force);
			static BlockCollision getCollision(VoxelID type);
			static int getHardness(VoxelID type);
			static float getResistance(VoxelID type);
			static float getLightEmission(VoxelID type);
			static float getLightOpacity(VoxelID type);
			static float getTransparency(VoxelID type);
			static int getTexture(VoxelID type, CubeFace face);

		private:
			static BlockInfo m_blocks[static_cast<int>(Block::VoxelType::Count)];
			static bool m_initialized;
		};
	}



	inline const Block::BlockInfo& getBlockInfo(VoxelID id)
	{
		static const Block::BlockInfo air{
			{
				-1, // +X
				-1, // -X
				-1, // +Y
				-1, // -Y
				-1, // +Z
				-1  // -Z
			},
			{
				false, // isBreakable
				false, // isSolid
				false, // isTransparent
				false, // isInteractable
				Block::BlockCollision::None, // collision
				0, // hardness
				0.0f, // resistance
				0.0f, // lightEmission
				0.0f, // lightOpacity
				0.0f // transparency
			}
		};

		static const Block::BlockInfo grass{
			{
				1, // +X
				1, // -X
				0, // +Y
				1, // -Y
				1, // +Z
				1  // -Z
			},
			{
				true,  // isBreakable
				true,  // isSolid
				false, // isTransparent
				false, // isInteractable
				Block::BlockCollision::Solid, // collision
				1,    // hardness
				1.0f, // resistance
				0.0f, // lightEmission
				1.0f, // lightOpacity
				0.0f  // transparency
			}
		};

		static const Block::BlockInfo dirt{
			{
				2, // +X
				2, // -X
				2, // +Y
				2, // -Y
				2, // +Z
				2  // -Z
			}
		};

		static const Block::BlockInfo stone{
			{
				3, // +X
				3, // -X
				3, // +Y
				3, // -Y
				3, // +Z
				3  // -Z
			}
		};

		static const Block::BlockInfo sand{
			{
				4, // +X
				4, // -X
				4, // +Y
				4, // -Y
				4, // +Z
				4  // -Z
			}
		};

		static const Block::BlockInfo water{
			{
				5, // +X
				5, // -X
				5, // +Y
				5, // -Y
				5, // +Z
				5  // -Z
			}
		};

		static const Block::BlockInfo blank{
			{
				6, // +X
				6, // -X
				6, // +Y
				6, // -Y
				6, // +Z
				6  // -Z
			}
		};

		static const Block::BlockInfo Gold{
			{
				7, // +X
				7, // -X
				7, // +Y
				7, // -Y
				7, // +Z
				7  // -Z
			}
		};

		static const Block::BlockInfo log{
			{
				8, // +X
				8, // -X
				8, // +Y
				8, // -Y
				8, // +Z
				8  // -Z
			}
		};

		static const Block::BlockInfo leaves{
			{
				9, // +X
				9, // -X
				9, // +Y
				9, // -Y
				9, // +Z
				9  // -Z
			}
		};

		static const Block::BlockInfo cactus{
			{
				10, // +X
				10, // -X
				10, // +Y
				10, // -Y
				10, // +Z
				10  // -Z
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

		case Block::Log:
			return log;

		case Block::Leaves:
			return leaves;

		case Block::Cactus:
			return cactus;

		case Block::Count:
			return blank;

		default:
			return air;
		}
	}
}