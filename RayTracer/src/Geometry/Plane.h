#pragma once

#include "Geometry.h"

class Plane : public Geometry {
  public:
    Plane(const glm::vec3 &position, const glm::vec3 &normal, int materialIndex = 0,
          int materialIndex2 = -1)
        : m_Position(position), m_Normal(glm::normalize(normal)), Geometry(materialIndex),
          m_MaterialIndex2(materialIndex2) {
        // set axes
        if (normal == glm::vec3(0.0f, 1.0f, 0.0f) || normal == glm::vec3(0.0f, -1.0f, 0.0f) ||
            normal == glm::vec3(0.0f, 0.0f, 1.0f)) {
            m_WidthAxis = glm::vec3(1.0f, 0.0f, 0.0f);
        } else if (normal == glm::vec3(0.0f, 0.0f, -1.0f)) {
            m_WidthAxis = glm::vec3(-1.0f, 0.0f, 0.0f);
        } else if (normal == glm::vec3(1.0f, 0.0f, 0.0f)) {
            m_WidthAxis = glm::vec3(0.0f, 0.0f, -1.0f);
        } else if (normal == glm::vec3(-1.0f, 0.0f, 0.0f)) {
            m_WidthAxis = glm::vec3(0.0f, 0.0f, 1.0f);
        } else {
            m_WidthAxis = glm::normalize(glm::cross(normal, glm::vec3(0.0f, 0.0f, 1.0f)));
        }
        m_HeightAxis = glm::cross(m_WidthAxis, normal);
    }

    float Intersect(const Ray &ray) const override;
    glm::vec3 GetNormal(const glm::vec3 &point) const override { return m_Normal; }
    int GetMaterialIndex(const glm::vec3 &point) const override;

  private:
    glm::vec3 m_Position{0.0f};
    glm::vec3 m_Normal{0.0f, 1.0f, 0.0f};
    int m_MaterialIndex2 = -1;

    glm::vec3 m_WidthAxis;
    glm::vec3 m_HeightAxis;
};
