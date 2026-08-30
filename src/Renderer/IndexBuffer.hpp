#pragma once

namespace Voxel
{
    class IndexBuffer
    {
    public:

        IndexBuffer(
            const unsigned int* indices,
            unsigned int count
        );

        ~IndexBuffer();

        IndexBuffer(const IndexBuffer&) = delete;
        IndexBuffer& operator=(
            const IndexBuffer&
            ) = delete;

        void bind() const;
        void unbind() const;

        unsigned int getCount() const;

    private:

        unsigned int m_id = 0;
        unsigned int m_count = 0;
    };
}