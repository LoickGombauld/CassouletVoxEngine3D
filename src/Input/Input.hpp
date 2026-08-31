#pragma once

namespace Voxel
{
    class Window;

    class Input
    {
    public:

        static void initialize(
            Window& window
        );

        static void update();

        static bool isKeyDown(
            int key
        );

        static bool isMouseButtonDown(
            int button
        );

        static double getMouseX();

        static double getMouseY();

        static double getMouseDeltaX();

        static double getMouseDeltaY();

        static void setMouseCaptured(
            bool captured
        );

        static bool isMouseCaptured();

    private:

        static Window* s_window;

        static double s_mouseX;
        static double s_mouseY;

        static double s_lastMouseX;
        static double s_lastMouseY;

        static double s_deltaX;
        static double s_deltaY;

        static bool s_firstMouse;

        static bool s_mouseCaptured;
    };
}
