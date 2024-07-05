#pragma once

#include "Geometry.h"

class AABB : public Geometry {
  public:
    AABB(const glm::vec3 &min, const glm::vec3 &max, const glm::vec3 &albedo)
        : m_Min(min), m_Max(max), Geometry(albedo) {}

    HitPayload Intersect(const Ray &ray) const override;

  private:
    glm::vec3 m_Min{0.0f};
    glm::vec3 m_Max{1.0f};
};
