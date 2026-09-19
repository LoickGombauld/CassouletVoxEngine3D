#pragma once

#include <glm/vec3.hpp>

namespace Voxel
{
    class PlayerCollider;

    // Responsable de la simulation physique du joueur : mouvement
    // horizontal, gravité, saut et résolution des collisions axe par
    // axe contre le monde voxel (via un PlayerCollider).
    class PlayerPhysics
    {
    public:

        explicit PlayerPhysics(
            PlayerCollider& collider
        );

        // Applique le déplacement horizontal souhaité (déjà normalisé
        // par direction) à la vitesse du joueur.
        void setHorizontalMove(
            const glm::vec3& moveDirection,
            float speed
        );

        void jump(
            float jumpVelocity
        );

        void applyGravity(
            float gravity,
            float maxFallSpeed,
            float deltaTime
        );

        // Intègre la vitesse courante dans `position`, en résolvant les
        // collisions axe par axe (X, Y puis Z).
        void moveAndCollide(
            glm::vec3& position,
            float deltaTime
        );

        bool isOnGround() const
        {
            return m_onGround;
        }

        const glm::vec3& getVelocity() const
        {
            return m_velocity;
        }

        void setVelocity(
            const glm::vec3& velocity
        )
        {
            m_velocity = velocity;
        }

    private:

        void resolveAxisCollision(
            glm::vec3& position,
            int axis,
            float delta
        );

        PlayerCollider& m_collider;

        glm::vec3 m_velocity{ 0.0f, 0.0f, 0.0f };
        bool m_onGround = false;
    };
}
