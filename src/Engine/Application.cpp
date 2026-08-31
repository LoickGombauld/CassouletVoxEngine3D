#include "Application.hpp"
#include "Window.hpp"
#include "../Renderer/Renderer.hpp"
#include "../Renderer/Shader.hpp"
#include "../Renderer/Mesh.hpp"
#include "../Input/Input.hpp"
#include "../Camera/Camera.hpp"
#include <iostream>
#include <GLFW/glfw3.h>
#include <chrono>
#include <glad/glad.h>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/matrix_clip_space.hpp>

const int WIDTH = 1280;
const int HEIGHT = 720;



namespace Voxel
{
	Application::Application() : m_window(new Window(WIDTH, HEIGHT, "Voxel Engine")), m_renderer(new Renderer())
		, m_isRunning(true), m_deltaTime(0.0f), m_input(new Input())
	{
		m_camera = std::make_unique<Camera>(glm::vec3(0.0f, 0.0f, 3.0f));
		m_input->initialize(*m_window);
		m_renderer->setViewport(m_window->getWidth(), m_window->getHeight());

		m_renderer->clear(0.08f, 0.12f, 0.18f, 1.0f);
		m_shader = std::make_unique<Shader>("assets/shaders/basic.vert", "assets/shaders/basic.frag");

		std::vector<Vertex> vertices =
		{
			// Face +Z (front)
			{ { -0.5f, -0.5f,  0.5f }, { 0.0f,  0.0f,  1.0f }, { 0.0f, 0.0f } },
			{ {  0.5f, -0.5f,  0.5f }, { 0.0f,  0.0f,  1.0f }, { 1.0f, 0.0f } },
			{ {  0.5f,  0.5f,  0.5f }, { 0.0f,  0.0f,  1.0f }, { 1.0f, 1.0f } },
			{ { -0.5f,  0.5f,  0.5f }, { 0.0f,  0.0f,  1.0f }, { 0.0f, 1.0f } },

			// Face -Z (back)
			{ {  0.5f, -0.5f, -0.5f }, { 0.0f,  0.0f, -1.0f }, { 0.0f, 0.0f } },
			{ { -0.5f, -0.5f, -0.5f }, { 0.0f,  0.0f, -1.0f }, { 1.0f, 0.0f } },
			{ { -0.5f,  0.5f, -0.5f }, { 0.0f,  0.0f, -1.0f }, { 1.0f, 1.0f } },
			{ {  0.5f,  0.5f, -0.5f }, { 0.0f,  0.0f, -1.0f }, { 0.0f, 1.0f } },

			// Face +Y (top)
			{ { -0.5f,  0.5f,  0.5f }, { 0.0f,  1.0f,  0.0f }, { 0.0f, 0.0f } },
			{ {  0.5f,  0.5f,  0.5f }, { 0.0f,  1.0f,  0.0f }, { 1.0f, 0.0f } },
			{ {  0.5f,  0.5f, -0.5f }, { 0.0f,  1.0f,  0.0f }, { 1.0f, 1.0f } },
			{ { -0.5f,  0.5f, -0.5f }, { 0.0f,  1.0f,  0.0f }, { 0.0f, 1.0f } },

			// Face -Y (bottom)
			{ { -0.5f, -0.5f, -0.5f }, { 0.0f, -1.0f,  0.0f }, { 0.0f, 0.0f } },
			{ {  0.5f, -0.5f, -0.5f }, { 0.0f, -1.0f,  0.0f }, { 1.0f, 0.0f } },
			{ {  0.5f, -0.5f,  0.5f }, { 0.0f, -1.0f,  0.0f }, { 1.0f, 1.0f } },
			{ { -0.5f, -0.5f,  0.5f }, { 0.0f, -1.0f,  0.0f }, { 0.0f, 1.0f } },

			// Face +X (right)
			{ {  0.5f, -0.5f,  0.5f }, { 1.0f,  0.0f,  0.0f }, { 0.0f, 0.0f } },
			{ {  0.5f, -0.5f, -0.5f }, { 1.0f,  0.0f,  0.0f }, { 1.0f, 0.0f } },
			{ {  0.5f,  0.5f, -0.5f }, { 1.0f,  0.0f,  0.0f }, { 1.0f, 1.0f } },
			{ {  0.5f,  0.5f,  0.5f }, { 1.0f,  0.0f,  0.0f }, { 0.0f, 1.0f } },

			// Face -X (left)
			{ { -0.5f, -0.5f, -0.5f }, { -1.0f, 0.0f,  0.0f }, { 0.0f, 0.0f } },
			{ { -0.5f, -0.5f,  0.5f }, { -1.0f, 0.0f,  0.0f }, { 1.0f, 0.0f } },
			{ { -0.5f,  0.5f,  0.5f }, { -1.0f, 0.0f,  0.0f }, { 1.0f, 1.0f } },
			{ { -0.5f,  0.5f, -0.5f }, { -1.0f, 0.0f,  0.0f }, { 0.0f, 1.0f } }
		};

		std::vector<unsigned int> indices =
		{
			// front
			0, 1, 2, 2, 3, 0,
			// back
			4, 5, 6, 6, 7, 4,
			// top
			8, 9,10,10,11, 8,
			// bottom
		   12,13,14,14,15,12,
		   // right
		  16,17,18,18,19,16,
		  // left
		 20,21,22,22,23,20
		};
		m_mesh = std::make_unique<Mesh>(vertices, indices);
	}

	Application::~Application()
	{
	}

	void Application::processInput()
	{
	}

	void Application::run()
	{
		using Clock = std::chrono::high_resolution_clock;

		auto lastTime = Clock::now();

		while (m_isRunning && !m_window->shouldClose())
		{
			auto currentTime = Clock::now();

			std::chrono::duration<float> elapsed =
				currentTime - lastTime;

			lastTime = currentTime;

			m_deltaTime = elapsed.count();

			// Évite un énorme deltaTime après un freeze,
			// un breakpoint ou une perte de focus.
			if (m_deltaTime > 0.1f)
				m_deltaTime = 0.1f;

			Input::update();

			processInput();

			update(m_deltaTime);

			render();

			endFrame();
		}
	}
	void Application::update(float deltaTime)
	{
		// Mise à jour de la simulation.
		//
		// Plus tard :
		// - joueur
		// - physique
		// - entités
		// - génération des chunks
		// - chargement/déchargement du monde
		// - animations

		m_camera->update(deltaTime);
	}

	void Application::render()
	{
		m_renderer->beginFrame();
		m_shader->bind(); 

		glm::mat4 model = glm::mat4(1.0f);

		glm::mat4 view = m_camera->getViewMatrix();

		const float aspectRatio = static_cast<float>(m_window->getWidth()) / static_cast<float>(m_window->getHeight());

		glm::mat4 projection = m_camera->getProjectionMatrix(aspectRatio);

		m_shader->setMat4("u_Model", model);
		m_shader->setMat4("u_View", view);
		m_shader->setMat4("u_Projection", projection);

		m_mesh->draw();

		m_shader->unbind();

		m_renderer->endFrame();
	}

	void Application::endFrame()
	{
		m_window->swapBuffers();
		m_window->pollEvents();
	}
}