
#include "Renderer.hpp"
#include <glad/glad.h>

namespace Voxel
{
    Renderer::Renderer()
    {
        glEnable(GL_DEPTH_TEST);

        glEnable(GL_CULL_FACE);

        glCullFace(GL_BACK);

        glFrontFace(GL_CCW);
    }

    void Renderer::beginFrame()
    {
        glClear(
            GL_COLOR_BUFFER_BIT |
            GL_DEPTH_BUFFER_BIT
        );
    }

    void Renderer::endFrame()
    {
        // Pour l'instant rien.
        //
        // Plus tard :
        // - GPU synchronization
        // - debug markers
        // - statistiques
    }

    void Renderer::setViewport(
        int width,
        int height
    )
    {
        glViewport(
            0,
            0,
            width,
            height
        );
    }

    void Renderer::clear(
        float r,
        float g,
        float b,
        float a
    )
    {
        glClearColor(
            r,
            g,
            b,
            a
        );
    }
}
