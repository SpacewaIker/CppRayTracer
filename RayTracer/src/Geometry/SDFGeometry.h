#pragma once

#include "Geometry.h"

class SDFGeometry : public Geometry {
  public:
    SDFGeometry(int materialIndex) : Geometry(materialIndex) {}

    float Intersect(const Ray &ray) const override;
    glm::vec3 GetNormal(const glm::vec3 &point) const override;

  private:
    float Distance(const glm::vec3 &point) const;

  private:
    bool m_IsConvex = true;
};
