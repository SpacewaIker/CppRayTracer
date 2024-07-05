
#pragma once

#include "glm/glm.hpp"

struct Material {
    glm::vec3 Albedo{1.0f};
    float Roughness{1.0f};
    float Metallic{0.0f};
};

const Material NO_MATERIAL{{-1.0f, -1.0f, -1.0f}};
