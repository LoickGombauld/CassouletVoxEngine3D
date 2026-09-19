#pragma once

#include "Voxel.hpp"
#include "Chunk.hpp"

namespace Voxel
{
    namespace Block
    {
        BlockInfo BlockRegistry::m_blocks[
            static_cast<int>(VoxelType::Count)
        ];

        bool BlockRegistry::m_initialized = false;

        void BlockRegistry::initialize()
        {
            if (m_initialized)
                return;

            // Air
            m_blocks[static_cast<int>(VoxelType::Air)] = {
                .texture = { 
                -1, // +X
                -1, // -X
                -1, // +Y
                -1, // -Y
                -1, // +Z
                -1  // -Z 
            },
                .properties = {
                    .isBreakable = false,
                    .isSolid = false,
                    .isTransparent = true,
                    .isInteractable = false,
                    .collision = BlockCollision::None,
                    .hardness = 0,
                    .resistance = 0.0f,
                    .lightEmission = 0.0f,
                    .lightOpacity = 0.0f,
                    .transparency = 1.0f
                }
            };

            // Grass
            m_blocks[static_cast<int>(Block::VoxelType::Grass)] = {
                .texture = {
                1, // +X
                1, // -X
                0, // +Y
                1, // -Y
                1, // +Z
                1  // -Z
                },
                .properties = {
                    .isBreakable = true,
                    .isSolid = true,
                    .isTransparent = false,
                    .isInteractable = false,
                    .collision = BlockCollision::Solid,
                    .hardness = 1,
                    .resistance = 1.0f,
                    .lightEmission = 0.0f,
                    .lightOpacity = 1.0f,
                    .transparency = 0.0f
                }
            };

            // Dirt
            m_blocks[static_cast<int>(Block::VoxelType::Dirt)] = {
                .texture = {
                2, // +X
                2, // -X
                2, // +Y
                2, // -Y
                2, // +Z
                2  // -Z
                },
                .properties = {
                    .isBreakable = true,
                    .isSolid = true,
                    .isTransparent = false,
                    .isInteractable = false,
                    .collision = BlockCollision::Solid,
                    .hardness = 1,
                    .resistance = 1.0f,
                    .lightEmission = 0.0f,
                    .lightOpacity = 1.0f,
                    .transparency = 0.0f
                }
			};

            // Stone
            m_blocks[static_cast<int>(Block::VoxelType::Stone)] = {
                .texture = {
                3, // +X
                3, // -X
                3, // +Y
                3, // -Y
                3, // +Z
                3  // -Z
                },
                .properties = {
                    .isBreakable = true,
                    .isSolid = true,
                    .isTransparent = false,
                    .isInteractable = false,
                    .collision = BlockCollision::Solid,
                    .hardness = 2,
                    .resistance = 2.0f,
                    .lightEmission = 0.0f,
                    .lightOpacity = 1.0f,
                    .transparency = 0.0f
                }
			};

            // Sand
            m_blocks[static_cast<int>(Block::VoxelType::Sand)] = {
                .texture = {
                4, // +X
                4, // -X
                4, // +Y
                4, // -Y
                4, // +Z
                4  // -Z
                },
                .properties = {
                    .isBreakable = true,
                    .isSolid = true,
                    .isTransparent = false,
                    .isInteractable = false,
                    .collision = BlockCollision::Solid,
                    .hardness = 1,
                    .resistance = 1.0f,
                    .lightEmission = 0.0f,
                    .lightOpacity = 1.0f,
                    .transparency = 0.0f
                }
            };
                // Water
            m_blocks[static_cast<int>(Block::VoxelType::Water)] = {
                .texture = {
                5, // +X
                5, // -X
                5, // +Y
                5, // -Y
                5, // +Z
                5  // -Z
                },
                .properties = {
                    .isBreakable = false,
                    .isSolid = false,
                    .isTransparent = true,
                    .isInteractable = false,
                    .collision = BlockCollision::None,
                    .hardness = 0,
                    .resistance = 0.0f,
                    .lightEmission = 0.0f,
                    .lightOpacity = 0.5f,
                    .transparency = 0.5f
                }
            };
            // Blank
            m_blocks[static_cast<int>(Block::VoxelType::Blank)] = {
                .texture = {
                6, // +X
                6, // -X
                6, // +Y
                6, // -Y
                6, // +Z
                6  // -Z
                },
                .properties = {
                    .isBreakable = false,
                    .isSolid = false,
                    .isTransparent = true,
                    .isInteractable = false,
                    .collision = BlockCollision::None,
                    .hardness = 0,
                    .resistance = 0.0f,
                    .lightEmission = 0.0f,
                    .lightOpacity = 0.0f,
                    .transparency = 0.0f
                }
            };

            m_blocks[static_cast<int>(Block::VoxelType::Leaves)] = {
                .texture = {
                9, // +X
                9, // -X
                9, // +Y
                9, // -Y
                9, // +Z
                9  // -Z
                },
                .properties = {
                    .isBreakable = true,
                    .isSolid = true,
                    .isTransparent = false,
                    .isInteractable = false,
                    .collision = BlockCollision::None,
                    .hardness = 1,
                    .resistance = 1.0f,
                    .lightEmission = 0.0f,
                    .lightOpacity = 1.0f,
                    .transparency = 0.0f
                }
            };  

            m_blocks[static_cast<int>(Block::VoxelType::Log)] = {
                .texture = {
                8, // +X
                8, // -X
                8, // +Y
                8, // -Y
                8, // +Z
                8  // -Z
                },
                .properties = {
                    .isBreakable = true,
                    .isSolid = true,
                    .isTransparent = false,
                    .isInteractable = false,
                    .collision = BlockCollision::Solid,
                    .hardness = 2,
                    .resistance = 2.0f,
                    .lightEmission = 0.0f,
                    .lightOpacity = 1.0f,
                    .transparency = 0.0f
                }
			};

            m_initialized = true;
        }

