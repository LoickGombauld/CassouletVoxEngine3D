#include "Application.hpp"
#include "Window.hpp"
#include "../Renderer/Renderer.hpp"
#include "../Renderer/Shader.hpp"
#include "../Renderer/Mesh.hpp"
#include "../Renderer/Texture.hpp"
#include "../Input/Input.hpp"
#include "../Camera/Camera.hpp"
#include "../World/World.hpp"
#include "../World/WorldGenerationSettings.hpp"
#include "../Voxel/VoxelMesher.hpp"
#include "../Voxel/Chunk.hpp"
#include "../GPU/NoiseCompute.hpp"
#include "../Player/Player.hpp"
#include "../Player/GameManager.hpp"
#include <GLFW/glfw3.h>
#include <chrono>
#include <iomanip>
#include <glad/glad.h>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <stb_image.h>
#include "Profiler.hpp"


const int WIDTH = 1280;
const int HEIGHT = 720;



namespace Voxel
{
	Application::Application() : m_window(new Window(WIDTH, HEIGHT, "Voxel Engine")), m_renderer(new Renderer())
		, m_isRunning(true), m_deltaTime(0.0f), m_input(new Input()),
		m_textureAtlas(std::make_unique<Texture>("assets/textures/atlas.png", false))
	{
		m_camera = std::make_unique<Camera>(glm::vec3(0.0f, Chunk::HEIGHT, 3.0f));
		m_gameManager = std::make_unique<GameManager>();
		m_input->initialize(*m_window);
		m_renderer->setViewport(m_window->getWidth(), m_window->getHeight());

		m_renderer->clear(0.08f, 0.12f, 0.18f, 1.0f);
		m_shader = std::make_unique<Shader>("assets/shaders/basic.vert", "assets/shaders/basic.frag");
		m_textureAtlas->bind(0);
		m_shader->setInt("u_TextureAtlas", 0);
		m_world = std::make_unique<World>(375);
		m_world->generate();

		m_player = std::make_unique<Player>(
			*m_world,
			*m_camera,
			glm::vec3(0.0f, static_cast<float>(Chunk::HEIGHT), 3.0f)
		);

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
		float displayTimer = 0.0f;
		int displayedFrames = 0;

		while (m_isRunning && !m_window->shouldClose())
		{
			auto currentTime = Clock::now();

			std::chrono::duration<float> elapsed =
				currentTime - lastTime;

			lastTime = currentTime;

			m_deltaTime = elapsed.count();
			displayTimer += m_deltaTime;
			++displayedFrames;

			// Évite un énorme deltaTime après un freeze,
			// un breakpoint ou une perte de focus.
			if (m_deltaTime > 0.1f)
				m_deltaTime = 0.1f;

			Input::update();

			processInput();

			update(m_deltaTime);

			if (displayTimer >= 0.25f)
			{
				const glm::vec3& position = m_camera->getPosition();

				std::cout
					<< '\r'
					<< std::fixed
					<< std::setprecision(1)
					<< " | Camera: ("
					<< position.x << ", "
					<< position.y << ", "
					<< position.z << ")    "
					<< std::flush;

				displayTimer = 0.0f;
				displayedFrames = 0;
			}

			render();

			endFrame();
		}

		std::cout << '\n';
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

		profilerTimer += deltaTime;

		frameCount++;
		fpsTimer += deltaTime;

		if (fpsTimer >= 1.0)
		{
			std::cout << std::fixed << std::setprecision(1) << "FPS: " << frameCount << '\n';

			frameCount = 0;
			fpsTimer = 0.0;
		}
		if (profilerTimer >= 1.0)
		{
			Profiler::instance().print();
			Profiler::instance().reset();

			profilerTimer = 0.0;
		}

		m_camera->update(deltaTime);
		m_player->update(deltaTime);
		PROFILE_SCOPE("World::update");
		m_world->updateStreaming(
			m_player->getPosition()
		);
		m_world->updateLodStreaming(
			m_player->getPosition()
		);


	}

	void Application::render()
	{
		const int width = m_window->getWidth();
		const int height = m_window->getHeight();

		PROFILE_SCOPE("Rendering");

		if (width <= 0 || height <= 0)
			return;

		m_renderer->setViewport(width, height);
		m_renderer->beginFrame();

		m_shader->bind();
		m_textureAtlas->bind();

		const glm::mat4 model = glm::mat4(1.0f);
		const glm::mat4 view = m_camera->getViewMatrix();

		const float aspectRatio =
			static_cast<float>(width) /
			static_cast<float>(height);

		const glm::mat4 projection =
			m_camera->getProjectionMatrix(aspectRatio);

		m_shader->setMat4("u_Model", model);
		m_shader->setMat4("u_View", view);
		m_shader->setMat4("u_Projection", projection);
		m_shader->setInt("u_TextureAtlas", 0);

		m_shader->setInt("u_RenderWater", 0);
		m_world->render(
			view,
			projection,
			*m_shader,
			m_camera->getPosition()
		);

		glDepthMask(GL_FALSE);
		m_shader->setInt("u_RenderWater", 1);
		m_world->render(
			view,
			projection,
			*m_shader,
			m_camera->getPosition()
		);
		glDepthMask(GL_TRUE);

		m_shader->unbind();

		m_renderer->endFrame();
	}
	void Application::endFrame()
	{
		m_window->swapBuffers();
		m_window->pollEvents();
	}
}