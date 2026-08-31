
#include <glm/gtc/matrix_transform.hpp>
#include "../World/World.hpp"
#include "../Renderer/Shader.hpp"


namespace Voxel
{
    World::World() = default;

    long long World::makeChunkKey(
        int x,
        int z
    )
    {
        return
            (static_cast<long long>(x) << 32) ^
            static_cast<unsigned int>(z);
    }

    Chunk* World::getChunk(
        int chunkX,
        int chunkZ
    )
    {
        const long long key =
            makeChunkKey(
                chunkX,
                chunkZ
            );

        auto iterator =
            m_chunks.find(key);

        if (iterator == m_chunks.end())
            return nullptr;

        return iterator->second.get();
    }

    void World::generate()
    {
        constexpr int RADIUS = 2;

        for (
            int x = -RADIUS;
            x <= RADIUS;
            ++x
            )
        {
            for (
                int z = -RADIUS;
                z <= RADIUS;
                ++z
                )
            {
                auto chunk =
                    std::make_unique<Chunk>(
                        x,
                        z
                    );

                chunk->generateTestTerrain();

                const long long key =
                    makeChunkKey(
                        x,
                        z
                    );

                m_chunks.emplace(
                    key,
                    std::move(chunk)
                );
            }
        }
    }

    void World::render(
        const glm::mat4& view,
        const glm::mat4& projection,
        Shader& shader
    )
    {
        for (auto& [key, chunk] : m_chunks)
        {
            glm::mat4 model =
                glm::translate(
                    glm::mat4(1.0f),
                    chunk->getWorldPosition()
                );

            shader.setMat4(
                "u_Model",
                model
            );

            shader.setMat4(
                "u_View",
                view
            );

            shader.setMat4(
                "u_Projection",
                projection
            );

            chunk->render();
        }
    }
}