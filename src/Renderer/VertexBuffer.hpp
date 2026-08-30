#pragma once

namespace Voxel
{
    class VertexBuffer
    {
    public:

        VertexBuffer(
            const void* data,
            unsigned int size
        );

        ~VertexBuffer();

        VertexBuffer(const VertexBuffer&) = delete;
        VertexBuffer& operator=(
            const VertexBuffer&
            ) = delete;

        void bind() const;
        void unbind() const;

        unsigned int getID() const;

    private:

        unsigned int m_id = 0;
    };
}