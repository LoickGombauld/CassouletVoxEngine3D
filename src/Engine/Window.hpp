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

        GLFWwindow* m_window = nullptr;
		float m_width;
		float m_height;
    };
}