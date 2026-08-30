#pragma once


#include <vector>

namespace Voxel
{
	class VertexArray;
    class VertexBuffer;
    class IndexBuffer;

    struct Vertex
    {
        float x;
        float y;
        float z;
    };

    class Mesh
    {
    public:

        Mesh(
            const std::vector<Vertex>& vertices,
            const std::vector<unsigned int>& indices
        );

        void bind() const;
        void unbind() const;

        unsigned int getIndexCount() const;

    private:

        VertexArray* m_vertexArray;
        VertexBuffer* m_vertexBuffer;
        IndexBuffer* m_indexBuffer;
    };
}
