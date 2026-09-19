#include "../Player/Player.hpp"
#include "../World/World.hpp"
#include "../Camera/Camera.hpp"
#include "../Voxel/Voxel.hpp"
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
        m_collider(world, m_width, m_height),
        m_physics(m_collider),
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
        m_physics.applyGravity(m_gravity, m_maxFallSpeed, deltaTime);
        m_physics.moveAndCollide(m_position, deltaTime);

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
        (void)deltaTime;

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

        m_physics.setHorizontalMove(moveDirection, speed);
    }

    void Player::handleJump()
    {
        if (m_physics.isOnGround() && Input::isKeyDown(GLFW_KEY_SPACE))
        {
            m_physics.jump(m_jumpVelocity);
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

        return m_collider.raycastBlock(
            origin,
            direction,
            maxDistance,
            outHitBlock,
            outPreviousBlock
        );
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
