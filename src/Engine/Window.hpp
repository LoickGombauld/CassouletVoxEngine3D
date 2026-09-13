#pragma once

class GLFWwindow;

namespace Voxel
{
    class Window
    {
    public:

        Window(
            int width,
            int height,
            const char* title
        );

        ~Window();

        Window(const Window&) = delete;
        Window& operator=(const Window&) = delete;

        void pollEvents();
        void swapBuffers();

        bool shouldClose() const;

        GLFWwindow* getNativeWindow() const;

        int getWidth() const;
        int getHeight() const;

    private:

        static void framebufferSizeCallback(
            GLFWwindow* window,
            int width,
            int height
        );

        GLFWwindow* m_window = nullptr;
        int m_width = 0;
        int m_height = 0;
    };
}