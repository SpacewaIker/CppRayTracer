#pragma once

#include "glm/glm.hpp"

#include <variant>
#include <vector>

struct Sphere {
    glm::vec3 Position{0.0f};
    float Radius = 0.5f;

    glm::vec3 Albedo{1.0f};
};

struct Plane {
    glm::vec3 Position{0.0f};
    glm::vec3 Normal{0.0f, 1.0f, 0.0f};

    glm::vec3 Albedo{1.0f};
};

typedef std::variant<Sphere, Plane> Shape;

struct Scene {
    std::vector<Shape> Shapes;
};
