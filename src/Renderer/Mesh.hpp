#pragma once
#include <glad/glad.h>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

#include <vector>

namespace Voxel
{
	class VertexArray;
    class VertexBuffer;
    class IndexBuffer;

    struct Vertex
    {
        glm::vec3 position;
        glm::vec3 normal;
        glm::vec2 uv;

        float ao;

        float textureIndex;
    };

    class Mesh
    {
    public:

        Mesh(
            const std::vector<Vertex>& vertices,
            const std::vector<unsigned int>& indices
        );

        void draw() const;

        unsigned int getIndexCount() const;

    private:

        VertexArray* m_vertexArray;
        VertexBuffer* m_vertexBuffer;
        IndexBuffer* m_indexBuffer;
    };
}