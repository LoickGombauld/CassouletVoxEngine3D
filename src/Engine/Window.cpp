#include "Window.hpp"
#include <iostream>
#include <GLFW/glfw3.h>
#include <glad/glad.h>

namespace Voxel
{
	Window::Window(int width, int height, const char* title) : m_width(width), m_height(height)

	{
		if (!glfwInit())
		{
			throw std::runtime_error(
				"Failed to initialize GLFW."
			);
		}

		// OpenGL 4.6 Core Profile
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);

		glfwWindowHint(
			GLFW_OPENGL_PROFILE,
			GLFW_OPENGL_CORE_PROFILE
		);

#ifdef __APPLE__
		glfwWindowHint(
			GLFW_OPENGL_FORWARD_COMPAT,
			GLFW_TRUE
		);
#endif

		m_window = glfwCreateWindow(
			m_width,
			m_height,
			title,
			nullptr,
			nullptr
		);

		if (!m_window)
		{
			glfwTerminate();

			throw std::runtime_error(
				"Failed to create GLFW window."
			);
		}

		glfwMakeContextCurrent(m_window);

		// VSync
		glfwSwapInterval(1);

		// GLAD doit être chargé après la création
		// et l'activation du contexte OpenGL.
		int version = gladLoadGL();

		if (version == 0)
		{
			glfwDestroyWindow(m_window);
			glfwTerminate();

			throw std::runtime_error(
				"Failed to initialize GLAD."
			);
		}
	}

	Window::~Window()
	{
		if (m_window) {
			glfwDestroyWindow(m_window);
			m_window = nullptr;
		}
		glfwTerminate();
	}

	void Window::pollEvents()
	{
		glfwPollEvents();
	}

	void Window::swapBuffers()
	{
		glfwSwapBuffers(m_window);
	}

	bool Window::shouldClose() const
	{
		return glfwWindowShouldClose(m_window);
	}

	GLFWwindow* Window::getNativeWindow() const
	{
		return m_window;
	}

	int Window::getWidth() const
	{
		return m_width;
	}

	int Window::getHeight() const
	{
		return m_height;
	}

}
