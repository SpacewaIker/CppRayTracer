#pragma once

#include "../Geometry.h"

class SDFGeometry : public Geometry {
  public:
    SDFGeometry(int materialIndex) : Geometry(materialIndex) {}

    float Intersect(const Ray &ray) const override;
    glm::vec3 GetNormal(const glm::vec3 &point) const override;

  private:
    virtual float Distance(const glm::vec3 &point) const = 0;

  protected:
    bool m_IsConvex = false;
};
