#pragma once

#include "../Ray.h"
#include "glm/glm.hpp"

struct Intersection {
    float T = 0.0f;
    int GeometryIndex = -1;
};

class Geometry {
  public:
    Geometry(const glm::vec3 &albedo) : m_Albedo(albedo) {}

    virtual float Intersect(const Ray &ray) const = 0;
    virtual glm::vec3 GetNormal(const glm::vec3 &point) const = 0;
    virtual glm::vec3 GetAlbedo(const glm::vec3 &point) const { return m_Albedo; }

  protected:
    glm::vec3 m_Albedo{1.0f};
};
