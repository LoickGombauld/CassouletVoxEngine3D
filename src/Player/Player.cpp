#include "../Player/Player.hpp"
#include "../World/World.hpp"
#include "../Camera/Camera.hpp"
#include "../Voxel/Voxel.hpp"
#include "../Voxel/Chunk.hpp"
#include "../Input/Input.hpp"

#include <GLFW/glfw3.h>
#include <glm/vec3.hpp>
#include <glm/common.hpp>
#include <glm/trigonometric.hpp>
#include <glm/geometric.hpp>
#include <algorithm>
#include <cmath>

namespace Voxel
{
    Player::Player(
        World& world,
        Camera& camera,
        const glm::vec3& spawnPosition
    )
        : m_world(world),
        m_camera(camera),
        m_position(spawnPosition),
        m_selectedBlock(Block::Stone)
    {
        m_camera.setPosition(
            m_position + glm::vec3(0.0f, m_eyeHeight, 0.0f)
        );
    }

    void Player::setPosition(
        const glm::vec3& position
    )
    {
        m_position = position;

        m_camera.setPosition(
            m_position + glm::vec3(0.0f, m_eyeHeight, 0.0f)
        );
    }

    void Player::update(
        float deltaTime
    )
    {
        handleMovementInput(deltaTime);
        handleJump();
        applyGravity(deltaTime);
        moveAndCollide(deltaTime);

        m_camera.setPosition(
            m_position + glm::vec3(0.0f, m_eyeHeight, 0.0f)
        );

        if (m_interactionCooldown > 0.0f)
        {
            m_interactionCooldown -= deltaTime;
        }

        handleInteraction();
    }

    void Player::handleMovementInput(
        float deltaTime
    )
    {
        // Le déplacement horizontal suit l'orientation de la caméra
        // (yaw uniquement), pour ne pas voler quand le joueur regarde
        // vers le haut/bas.
        const float yawRadians = glm::radians(m_camera.getYaw());

        const glm::vec3 forward = glm::normalize(
            glm::vec3(
                std::cos(yawRadians),
                0.0f,
                std::sin(yawRadians)
            )
        );

        const glm::vec3 right = glm::normalize(
            glm::cross(forward, glm::vec3(0.0f, 1.0f, 0.0f))
        );

        glm::vec3 moveDirection(0.0f);

        if (Input::isKeyDown(GLFW_KEY_W))
        {
            moveDirection += forward;
        }

        if (Input::isKeyDown(GLFW_KEY_S))
        {
            moveDirection -= forward;
        }

        if (Input::isKeyDown(GLFW_KEY_D))
        {
            moveDirection += right;
        }

        if (Input::isKeyDown(GLFW_KEY_A))
        {
            moveDirection -= right;
        }

        float speed = m_moveSpeed;

        if (Input::isKeyDown(GLFW_KEY_LEFT_SHIFT))
        {
            speed *= m_sprintMultiplier;
        }

        if (glm::length(moveDirection) > 0.0f)
        {
            moveDirection = glm::normalize(moveDirection);
        }

        m_velocity.x = moveDirection.x * speed;
        m_velocity.z = moveDirection.z * speed;
    }

    void Player::handleJump()
    {
        if (m_onGround && Input::isKeyDown(GLFW_KEY_SPACE))
        {
            m_velocity.y = m_jumpVelocity;
            m_onGround = false;
        }
    }

    void Player::applyGravity(
        float deltaTime
    )
    {
        m_velocity.y -= m_gravity * deltaTime;

        if (m_velocity.y < -m_maxFallSpeed)
        {
            m_velocity.y = -m_maxFallSpeed;
        }
    }

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

