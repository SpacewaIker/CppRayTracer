
#include "SDFGeometry.h"

#define SDF_MAX_ITERATIONS 500
#define SDF_EPSILON 0.001f
#define SDF_MAX_DEPTH 1000.0f

float SDFGeometry::Intersect(const Ray &ray) const {
    float depth = 0.0f;
    float lastDist = std::numeric_limits<float>::max();
    glm::vec3 point = ray.Origin;

    for (int _i = 0; _i < SDF_MAX_ITERATIONS; _i++) {
        float dist = Distance(ray.Origin + ray.Direction * depth);

        if (dist < SDF_EPSILON) {
            return depth;
        }

        depth += dist;

        if (depth >= SDF_MAX_DEPTH || (m_IsConvex && dist >= lastDist)) {
            return -1.0f;
        }

        lastDist = dist;
    }

    return -1.0f;
}

glm::vec3 SDFGeometry::GetNormal(const glm::vec3 &point) const {
    float epsilon = 0.0001f;
    glm::vec3 normal =
        glm::vec3(Distance(point + glm::vec3(epsilon, 0.0f, 0.0f)) - Distance(point - glm::vec3(epsilon, 0.0f, 0.0f)),
                  Distance(point + glm::vec3(0.0f, epsilon, 0.0f)) - Distance(point - glm::vec3(0.0f, epsilon, 0.0f)),
                  Distance(point + glm::vec3(0.0f, 0.0f, epsilon)) - Distance(point - glm::vec3(0.0f, 0.0f, epsilon)));

    return glm::normalize(normal);
}
