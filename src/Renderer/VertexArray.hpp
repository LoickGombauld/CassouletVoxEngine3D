#pragma once

namespace Voxel
{
    class VertexArray
    {
    public:

        VertexArray();

        ~VertexArray();

        VertexArray(const VertexArray&) = delete;
        VertexArray& operator=(
            const VertexArray&
            ) = delete;

        void bind() const;
        void unbind() const;

        void setAttribute(
            unsigned int index,
            int count,
            unsigned int type,
            bool normalized,
            unsigned int stride,
            const void* offset
        );

    private:

        unsigned int m_id = 0;
    };
}