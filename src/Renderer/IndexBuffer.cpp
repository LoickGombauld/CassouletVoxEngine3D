
#include "../Renderer/IndexBuffer.hpp"

#include <glad/glad.h>

namespace Voxel
{
    IndexBuffer::IndexBuffer(
        const unsigned int* indices,
        unsigned int count
    )
        : m_count(count)
    {
        glGenBuffers(
            1,
            &m_id
        );

        glBindBuffer(
            GL_ELEMENT_ARRAY_BUFFER,
            m_id
        );

        glBufferData(
            GL_ELEMENT_ARRAY_BUFFER,
            count * sizeof(unsigned int),
            indices,
            GL_STATIC_DRAW
        );

        glBindBuffer(
            GL_ELEMENT_ARRAY_BUFFER,
            0
        );
    }

    IndexBuffer::~IndexBuffer()
    {
        glDeleteBuffers(
            1,
            &m_id
        );
    }

    void IndexBuffer::bind() const
    {
        glBindBuffer(
            GL_ELEMENT_ARRAY_BUFFER,
            m_id
        );
    }

    void IndexBuffer::unbind() const
    {
        glBindBuffer(
            GL_ELEMENT_ARRAY_BUFFER,
            0
        );
    }

    unsigned int IndexBuffer::getCount() const
    {
        return m_count;
    }
}