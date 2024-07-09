#pragma once

#include "SDFGeometry.h"

class SDFHollowSphere : public SDFGeometry {
  public:
    SDFHollowSphere(const glm::vec3 &position, float radius, float thickness, float height,
                    int materialIndex)
        : m_Position(position), m_Radius(radius), m_Thickness(thickness), m_Height(height),
          SDFGeometry(materialIndex) {
        m_w = sqrt(m_Radius * m_Radius - m_Height * m_Height);
    }

    float Distance(const glm::vec3 &point) const override {
        glm::vec3 movedPoint = point - m_Position;
        glm::vec2 q = {glm::length(glm::vec2(movedPoint.x, movedPoint.z)), movedPoint.y};

        return (m_Height * q.x < m_w * q.y) ? (glm::length(q - glm::vec2(m_w, m_Height)))
                                            : (glm::abs(glm::length(q) - m_Radius) - m_Thickness);
    }

  private:
    glm::vec3 m_Position{0.0f};
    float m_Radius = 0.0f;
    float m_Thickness = 0.0f;
    float m_Height = 0.0f;
    float m_w = 0.0f;
};
