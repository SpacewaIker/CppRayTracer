#include "Plane.h"

HitPayload Plane::Intersect(const Ray &ray) const {
    float denom = glm::dot(ray.Direction, m_Normal);

    if (std::abs(denom) > 1e-6f) {
        glm::vec3 p0l0 = m_Position - ray.Origin;
        float t = glm::dot(p0l0, m_Normal) / denom;
        if (t >= 0.0f) {
            HitPayload hit;
            hit.T = t;
            hit.Normal = m_Normal;
            return hit;
        }
    }
    return NoHit;
}

glm::vec3 Plane::GetAlbedo(const glm::vec3 &point) const {
    if (m_Albedo2 == glm::vec3(-1.0f)) {
        return m_Albedo;
    }

    glm::vec3 p = point - glm::dot(point - m_Position, m_Normal) * m_Normal;
    float x = glm::dot(point - m_Position, m_WidthAxis);
    float z = glm::dot(point - m_Position, m_HeightAxis);

    float dx = glm::floor(m_Position.x - x);
    float dz = glm::floor(m_Position.z - z);
    int i = (int)(dx + dz);

    if (i % 2) {
        return m_Albedo;
    } else {
        return m_Albedo2;
    }
}