        BlockInfo& Block::BlockRegistry::get(Block::VoxelType type)
        {
            if (!m_initialized)
                initialize();

            return m_blocks[static_cast<int>(type)];
        }

        BlockInfo& BlockRegistry::get(VoxelID type)
        {
			return get(static_cast<VoxelType>(type));
        }

        bool BlockRegistry::isAir(VoxelType type)
        {
            return type == VoxelType::Air;
        }

        bool BlockRegistry::isSolid(VoxelType type)
        {
            return get(type).properties.isSolid;
        }

        bool BlockRegistry::isTransparent(VoxelType type)
        {
            return get(type).properties.isTransparent;
        }

        bool BlockRegistry::isBreakable(VoxelType type)
        {
            return get(type).properties.isBreakable;
        }

        bool  BlockRegistry::canBreak(VoxelType voxel, int force)
        {
            return get(voxel).properties.isBreakable && force >= get(voxel).properties.hardness;
        }

        BlockCollision BlockRegistry::getCollision(VoxelType type)
        {
            return get(type).properties.collision;
        }

        int BlockRegistry::getHardness(VoxelType type)
        {
            return get(type).properties.hardness;
		}

        float BlockRegistry::getResistance(VoxelType type)
        {
            return get(type).properties.resistance;
		}

        float BlockRegistry::getLightEmission(VoxelType type)
        {
            return get(type).properties.lightEmission;
        }

        float BlockRegistry::getLightOpacity(VoxelType type)
        {
            return get(type).properties.lightOpacity;
		}

        float BlockRegistry::getTransparency(VoxelType type)
        {
            return get(type).properties.transparency;
		}

        int BlockRegistry::getTexture(VoxelType type, CubeFace face)
        {
            return get(type).texture[static_cast<int>(face)];
		}

        bool BlockRegistry::isAir(VoxelID type)
        {
            return isAir(static_cast<VoxelType>(type));
        }

        bool BlockRegistry::isSolid(VoxelID type)
        {
			return isSolid(static_cast<VoxelType>(type));
        }

        bool BlockRegistry::isTransparent(VoxelID type)
        {
            return isTransparent(static_cast<VoxelType>(type));
		}

        bool BlockRegistry::isBreakable(VoxelID type)
        {
            return isBreakable(static_cast<VoxelType>(type));
        }

        bool BlockRegistry::canBreak(VoxelID type, int force)
        {
            return canBreak(static_cast<VoxelType>(type), force);
		}

        BlockCollision BlockRegistry::getCollision(VoxelID type)
        {
			return getCollision(static_cast<VoxelType>(type));
        }

        int BlockRegistry::getHardness(VoxelID type)
        {
            return getHardness(static_cast<VoxelType>(type));
		}

        float BlockRegistry::getResistance(VoxelID type)
        {
            return getResistance(static_cast<VoxelType>(type));
        }

        float BlockRegistry::getLightEmission(VoxelID type)
        {
            return getLightEmission(static_cast<VoxelType>(type));
		}

        float BlockRegistry::getLightOpacity(VoxelID type)
        {
            return getLightOpacity(static_cast<VoxelType>(type));
        }

        float BlockRegistry::getTransparency(VoxelID type)
        {
            return getTransparency(static_cast<VoxelType>(type));
		}

        int BlockRegistry::getTexture(VoxelID type, CubeFace face)
        {
            return getTexture(static_cast<VoxelType>(type), face);
        }



        bool BlockRegistry::isInteractable(VoxelID type)
        {
            return isInteractable(static_cast<VoxelType>(type));
        }

        bool  BlockRegistry::isInteractable(VoxelType voxel)
        {
            return get(voxel).properties.isInteractable;
        }
    }
}