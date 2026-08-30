#pragma once

namespace Voxel
{
    class Renderer
    {
    public:

        Renderer();

        void beginFrame();
        void endFrame();

        void setViewport(
            int width,
            int height
        );

        void clear(
            float r,
            float g,
            float b,
            float a
        );
    };
}
