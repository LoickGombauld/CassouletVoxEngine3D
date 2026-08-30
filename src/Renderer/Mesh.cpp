#include "../Renderer/Mesh.hpp"
#include "../Renderer/VertexArray.hpp"
#include "../Renderer/VertexBuffer.hpp"
#include "../Renderer/IndexBuffer.hpp"
#include <glad/glad.h>

namespace Voxel
{
    Mesh::Mesh(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices )
        : 
        m_vertexArray(new VertexArray()),
        m_vertexBuffer(new VertexBuffer(vertices.data(), static_cast<unsigned int>(vertices.size() * sizeof(Vertex)))),
        m_indexBuffer(new IndexBuffer(indices.data(), static_cast<unsigned int>(indices.size())))
    {
        m_vertexArray->bind();

        m_vertexBuffer->bind();
        
        m_vertexArray->setAttribute(
            0,
            3,
            GL_FLOAT,
            false,
            sizeof(Vertex),
            reinterpret_cast<const void*>(offsetof(Vertex, position))
        );

        m_vertexArray->setAttribute(
            1, 
            3,
            GL_FLOAT,
            false, 
            sizeof(Vertex),
            reinterpret_cast<const void*>(offsetof(Vertex, normal))
        );

        m_vertexArray->setAttribute(
            2,
            2, 
            GL_FLOAT, 
            false, 
            sizeof(Vertex), 
            reinterpret_cast<const void*>(offsetof(Vertex, uv))
        );

        m_indexBuffer->bind();

        m_vertexArray->unbind();
    }

    void Mesh::bind() const
    {
        m_vertexArray->bind();
    }

    void Mesh::unbind() const
    {
        m_vertexArray->unbind();
    }

    unsigned int Mesh::getIndexCount() const
    {
        return m_indexBuffer->getCount();
    }
}