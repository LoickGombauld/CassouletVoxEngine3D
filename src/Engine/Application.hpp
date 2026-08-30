#pragma once

#ifdef _WIN32
#   include <Windows.h>
#endif


namespace Voxel
{
	class Window;
	class Renderer;

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

		bool m_isRunning = true;

		float m_lastFrameTime = 0.0f;
		float m_deltaTime = 0.0f;
	};
}