#include "Sphere.h"

float Sphere::Intersect(const Ray &ray) const {
    glm::vec3 oc = ray.Origin - this->m_Position;
    float a = glm::dot(ray.Direction, ray.Direction);
    float b = 2.0f * glm::dot(ray.Direction, oc);
    float c = glm::dot(oc, oc) - this->m_Radius * this->m_Radius;

    float discriminant = b * b - 4.0f * a * c;

    if (discriminant < 0.0f) {
        return -1.0f;
    }

    float discriminantSqrt = glm::sqrt(discriminant);
    float t = (-b - discriminantSqrt) / (2.0f * a);
    return t;
}

glm::vec3 Sphere::GetNormal(const glm::vec3 &point) const {
    return glm::normalize(point - this->m_Position);
}
