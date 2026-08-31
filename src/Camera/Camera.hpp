#pragma once

#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>

namespace Voxel
{
    class Camera
    {
    public:

        Camera();

        explicit Camera(
            const glm::vec3& position
        );

        void update(
            float deltaTime
        );

        glm::mat4 getViewMatrix() const;

        glm::mat4 getProjectionMatrix(
            float aspectRatio
        ) const;

        const glm::vec3& getPosition() const;

        void setPosition(
            const glm::vec3& position
        );

        float getYaw() const;
        float getPitch() const;

        void setFOV(float fov);

    private:

        void updateVectors();

    private:

        glm::vec3 m_position;

        glm::vec3 m_front;
        glm::vec3 m_up;
        glm::vec3 m_right;

        float m_yaw;
        float m_pitch;

        float m_fov;

        float m_nearPlane;
        float m_farPlane;

        float m_moveSpeed;
        float m_mouseSensitivity;
    };
}