#pragma once

#include <glm/vec3.hpp>
#include <glm/fwd.hpp>
#include <cstdint>
#include <limits>
#include <memory>

namespace Voxel
{
    class World;
    class Camera;
    class Chunk;

    // Contrôleur physique du joueur : mouvement, gravité, collisions
    // AABB contre le monde voxel, saut et interaction (casser/poser).
    //
    // Player
    //  ?
    //  ??? Movement
    //  ??? Gravity
    //  ??? Collision (Sol, Murs, Plafond, Blocs voisins)
    //  ??? Jump
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
            return m_onGround;
        }

    private:

        void handleMovementInput(
            float deltaTime
        );

        void applyGravity(
            float deltaTime
        );

        void handleJump();

        void moveAndCollide(
            float deltaTime
        );

        void resolveAxisCollision(
            glm::vec3& position,
            glm::vec3& velocity,
            int axis,
            float delta
        );

        bool isBlockSolid(
            int worldX,
            int worldY,
            int worldZ
        ) const;

        bool collidesAt(
            const glm::vec3& position
        ) const;

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
        glm::vec3 m_velocity{ 0.0f, 0.0f, 0.0f };

        // Dimensions de la boîte de collision du joueur.
        float m_width = 0.6f;
        float m_height = 1.8f;
        float m_eyeHeight = 1.62f;

        float m_moveSpeed = 5.5f;
        float m_sprintMultiplier = 1.8f;
        float m_jumpVelocity = 8.0f;
        float m_gravity = 24.0f;
        float m_maxFallSpeed = 60.0f;

        bool m_onGround = false;

        // Bloc utilisé pour poser (interaction).
        std::uint16_t m_selectedBlock;

        // Cache du dernier chunk consulté lors des tests de collision,
        // pour éviter de refaire une recherche dans la table des chunks
        // du monde pour chaque voxel testé (souvent dans le même chunk).
        mutable int m_cachedChunkX = std::numeric_limits<int>::min();
        mutable int m_cachedChunkZ = std::numeric_limits<int>::min();
        mutable const Chunk* m_cachedChunk = nullptr;

        float m_interactionCooldown = 0.0f;
        float m_interactionDelay = 0.2f;
        float m_interactionReach = 6.0f;

        bool m_previousLeftClick = false;
        bool m_previousRightClick = false;
    };
}
