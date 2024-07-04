#pragma once

#include "../Ray.h"
#include "glm/glm.hpp"

class Geometry {
  public:
    Geometry(const glm::vec3 &albedo) : m_Albedo(albedo) {}

    virtual float Intersect(const Ray &ray) const = 0;
    virtual glm::vec3 GetNormal(const glm::vec3 &point) const = 0;
    virtual glm::vec3 GetAlbedo() const { return m_Albedo; }

  protected:
    glm::vec3 m_Albedo{1.0f};
};
