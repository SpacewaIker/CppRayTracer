#pragma once

#include "Geometry.h"

#include <memory>

class Transform : public Geometry {
  public:
    Transform(glm::vec3 translation, glm::vec3 rotation, glm::vec3 scale,
              std::unique_ptr<Geometry> child);

    float Intersect(const Ray &ray) const override;
    glm::vec3 GetNormal(const glm::vec3 &point) const override;
    int GetMaterialIndex(const glm::vec3 &point) const override;

  private:
    glm::mat4 m_Transform;
    glm::mat4 m_TransformInverse;
    std::unique_ptr<Geometry> m_Child;
};
