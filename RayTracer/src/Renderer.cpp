#include "Renderer.h"

#include "glm/geometric.hpp"

#include <cstdint>

namespace Utils {
static uint32_t ConvertToRGBA(const glm::vec4 &color) {
    glm::vec4 clampedColor = glm::clamp(color, 0.0f, 1.0f);

    uint8_t r = (uint8_t)(clampedColor.r * 255.0f);
    uint8_t g = (uint8_t)(clampedColor.g * 255.0f);
    uint8_t b = (uint8_t)(clampedColor.b * 255.0f);
    uint8_t a = (uint8_t)(clampedColor.a * 255.0f);

    return (a << 24) | (b << 16) | (g << 8) | r;
}
} // namespace Utils

void Renderer::OnResize(uint32_t width, uint32_t height) {
    if (m_FinalImage) {
        if (m_FinalImage->GetWidth() == width && m_FinalImage->GetHeight() == height) {
            return;
        }

        m_FinalImage->Resize(width, height); // doesn't resize unless necessary
    } else {
        m_FinalImage = std::make_shared<Walnut::Image>(width, height, Walnut::ImageFormat::RGBA);
    }

    delete[] m_ImageData;
    m_ImageData = new uint32_t[width * height];
}

void Renderer::Render(const Scene &scene, const Camera &camera) {
    Ray ray;
    ray.Origin = camera.GetPosition();

    for (uint32_t y = 0; y < m_FinalImage->GetHeight(); y++) {
        for (uint32_t x = 0; x < m_FinalImage->GetWidth(); x++) {
            ray.Direction = camera.GetRayDirections()[y * m_FinalImage->GetWidth() + x];

            glm::vec4 color = TraceRay(scene, ray);
            m_ImageData[y * m_FinalImage->GetWidth() + x] = Utils::ConvertToRGBA(color);
        }
    }

    m_FinalImage->SetData(m_ImageData);
}

glm::vec4 Renderer::TraceRay(const Scene &scene, const Ray &ray) {
    float closestT = std::numeric_limits<float>::max();
    const Sphere *hitSphere = nullptr;
    const Plane *hitPlane = nullptr;

    for (const auto &shape : scene.Shapes) {
        if (std::holds_alternative<Sphere>(shape)) {
            const Sphere &sphere = std::get<Sphere>(shape);

            glm::vec3 oc = ray.Origin - sphere.Position;
            float a = glm::dot(ray.Direction, ray.Direction);
            float b = 2.0f * glm::dot(ray.Direction, oc);
            float c = glm::dot(oc, oc) - sphere.Radius * sphere.Radius;

            float discriminant = b * b - 4.0f * a * c;

            if (discriminant < 0.0f) {
                continue;
            }

            float discriminantSqrt = glm::sqrt(discriminant);
            float t = (-b - discriminantSqrt) / (2.0f * a);

            if (t > 0.0f && t < closestT) {
                closestT = t;
                hitSphere = &sphere;
            }
        } else if (std::holds_alternative<Plane>(shape)) {
            const Plane &plane = std::get<Plane>(shape);

            float denom = glm::dot(plane.Normal, ray.Direction);
            if (glm::abs(denom) > 1e-6) {
                float t = glm::dot(plane.Position - ray.Origin, plane.Normal) / denom;
                if (t > 0.0f && t < closestT) {
                    closestT = t;
                    hitPlane = &plane;
                }
            }
        }
    }

    // no hit
    if (!hitSphere && !hitPlane) {
        return glm::vec4(0);
    }

    glm::vec3 hitPoint = ray.Origin + closestT * ray.Direction;
    glm::vec3 normal;
    glm::vec3 colour;
    if (hitSphere) {
        normal = glm::normalize(hitPoint - hitSphere->Position);
        colour = hitSphere->Albedo;
    } else {
        normal = hitPlane->Normal;
        colour = hitPlane->Albedo;
    }

    glm::vec3 lightDir = -glm::normalize(m_LightDirection);

    float lambert = glm::dot(normal, lightDir);
    glm::vec3 halfVector = glm::normalize(lightDir - ray.Direction);
    float specular =
        m_LightSpecularIntensity * glm::pow(glm::dot(normal, halfVector), m_LightSpecularHardness);
    float intensity = glm::clamp(lambert + specular, 0.0f, 1.0f);

    return glm::vec4(colour * m_LightColour * intensity, 1.0f);
}
