#include "AABB.h"

HitPayload AABB::Intersect(const Ray &ray) const {
    // t_x_min is the t value associated with m_Min.x, etc.
    float t_x_min, t_x_max, t_y_min, t_y_max, t_z_min, t_z_max;

    // ray direction aligned with AABB
    if (ray.Direction.x == 0.0f) {
        if (ray.Origin.x < m_Min.x || ray.Origin.x > m_Max.x) {
            // ray lies outside of AABB slab
            return NoHit;
        }
        // no intersection
        t_x_min = -INFINITY;
        t_x_max = INFINITY;
    } else {
        t_x_min = (m_Min.x - ray.Origin.x) / ray.Direction.x;
        t_x_max = (m_Max.x - ray.Origin.x) / ray.Direction.x;
    }

    // ray direction aligned with AABB
    if (ray.Direction.y == 0.0f) {
        if (ray.Origin.y < m_Min.y || ray.Origin.y > m_Max.y) {
            // ray lies outside of AABB slab
            return NoHit;
        }
        // no intersection
        t_y_min = -INFINITY;
        t_y_max = INFINITY;
    } else {
        t_y_min = (m_Min.y - ray.Origin.y) / ray.Direction.y;
        t_y_max = (m_Max.y - ray.Origin.y) / ray.Direction.y;
    }

    // ray direction aligned with AABB
    if (ray.Direction.z == 0.0f) {
        if (ray.Origin.z < m_Min.z || ray.Origin.z > m_Max.z) {
            // ray lies outside of AABB slab
            return NoHit;
        }
        // no intersection
        t_z_min = -INFINITY;
        t_z_max = INFINITY;
    } else {
        t_z_min = (m_Min.z - ray.Origin.z) / ray.Direction.z;
        t_z_max = (m_Max.z - ray.Origin.z) / ray.Direction.z;
    }

    // lowest t values
    float t_x_low = glm::min(t_x_min, t_x_max);
    float t_y_low = glm::min(t_y_min, t_y_max);
    float t_z_low = glm::min(t_z_min, t_z_max);
    float t_x_high = glm::max(t_x_min, t_x_max);
    float t_y_high = glm::max(t_y_min, t_y_max);
    float t_z_high = glm::max(t_z_min, t_z_max);

    float t_low = glm::max(glm::max(t_x_low, t_y_low), t_z_low);
    float t_high = glm::min(glm::min(t_x_high, t_y_high), t_z_high);

    if (t_low > t_high || t_high < 0.0f) {
        return NoHit;
    }

    HitPayload hit;
    hit.T = t_low;

    // find normal
    if (t_low == t_x_low) {
        if (ray.Direction.x < 0.0f) {
            hit.Normal = glm::vec3(1.0f, 0.0f, 0.0f);
        } else {
            hit.Normal = glm::vec3(-1.0f, 0.0f, 0.0f);
        }
    } else if (t_low == t_y_low) {
        if (ray.Direction.y < 0.0f) {
            hit.Normal = glm::vec3(0.0f, 1.0f, 0.0f);
        } else {
            hit.Normal = glm::vec3(0.0f, -1.0f, 0.0f);
        }
    } else {
        if (ray.Direction.z < 0.0f) {
            hit.Normal = glm::vec3(0.0f, 0.0f, 1.0f);
        } else {
            hit.Normal = glm::vec3(0.0f, 0.0f, -1.0f);
        }
    }

    return hit;
}
