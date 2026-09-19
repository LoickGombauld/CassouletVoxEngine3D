#pragma once
#pragma once

#ifdef _WIN32
#   include <Windows.h>
#endif
#include <memory>

namespace Voxel
{
	class World;
	class Window;
	class Renderer;
	class Shader;
	class Mesh;
	class Input;
	class Camera;
	class Texture;
	class Player;
	class GameManager;


	class Application
	{
	public:

		Application();

		~Application();

		void run();

		bool isRunning() const { return m_isRunning; }

	private:
		void processInput();
		void update(float deltaTime);
		void render();
		void endFrame();

		Window* m_window;

		Renderer* m_renderer;

		Input* m_input;

		bool m_isRunning = true;

		float m_lastFrameTime = 0.0f;
		float m_deltaTime = 0.0f;

		std::unique_ptr<Shader> m_shader;
		std::unique_ptr<Camera> m_camera;
		std::unique_ptr<World> m_world;
		std::unique_ptr<Texture> m_textureAtlas;
		std::unique_ptr<Player> m_player;
		std::unique_ptr<GameManager> m_gameManager;
	};
}