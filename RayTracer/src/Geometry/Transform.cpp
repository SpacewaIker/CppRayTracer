#include "Transform.h"

#include "glm/gtc/matrix_transform.hpp"

Transform::Transform(glm::vec3 translation, glm::vec3 rotation, glm::vec3 scale,
                     std::unique_ptr<Geometry> child)
    : m_Child(std::move(child)), Geometry(-1) {
    m_Transform = glm::mat4(1.0f);
    m_Transform = glm::translate(m_Transform, translation);
    m_Transform = glm::rotate(m_Transform, glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
    m_Transform = glm::rotate(m_Transform, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
    m_Transform = glm::rotate(m_Transform, glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
    m_Transform = glm::scale(m_Transform, scale);
    m_TransformInverse = glm::inverse(m_Transform);
}

float Transform::Intersect(const Ray &ray) const {
    Ray transformedRay;
    transformedRay.Origin = glm::vec3(m_TransformInverse * glm::vec4(ray.Origin, 1.0f));
    transformedRay.Direction = glm::vec3(m_TransformInverse * glm::vec4(ray.Direction, 0.0f));

    return m_Child->Intersect(transformedRay);
}

glm::vec3 Transform::GetNormal(const glm::vec3 &point) const {
    glm::vec3 transformedPoint = glm::vec3(m_TransformInverse * glm::vec4(point, 1.0f));
    glm::vec3 localNormal = m_Child->GetNormal(transformedPoint);
    return glm::vec3(m_Transform * glm::vec4(localNormal, 0.0f));
}

int Transform::GetMaterialIndex(const glm::vec3 &point) const {
    glm::vec3 transformedPoint = glm::vec3(m_TransformInverse * glm::vec4(point, 1.0f));
    return m_Child->GetMaterialIndex(transformedPoint);
}
