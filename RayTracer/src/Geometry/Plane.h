#pragma once

#include "Geometry.h"

class Plane : public Geometry {
  public:
    Plane(const glm::vec3 &position, const glm::vec3 &normal, const glm::vec3 &albedo)
        : m_Position(position), m_Normal(normal), Geometry(albedo) {}

    float Intersect(const Ray &ray) const override;
    glm::vec3 GetNormal(const glm::vec3 &point) const override { return m_Normal; }

  private:
    glm::vec3 m_Position{0.0f};
    glm::vec3 m_Normal{0.0f, 1.0f, 0.0f};
};
