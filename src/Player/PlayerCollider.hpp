#pragma once

#include <glm/vec3.hpp>
#include <glm/fwd.hpp>
#include <cstdint>
#include <limits>

namespace Voxel
{
    class World;
    class Chunk;

    // Responsable des tests de collision AABB contre le monde voxel et
    // du raycast bloc par bloc (utilisé pour l'interaction). Isolé du
    // reste de la physique du joueur pour pouvoir être réutilisé ou
    // testé indépendamment (ex: entités, autres acteurs du monde).
    class PlayerCollider
    {
    public:

        PlayerCollider(
            World& world,
            float width,
            float height
        );

        // Teste si l'AABB du joueur (centrée en largeur, posée en Y)
        // chevauche un bloc solide lorsqu'elle est placée à `position`.
        bool collidesAt(
            const glm::vec3& position
        ) const;

        // Teste si le voxel monde donné est solide (ni air, ni eau).
        bool isBlockSolid(
            int worldX,
            int worldY,
            int worldZ
        ) const;

        // Effectue un raycast bloc par bloc depuis `origin` le long de
        // `direction` (normalisée). Retourne true si un bloc solide a
        // été touché avant `maxDistance`.
        bool raycastBlock(
            const glm::vec3& origin,
            const glm::vec3& direction,
            float maxDistance,
            glm::ivec3& outHitBlock,
            glm::ivec3& outPreviousBlock
        ) const;

        float getWidth() const
        {
            return m_width;
        }

        float getHeight() const
        {
            return m_height;
        }

    private:

        World& m_world;

        float m_width;
        float m_height;

        // Cache du dernier chunk consulté lors des tests de collision,
        // pour éviter de refaire une recherche dans la table des chunks
        // du monde pour chaque voxel testé (souvent dans le même chunk).
        mutable int m_cachedChunkX = std::numeric_limits<int>::min();
        mutable int m_cachedChunkZ = std::numeric_limits<int>::min();
        mutable const Chunk* m_cachedChunk = nullptr;
    };
}
