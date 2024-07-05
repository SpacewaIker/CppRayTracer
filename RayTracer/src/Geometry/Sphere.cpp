#include "Sphere.h"

HitPayload Sphere::Intersect(const Ray &ray) const {
    glm::vec3 oc = ray.Origin - this->m_Position;
    float a = glm::dot(ray.Direction, ray.Direction);
    float b = 2.0f * glm::dot(ray.Direction, oc);
    float c = glm::dot(oc, oc) - this->m_Radius * this->m_Radius;

    float discriminant = b * b - 4.0f * a * c;

    if (discriminant < 0.0f) {
        return NoHit;
    }

    HitPayload hit;

    float discriminantSqrt = glm::sqrt(discriminant);
    hit.T = (-b - discriminantSqrt) / (2.0f * a);
    hit.Normal = glm::normalize(ray.Origin + hit.T * ray.Direction - this->m_Position);

    return hit;
}
