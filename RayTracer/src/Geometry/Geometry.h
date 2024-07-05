#pragma once

#include "../HitPayload.h"
#include "../Ray.h"
#include "glm/glm.hpp"

class Geometry {
  public:
    Geometry(const glm::vec3 &albedo) : m_Albedo(albedo) {}

    virtual HitPayload Intersect(const Ray &ray) const = 0;
    virtual glm::vec3 GetAlbedo(const glm::vec3 &point) const { return m_Albedo; }

  protected:
    glm::vec3 m_Albedo{1.0f};
};
