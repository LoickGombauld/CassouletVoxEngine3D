#pragma once

#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>
#include <glm/fwd.hpp>

namespace Voxel
{
    struct Transform
    {
        glm::vec3 position{ 0.0f };
        glm::vec3 rotation{ 0.0f };
        glm::vec3 scale{ 1.0f };

        glm::mat4 getMatrix() const;
    };
}
