
#include "../Renderer/VertexArray.hpp"

#include <glad/glad.h>

namespace Voxel
{
    VertexArray::VertexArray()
    {
        glGenVertexArrays(
            1,
            &m_id
        );
    }

    VertexArray::~VertexArray()
    {
        glDeleteVertexArrays(
            1,
            &m_id
        );
    }

    void VertexArray::bind() const
    {
        glBindVertexArray(m_id);
    }

    void VertexArray::unbind() const
    {
        glBindVertexArray(0);
    }

    void VertexArray::setAttribute(
        unsigned int index,
        int count,
        unsigned int type,
        bool normalized,
        unsigned int stride,
        const void* offset
    )
    {
        glEnableVertexAttribArray(index);

        glVertexAttribPointer(
            index,
            count,
            type,
            normalized ? GL_TRUE : GL_FALSE,
            stride,
            offset
        );
    }
}