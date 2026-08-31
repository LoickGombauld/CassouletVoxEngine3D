#pragma once


#include <vector>

namespace Voxel
{
	class VertexArray;
    class VertexBuffer;
    class IndexBuffer;

    struct Vertex
    {
        float position[3];
        float normal[3];
        float uv[2];
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