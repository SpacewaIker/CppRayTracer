#pragma once

#include "Geometry.h"

class Plane : public Geometry {
  public:
    Plane(const glm::vec3 &position, const glm::vec3 &normal, const glm::vec3 &albedo,
          const glm::vec3 &albedo2 = glm::vec3(-1.0f))
        : m_Position(position), m_Normal(normal), Geometry(albedo), m_Albedo2(albedo2) {
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

    HitPayload Intersect(const Ray &ray) const override;
    glm::vec3 GetAlbedo(const glm::vec3 &point) const override;

  private:
    glm::vec3 m_Position{0.0f};
    glm::vec3 m_Normal{0.0f, 1.0f, 0.0f};
    glm::vec3 m_Albedo2;

    glm::vec3 m_WidthAxis;
    glm::vec3 m_HeightAxis;
};
