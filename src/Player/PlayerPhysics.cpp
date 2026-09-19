#include "../Player/PlayerPhysics.hpp"
#include "../Player/PlayerCollider.hpp"

#include <glm/geometric.hpp>

namespace Voxel
{
    PlayerPhysics::PlayerPhysics(
        PlayerCollider& collider
    )
        : m_collider(collider)
    {
    }

    void PlayerPhysics::setHorizontalMove(
        const glm::vec3& moveDirection,
        float speed
    )
    {
        m_velocity.x = moveDirection.x * speed;
        m_velocity.z = moveDirection.z * speed;
    }

    void PlayerPhysics::jump(
        float jumpVelocity
    )
    {
        if (m_onGround)
        {
            m_velocity.y = jumpVelocity;
            m_onGround = false;
        }
    }

    void PlayerPhysics::applyGravity(
        float gravity,
        float maxFallSpeed,
        float deltaTime
    )
    {
        m_velocity.y -= gravity * deltaTime;

        if (m_velocity.y < -maxFallSpeed)
        {
            m_velocity.y = -maxFallSpeed;
        }
    }

    void PlayerPhysics::resolveAxisCollision(
        glm::vec3& position,
        int axis,
        float delta
    )
    {
        glm::vec3 candidate = position;
        candidate[axis] += delta;

        if (!m_collider.collidesAt(candidate))
        {
            position = candidate;
            return;
        }

        // Collision détectée : on annule la vitesse sur cet axe et on
        // s'arrête juste avant le bloc (résolution simple par pas).
        m_velocity[axis] = 0.0f;

        if (axis == 1 && delta < 0.0f)
        {
            m_onGround = true;
        }
    }

    void PlayerPhysics::moveAndCollide(
        glm::vec3& position,
        float deltaTime
    )
    {
        m_onGround = false;

        // Résolution axe par axe pour gérer correctement le sol, les
        // murs, le plafond et les blocs voisins sans coincer le joueur.
        resolveAxisCollision(
            position,
            0,
            m_velocity.x * deltaTime
        );

        resolveAxisCollision(
            position,
            1,
            m_velocity.y * deltaTime
        );

        resolveAxisCollision(
            position,
            2,
            m_velocity.z * deltaTime
        );

        // Vérifie explicitement si le joueur repose sur le sol (pour le
        // cas où la vitesse verticale est nulle mais le joueur est posé).
        if (!m_onGround)
        {
            glm::vec3 belowPosition = position;
            belowPosition.y -= 0.05f;

            if (m_collider.collidesAt(belowPosition))
            {
                m_onGround = true;
            }
        }
    }
}
