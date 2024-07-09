#pragma once

#include <cstdint>
#include <glm/glm.hpp>
#include <limits>

namespace RTRandom {

static uint32_t PCG_Hash(uint32_t input) {
    uint32_t state = input * 747796405u + 2891336453u;
    uint32_t word = ((state >> ((state >> 28u) + 4u)) ^ state) * 277803737u;
    return (word >> 22u) ^ word;
}

static float Float(uint32_t &seed) {
    seed = PCG_Hash(seed);
    return (float)seed / (float)std::numeric_limits<uint32_t>::max();
}

static float Float(uint32_t &seed, float min, float max) { return min + (max - min) * Float(seed); }

static glm::vec3 Vec3(uint32_t &seed) { return glm::vec3(Float(seed), Float(seed), Float(seed)); }

static glm::vec3 Vec3(uint32_t &seed, float min, float max) {
    return glm::vec3(Float(seed, min, max), Float(seed, min, max), Float(seed, min, max));
}

static glm::vec3 InUnitSphere(uint32_t &seed) {
    glm::vec3 p;
    do {
        p = Vec3(seed, -1.0f, 1.0f);
    } while (glm::dot(p, p) >= 1.0f);
    return glm::normalize(p);
}

} // namespace RTRandom
