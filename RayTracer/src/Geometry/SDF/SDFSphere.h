#pragma once

#include "SDFGeometry.h"

class SDFSphere : public SDFGeometry {
  public:
    SDFSphere(const glm::vec3 &position, float radius, int materialIndex)
        : m_Position(position), m_Radius(radius), SDFGeometry(materialIndex) {
        m_IsConvex = true;
    }

    float Distance(const glm::vec3 &point) const override {
        return glm::length(point - m_Position) - m_Radius;
    }

  private:
    glm::vec3 m_Position{0.0f};
    float m_Radius = 1.0f;
};
