#include "Application.hpp"
#include "Window.hpp"
#include "../Renderer/Renderer.hpp"
#include <iostream>
#include <GLFW/glfw3.h>
#include <chrono>
#include <glad/glad.h>

const int WIDTH = 1280;
const int HEIGHT = 720;



namespace Voxel
{
	Application::Application() : m_window(new Window(WIDTH, HEIGHT, "Voxel Engine")), m_renderer(new Renderer())
		, m_isRunning(true), m_deltaTime(0.0f)
	{
		m_renderer->setViewport(m_window->getWidth(), m_window->getHeight());

		m_renderer->clear(0.08f, 0.12f, 0.18f, 1.0f);
	}

	Application::~Application()
	{
	}

	void Application::processInput()
	{

		if (!m_window->getNativeWindow())
		{
			return;
		}

		// Fermer la fenêtre si ESC est pressé
		if (glfwGetKey(m_window->getNativeWindow(), GLFW_KEY_ESCAPE) == GLFW_PRESS)
		{
			glfwSetWindowShouldClose(m_window->getNativeWindow(), GLFW_TRUE);
		}

		// Exemples d'inputs de mouvement -- remplacer par gestionnaire d'événements si nécessaire
		if (glfwGetKey(m_window->getNativeWindow(), GLFW_KEY_W) == GLFW_PRESS)
		{
			std::cout << "[Input] W pressed\n";
		}
		if (glfwGetKey(m_window->getNativeWindow(), GLFW_KEY_S) == GLFW_PRESS)
		{
			std::cout << "[Input] S pressed\n";
		}
		if (glfwGetKey(m_window->getNativeWindow(), GLFW_KEY_A) == GLFW_PRESS)
		{
			std::cout << "[Input] A pressed\n";
		}
		if (glfwGetKey(m_window->getNativeWindow(), GLFW_KEY_D) == GLFW_PRESS)
		{
			std::cout << "[Input] D pressed\n";
		}
		if (glfwGetKey(m_window->getNativeWindow(), GLFW_KEY_UP) == GLFW_PRESS)
		{
			std::cout << "[Input] Up pressed\n";
		}
		if (glfwGetKey(m_window->getNativeWindow(), GLFW_KEY_DOWN) == GLFW_PRESS)
		{
			std::cout << "[Input] Down pressed\n";
		}
		if (glfwGetKey(m_window->getNativeWindow(), GLFW_KEY_LEFT) == GLFW_PRESS)
		{
			std::cout << "[Input] Left pressed\n";
		}
		if (glfwGetKey(m_window->getNativeWindow(), GLFW_KEY_RIGHT) == GLFW_PRESS)
		{
			std::cout << "[Input] Right pressed\n";
		}
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
		//
		// Exemple :
		//
		// m_world.update(deltaTime);
		// m_player.update(deltaTime);
	}

	void Application::render()
	{
		m_renderer->beginFrame();
		// Rendu graphique.
		//
		// Plus tard :
		// m_world.render();
		m_renderer->endFrame();
	}

	void Application::endFrame()
	{
		m_window->swapBuffers();
		m_window->pollEvents();
	}
}