#include "Plane.h"

float Plane::Intersect(const Ray &ray) const {
    float denom = glm::dot(ray.Direction, m_Normal);

    if (std::abs(denom) > 1e-6f) {
        glm::vec3 p0l0 = m_Position - ray.Origin;
        float t = glm::dot(p0l0, m_Normal) / denom;
        if (t >= 0.0f) {
            return t;
        }
    }
    return -1.0f;
}
