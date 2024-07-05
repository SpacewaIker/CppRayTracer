#pragma once

#include "Geometry.h"

class Sphere : public Geometry {
  public:
    Sphere(const glm::vec3 &position, float radius, const glm::vec3 &albedo)
        : m_Position(position), m_Radius(radius), Geometry(albedo) {}

    HitPayload Intersect(const Ray &ray) const override;

  private:
    glm::vec3 m_Position{0.0f};
    float m_Radius = 0.5f;
};