    bool Player::isBlockSolid(
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

    bool Player::collidesAt(
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

    void Player::resolveAxisCollision(
        glm::vec3& position,
        glm::vec3& velocity,
        int axis,
        float delta
    )
    {
        glm::vec3 candidate = position;
        candidate[axis] += delta;

        if (!collidesAt(candidate))
        {
            position = candidate;
            return;
        }

        // Collision détectée : on annule la vitesse sur cet axe et on
        // s'arrête juste avant le bloc (résolution simple par pas).
        velocity[axis] = 0.0f;

        if (axis == 1 && delta < 0.0f)
        {
            m_onGround = true;
        }
    }

    void Player::moveAndCollide(
        float deltaTime
    )
    {
        m_onGround = false;

        // Résolution axe par axe pour gérer correctement le sol, les
        // murs, le plafond et les blocs voisins sans coincer le joueur.
        resolveAxisCollision(
            m_position,
            m_velocity,
            0,
            m_velocity.x * deltaTime
        );

        resolveAxisCollision(
            m_position,
            m_velocity,
            1,
            m_velocity.y * deltaTime
        );

        resolveAxisCollision(
            m_position,
            m_velocity,
            2,
            m_velocity.z * deltaTime
        );

        // Vérifie explicitement si le joueur repose sur le sol (pour le
        // cas où la vitesse verticale est nulle mais le joueur est posé).
        if (!m_onGround)
        {
            glm::vec3 belowPosition = m_position;
            belowPosition.y -= 0.05f;

            if (collidesAt(belowPosition))
            {
                m_onGround = true;
            }
        }
    }

    bool Player::raycastBlock(
        glm::ivec3& outHitBlock,
        glm::ivec3& outPreviousBlock,
        float maxDistance
    ) const
    {
        const glm::vec3 origin =
            m_position + glm::vec3(0.0f, m_eyeHeight, 0.0f);

        const float yawRadians = glm::radians(m_camera.getYaw());
        const float pitchRadians = glm::radians(m_camera.getPitch());

        const glm::vec3 direction = glm::normalize(
            glm::vec3(
                std::cos(yawRadians) * std::cos(pitchRadians),
                std::sin(pitchRadians),
                std::sin(yawRadians) * std::cos(pitchRadians)
            )
        );

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

    void Player::breakBlock()
    {
        glm::ivec3 hitBlock;
        glm::ivec3 previousBlock;

        if (!raycastBlock(hitBlock, previousBlock, m_interactionReach))
        {
            return;
        }

        m_world.setVoxel(
            hitBlock.x,
            hitBlock.y,
            hitBlock.z,
            Block::Air
        );
    }

    void Player::placeBlock()
    {
        glm::ivec3 hitBlock;
        glm::ivec3 previousBlock;

        if (!raycastBlock(hitBlock, previousBlock, m_interactionReach))
        {
            return;
        }

        // Empêche de poser un bloc à l'intérieur du joueur.
        const glm::vec3 placedCenter(
            static_cast<float>(previousBlock.x) + 0.5f,
            static_cast<float>(previousBlock.y) + 0.5f,
            static_cast<float>(previousBlock.z) + 0.5f
        );

        const float halfWidth = m_width * 0.5f;

        const bool overlapsPlayer =
            placedCenter.x + 0.5f > m_position.x - halfWidth &&
            placedCenter.x - 0.5f < m_position.x + halfWidth &&
            placedCenter.z + 0.5f > m_position.z - halfWidth &&
            placedCenter.z - 0.5f < m_position.z + halfWidth &&
            placedCenter.y + 0.5f > m_position.y &&
            placedCenter.y - 0.5f < m_position.y + m_height;

        if (overlapsPlayer)
        {
            return;
        }

        m_world.setVoxel(
            previousBlock.x,
            previousBlock.y,
            previousBlock.z,
            m_selectedBlock
        );
    }

    void Player::handleInteraction()
    {
        if (m_interactionCooldown > 0.0f)
        {
            return;
        }

        const bool leftClick =
            Input::isMouseButtonDown(GLFW_MOUSE_BUTTON_LEFT);

        const bool rightClick =
            Input::isMouseButtonDown(GLFW_MOUSE_BUTTON_RIGHT) &&
            !Input::isMouseButtonDown(GLFW_MOUSE_BUTTON_LEFT);

        if (leftClick && !m_previousLeftClick)
        {
            breakBlock();
            m_interactionCooldown = m_interactionDelay;
        }
        else if (rightClick && !m_previousRightClick && Input::isMouseCaptured())
        {
            placeBlock();
            m_interactionCooldown = m_interactionDelay;
        }

        m_previousLeftClick = leftClick;
        m_previousRightClick = rightClick;
    }
}
