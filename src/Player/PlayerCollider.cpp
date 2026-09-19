#include "../Player/PlayerCollider.hpp"
#include "../World/World.hpp"
#include "../Voxel/Voxel.hpp"
#include "../Voxel/Chunk.hpp"

#include <glm/vec3.hpp>
#include <cmath>

namespace Voxel
{
    namespace
    {
        int floorDiv(int value, int divisor)
        {
            int result = value / divisor;
            int remainder = value % divisor;

            if (remainder != 0 && remainder < 0)
            {
                --result;
            }

            return result;
        }

        int positiveModulo(int value, int divisor)
        {
            int result = value % divisor;

            if (result < 0)
            {
                result += divisor;
            }

            return result;
        }
    }

    PlayerCollider::PlayerCollider(
        World& world,
        float width,
        float height
    )
        : m_world(world),
        m_width(width),
        m_height(height)
    {
    }

    bool PlayerCollider::isBlockSolid(
        int worldX,
        int worldY,
        int worldZ
    ) const
    {
        if (worldY < 0 || worldY >= Chunk::HEIGHT)
        {
            return false;
        }

        const int chunkX = floorDiv(worldX, Chunk::WIDTH);
        const int chunkZ = floorDiv(worldZ, Chunk::DEPTH);

        // Cache du dernier chunk consulté : les tests de collision AABB
        // testent plusieurs voxels qui appartiennent souvent au même
        // chunk, ce qui évite de refaire une recherche par hash dans
        // World::m_chunks pour chaque voxel.
        if (chunkX != m_cachedChunkX || chunkZ != m_cachedChunkZ || !m_cachedChunk)
        {
            m_cachedChunk = m_world.getChunk(chunkX, chunkZ);
            m_cachedChunkX = chunkX;
            m_cachedChunkZ = chunkZ;
        }

        if (!m_cachedChunk)
        {
            return false;
        }

        const int localX = positiveModulo(worldX, Chunk::WIDTH);
        const int localZ = positiveModulo(worldZ, Chunk::DEPTH);

        const std::uint16_t voxel =
            m_cachedChunk->get(localX, worldY, localZ);

        return voxel != Block::Air && voxel != Block::Water;
    }

    bool PlayerCollider::collidesAt(
        const glm::vec3& position
    ) const
    {
        const float halfWidth = m_width * 0.5f;

        const int minX = static_cast<int>(std::floor(position.x - halfWidth));
        const int maxX = static_cast<int>(std::floor(position.x + halfWidth));
        const int minY = static_cast<int>(std::floor(position.y));
        const int maxY = static_cast<int>(std::floor(position.y + m_height));
        const int minZ = static_cast<int>(std::floor(position.z - halfWidth));
        const int maxZ = static_cast<int>(std::floor(position.z + halfWidth));

        for (int x = minX; x <= maxX; ++x)
        {
            for (int y = minY; y <= maxY; ++y)
            {
                for (int z = minZ; z <= maxZ; ++z)
                {
                    if (isBlockSolid(x, y, z))
                    {
                        return true;
                    }
                }
            }
        }

        return false;
    }

    bool PlayerCollider::raycastBlock(
        const glm::vec3& origin,
        const glm::vec3& direction,
        float maxDistance,
        glm::ivec3& outHitBlock,
        glm::ivec3& outPreviousBlock
    ) const
    {
        constexpr float STEP = 0.05f;

        glm::ivec3 previousBlock(
            static_cast<int>(std::floor(origin.x)),
            static_cast<int>(std::floor(origin.y)),
            static_cast<int>(std::floor(origin.z))
        );

        for (float distance = 0.0f; distance <= maxDistance; distance += STEP)
        {
            const glm::vec3 samplePosition = origin + direction * distance;

            const glm::ivec3 blockPosition(
                static_cast<int>(std::floor(samplePosition.x)),
                static_cast<int>(std::floor(samplePosition.y)),
                static_cast<int>(std::floor(samplePosition.z))
            );

            if (isBlockSolid(blockPosition.x, blockPosition.y, blockPosition.z))
            {
                outHitBlock = blockPosition;
                outPreviousBlock = previousBlock;
                return true;
            }

            previousBlock = blockPosition;
        }

        return false;
    }
}
