#pragma once

#include <glm/glm.hpp>

struct HitPayload {
    float T = 0.0f;
    glm::vec3 Normal = glm::vec3(0.0f);
};

const HitPayload NoHit = HitPayload{-1.0f, {0.0f, 0.0f, 0.0f}};
