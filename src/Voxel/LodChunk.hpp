#pragma once

#include <memory>
#include <vector>
#include <glm/vec3.hpp>

namespace Voxel
{
    class Mesh;
    class WorldGenerator;

    // Représentation CPU simplifiée d'un chunk lointain, utilisée pour le
    // rendu LOD (Level Of Detail) façon Voxy. Un LodChunk ne stocke pas de
    // voxels : uniquement une grille de hauteurs échantillonnées, à partir
    // de laquelle un maillage simplifié (LodMesh) est généré.
    class LodChunk
    {
    public:

        // Nombre d'échantillons de hauteur par côté du chunk LOD.
        static constexpr int GRID_SIZE = 9;

        struct LodVertex
        {
            glm::vec3 position;
            glm::vec3 normal;
        };

        // Données de maillage simplifiées, générées côté CPU et prêtes à
        // être uploadées sur le thread principal (GPU) sous forme de Mesh.
        struct LodMeshData
        {
            std::vector<LodVertex> vertices;
            std::vector<unsigned int> indices;

            bool empty() const
            {
                return vertices.empty();
            }
        };

        LodChunk(
            int chunkX,
            int chunkZ,
            int lodStep
        );

        ~LodChunk() = default;

        LodChunk(const LodChunk&) = delete;
        LodChunk& operator=(const LodChunk&) = delete;

        // Génère la grille de hauteurs échantillonnées à partir du
        // générateur de terrain (2D uniquement, pas de voxels).
        void generateHeights(
            const WorldGenerator& generator
        );

        // Construit les données de maillage simplifiées (une grille de
        // quads/triangles suivant les hauteurs échantillonnées).
        LodMeshData buildMeshData() const;

        void setMesh(
            std::unique_ptr<Mesh> mesh
        );

        Mesh* getMesh() const
        {
            return m_mesh.get();
        }

        int getChunkX() const
        {
            return m_chunkX;
        }

        int getChunkZ() const
        {
            return m_chunkZ;
        }

        int getLodStep() const
        {
            return m_lodStep;
        }

        float getHeight(
            int sampleX,
            int sampleZ
        ) const;

    private:

        int m_chunkX;
        int m_chunkZ;

        // Espacement en blocs entre deux échantillons (résolution LOD).
        int m_lodStep;

        std::vector<float> m_heights;

        std::unique_ptr<Mesh> m_mesh;
    };
}
