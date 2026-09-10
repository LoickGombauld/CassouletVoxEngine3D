
#include "../Camera/Camera.hpp"

#include "../Input/Input.hpp"

#include <GLFW/glfw3.h>

#include <glm/gtc/matrix_transform.hpp>

namespace Voxel
{
    Camera::Camera()
        : m_position(0.0f, 10.0f, 5.0f),
        m_front(0.0f, 0.0f, -1.0f),
        m_up(0.0f, 1.0f, 0.0f),
        m_right(1.0f, 0.0f, 0.0f),
        m_yaw(0),
        m_pitch(0.0f),
        m_fov(70.0f),
        m_nearPlane(0.1f),
        m_farPlane(1000.0f),
        m_moveSpeed(10.0f),
        m_mouseSensitivity(0.1f)
    {
        updateVectors();
    }

    Camera::Camera(
        const glm::vec3& position
    )
        : Camera()
    {
        m_position = position;
    }

    void Camera::update(
        float deltaTime
    )
    {
        // ------------------------------------------------
        // Mouse
        // ------------------------------------------------

        if(Input::isMouseButtonDown(GLFW_MOUSE_BUTTON_RIGHT))
        {
            Input::setMouseCaptured(true);
        }
        else
        {
            Input::setMouseCaptured(false);
		}


        if (Input::isMouseCaptured())
        {
            const float mouseX =
                static_cast<float>(
                    Input::getMouseDeltaX()
                    );

            const float mouseY =
                static_cast<float>(
                    Input::getMouseDeltaY()
                    );

            m_yaw +=
                mouseX *
                m_mouseSensitivity;

            m_pitch -=
                mouseY *
                m_mouseSensitivity;

            // Empêche de retourner complètement
            // la caméra.
            if (m_pitch > 89.0f)
                m_pitch = 89.0f;

            if (m_pitch < -89.0f)
                m_pitch = -89.0f;

            updateVectors();
        }

        // ------------------------------------------------
        // Keyboard
        // ------------------------------------------------

        float speed =
            m_moveSpeed * deltaTime;

        if (Input::isKeyDown(GLFW_KEY_LEFT_SHIFT))
        {
            speed *= 2.0f;
        }

        if (Input::isKeyDown(GLFW_KEY_W))
        {
            m_position +=
                m_front * speed;
        }

        if (Input::isKeyDown(GLFW_KEY_S))
        {
            m_position -=
                m_front * speed;
        }

        if (Input::isKeyDown(GLFW_KEY_D))
        {
            m_position +=
                m_right * speed;
        }

        if (Input::isKeyDown(GLFW_KEY_A))
        {
            m_position -=
                m_right * speed;
        }

        if (Input::isKeyDown(GLFW_KEY_SPACE))
        {
            m_position.y += speed;
        }

        if (Input::isKeyDown(GLFW_KEY_LEFT_CONTROL))
        {
            m_position.y -= speed;
        }
    }

    void Camera::updateVectors()
    {
        glm::vec3 direction;

        direction.x =
            glm::cos(
                glm::radians(m_yaw)
            ) *
            glm::cos(
                glm::radians(m_pitch)
            );

        direction.y =
            glm::sin(
                glm::radians(m_pitch)
            );

        direction.z =
            glm::sin(
                glm::radians(m_yaw)
            ) *
            glm::cos(
                glm::radians(m_pitch)
            );

        m_front =
            glm::normalize(direction);

        m_right =
            glm::normalize(
                glm::cross(
                    m_front,
                    glm::vec3(
                        0.0f,
                        1.0f,
                        0.0f
                    )
                )
            );

        m_up =
            glm::normalize(
                glm::cross(
                    m_right,
                    m_front
                )
            );
    }

    glm::mat4 Camera::getViewMatrix() const
    {
        return glm::lookAt(
            m_position,
            m_position + m_front,
            m_up
        );
    }

    glm::mat4 Camera::getProjectionMatrix(
        float aspectRatio
    ) const
    {
        return glm::perspective(
            glm::radians(m_fov),
            aspectRatio,
            m_nearPlane,
            m_farPlane
        );
    }

    const glm::vec3& Camera::getPosition() const
    {
        return m_position;
    }

    void Camera::setPosition(
        const glm::vec3& position
    )
    {
        m_position = position;
    }

    float Camera::getYaw() const
    {
        return m_yaw;
    }

    float Camera::getPitch() const
    {
        return m_pitch;
    }

    void Camera::setFOV(
        float fov
    )
    {
        m_fov = fov;
    }
}