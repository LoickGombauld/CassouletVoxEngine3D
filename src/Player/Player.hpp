#pragma once

#include <glm/vec3.hpp>
#include <glm/fwd.hpp>
#include <cstdint>
#include <memory>

#include "../Player/PlayerCollider.hpp"
#include "../Player/PlayerPhysics.hpp"

namespace Voxel
{
    class World;
    class Camera;

    // Contrôleur du joueur : délègue la physique/collision à
    // PlayerPhysics/PlayerCollider et gère les entrées, la caméra et
    // l'interaction (casser/poser des blocs).
    //
    // Player
    //  ?
    //  ??? PlayerPhysics (Movement, Gravity, Jump, résolution collision)
    //  ??? PlayerCollider (Collision AABB, raycast, test voxel)
    //  ??? Interaction (Break block / Place block)
    class Player
    {
    public:

        Player(
            World& world,
            Camera& camera,
            const glm::vec3& spawnPosition
        );

        void update(
            float deltaTime
        );

        const glm::vec3& getPosition() const
        {
            return m_position;
        }

        void setPosition(
            const glm::vec3& position
        );

        bool isOnGround() const
        {
            return m_physics.isOnGround();
        }

    private:

        void handleMovementInput(
            float deltaTime
        );

        void handleJump();

        void handleInteraction();

        void breakBlock();

        void placeBlock();

        // Effectue un raycast depuis les yeux du joueur le long de sa
        // direction de vue. Retourne true si un bloc a été touché.
        bool raycastBlock(
            glm::ivec3& outHitBlock,
            glm::ivec3& outPreviousBlock,
            float maxDistance
        ) const;

    private:

        World& m_world;
        Camera& m_camera;

        // Position des pieds du joueur (centre de la base de l'AABB).
        glm::vec3 m_position;

        // Dimensions de la boîte de collision du joueur.
        float m_width = 0.6f;
        float m_height = 1.8f;
        float m_eyeHeight = 1.62f;

        float m_moveSpeed = 5.5f;
        float m_sprintMultiplier = 1.8f;
        float m_jumpVelocity = 8.0f;
        float m_gravity = 24.0f;
        float m_maxFallSpeed = 60.0f;

        PlayerCollider m_collider;
        PlayerPhysics m_physics;

        // Bloc utilisé pour poser (interaction).
        std::uint16_t m_selectedBlock;

        float m_interactionCooldown = 0.0f;
        float m_interactionDelay = 0.2f;
        float m_interactionReach = 6.0f;

        bool m_previousLeftClick = false;
        bool m_previousRightClick = false;
    };
}
