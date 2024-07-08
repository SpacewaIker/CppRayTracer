#pragma once

#include "SDFGeometry.h"

class SDFPlane : public SDFGeometry {
  public:
    SDFPlane(const glm::vec3 &position, const glm::vec3 &normal, int materialIndex,
             int materialIndex2 = -1)
        : m_Position(position), m_Normal(glm::normalize(normal)), SDFGeometry(materialIndex),
          m_MaterialIndex2(materialIndex2) {
        m_IsConvex = true;

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

    int GetMaterialIndex(const glm::vec3 &point) const override {
        if (m_MaterialIndex2 == -1) {
            return m_MaterialIndex;
        }

        glm::vec3 p = point - glm::dot(point - m_Position, m_Normal) * m_Normal;
        float x = glm::dot(point - m_Position, m_WidthAxis);
        float z = glm::dot(point - m_Position, m_HeightAxis);

        float dx = glm::floor(m_Position.x - x);
        float dz = glm::floor(m_Position.z - z);
        int i = (int)(dx + dz);

        if (i % 2) {
            return m_MaterialIndex;
        } else {
            return m_MaterialIndex2;
        }
    }

  private:
    float Distance(const glm::vec3 &point) const override {
        return glm::dot(point - m_Position, m_Normal);
    }

  private:
    glm::vec3 m_Normal{0.0f};
    glm::vec3 m_Position{0.0f};
    int m_MaterialIndex2 = -1;

    glm::vec3 m_WidthAxis;
    glm::vec3 m_HeightAxis;
};
