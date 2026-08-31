#include "../Input/Input.hpp"

#include "../Engine/Window.hpp"

#include <GLFW/glfw3.h>

namespace Voxel
{
    Window* Input::s_window = nullptr;

    double Input::s_mouseX = 0.0;
    double Input::s_mouseY = 0.0;

    double Input::s_lastMouseX = 0.0;
    double Input::s_lastMouseY = 0.0;

    double Input::s_deltaX = 0.0;
    double Input::s_deltaY = 0.0;

    bool Input::s_firstMouse = true;
    bool Input::s_mouseCaptured = false;

    void Input::initialize(
        Window& window
    )
    {
        s_window = &window;

        GLFWwindow* native =
            window.getNativeWindow();

        glfwGetCursorPos(
            native,
            &s_lastMouseX,
            &s_lastMouseY
        );
    }

    void Input::update()
    {
        if (!s_window)
            return;

        GLFWwindow* native =
            s_window->getNativeWindow();

        s_deltaX = 0.0;
        s_deltaY = 0.0;

        if (!s_mouseCaptured)
            return;

        double mouseX;
        double mouseY;

        glfwGetCursorPos(
            native,
            &mouseX,
            &mouseY
        );

        if (s_firstMouse)
        {
            s_lastMouseX = mouseX;
            s_lastMouseY = mouseY;

            s_firstMouse = false;

            return;
        }

        s_deltaX =
            mouseX - s_lastMouseX;

        s_deltaY =
            mouseY - s_lastMouseY;

        s_lastMouseX = mouseX;
        s_lastMouseY = mouseY;
    }

    bool Input::isKeyDown(
        int key
    )
    {
        if (!s_window)
            return false;

        return glfwGetKey(
            s_window->getNativeWindow(),
            key
        ) == GLFW_PRESS;
    }

    bool Input::isMouseButtonDown(
        int button
    )
    {
        if (!s_window)
            return false;

        return glfwGetMouseButton(
            s_window->getNativeWindow(),
            button
        ) == GLFW_PRESS;
    }

    double Input::getMouseX()
    {
        return s_lastMouseX;
    }

    double Input::getMouseY()
    {
        return s_lastMouseY;
    }

    double Input::getMouseDeltaX()
    {
        return s_deltaX;
    }

    double Input::getMouseDeltaY()
    {
        return s_deltaY;
    }

    void Input::setMouseCaptured(
        bool captured
    )
    {
        if (!s_window)
            return;

        GLFWwindow* native =
            s_window->getNativeWindow();

        s_mouseCaptured = captured;

        s_firstMouse = false;

        glfwSetInputMode(
            native,
            GLFW_CURSOR,
            captured
            ? GLFW_CURSOR_DISABLED
            : GLFW_CURSOR_NORMAL
        );

        if (s_mouseCaptured) {
            // Placer la souris au centre de la fenêtre et synchroniser l'état interne.
            int width, height;
            glfwGetWindowSize(native, &width, &height);
            double centerX = width / 2.0;
            double centerY = height / 2.0;
            glfwSetCursorPos(native, centerX, centerY);
            s_lastMouseX = centerX;
            s_lastMouseY = centerY;
        }
    }

    bool Input::isMouseCaptured()
    {
        return s_mouseCaptured;
    }
}