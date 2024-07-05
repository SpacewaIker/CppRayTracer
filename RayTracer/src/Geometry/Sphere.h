#pragma once

#include "Geometry.h"

class Sphere : public Geometry {
  public:
    Sphere(const glm::vec3 &position, float radius, int materialIndex = 0)
        : m_Position(position), m_Radius(radius), Geometry(materialIndex) {}

    float Intersect(const Ray &ray) const override;
    glm::vec3 GetNormal(const glm::vec3 &point) const override;

  private:
    glm::vec3 m_Position{0.0f};
    float m_Radius = 0.5f;
};
