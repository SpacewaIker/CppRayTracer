#pragma once

#include "SDFGeometry.h"

class SDFAABB : public SDFGeometry {
  public:
    SDFAABB(const glm::vec3 &min, const glm::vec3 &max, float rounded, int materialIndex)
        : m_Rounded(rounded), SDFGeometry(materialIndex) {
        m_IsConvex = true;

        m_Center = (min + max) / 2.0f;
        m_R = (max - min) / 2.0f;
    }

  private:
    float Distance(const glm::vec3 &point) const override {
        glm::vec3 q = glm::abs(point - m_Center) - m_R + m_Rounded;
        return glm::length(glm::max(q, glm::vec3(0.0f))) +
               glm::min(glm::max(q.x, glm::max(q.y, q.z)), 0.0f) - m_Rounded;
    }

  private:
    glm::vec3 m_Center{0.0f};
    glm::vec3 m_R{0.0f};
    float m_Rounded = 0.0f;
};
