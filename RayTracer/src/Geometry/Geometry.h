#pragma once

#include "../Ray.h"

struct Intersection {
    float T = 0.0f;
    int GeometryIndex = -1;
};

class Geometry {
  public:
    Geometry(int materialIndex) : m_MaterialIndex(materialIndex) {}
    virtual ~Geometry() = default;

    virtual float Intersect(const Ray &ray) const = 0;
    virtual glm::vec3 GetNormal(const glm::vec3 &point) const = 0;
    virtual int GetMaterialIndex(const glm::vec3 &point) const { return m_MaterialIndex; }

  protected:
    int m_MaterialIndex = 0;
};
