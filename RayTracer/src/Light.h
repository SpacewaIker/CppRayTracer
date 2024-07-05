#pragma once

#include <glm/glm.hpp>

enum class LightType {
    Point,
    Directional,
};

struct Light {
    LightType Type;
    union {
        glm::vec3 Position;
        glm::vec3 Direction;
    };
    glm::vec3 Colour{1.0f, 1.0f, 1.0f};
    float Intensity = 1.0f;
};
