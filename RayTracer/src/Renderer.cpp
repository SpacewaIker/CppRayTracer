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
    m_ActiveScene = &scene;
    m_ActiveCamera = &camera;

    for (uint32_t y = 0; y < m_FinalImage->GetHeight(); y++) {
        for (uint32_t x = 0; x < m_FinalImage->GetWidth(); x++) {
            glm::vec4 colour = PerPixel(x, y);
            m_ImageData[y * m_FinalImage->GetWidth() + x] = Utils::ConvertToRGBA(colour);
        }
    }

    m_FinalImage->SetData(m_ImageData);
}

glm::vec4 Renderer::PerPixel(uint32_t x, uint32_t y) {
    Ray ray;
    ray.Origin = m_ActiveCamera->GetPosition();
    ray.Direction = m_ActiveCamera->GetRayDirections()[y * m_FinalImage->GetWidth() + x];

    Renderer::HitPayload hit = TraceRay(ray);

    // no hit
    if (hit.Intersection.GeometryIndex == -1) {
        return glm::vec4(0);
    }

    glm::vec3 colour = m_AmbientColour;

    // shadow
    Ray shadowRay;
    shadowRay.Direction = -m_LightDirection;
    shadowRay.Origin = hit.WorldPosition + hit.WorldNormal * 0.001f;

    if (!TraceShadowRay(shadowRay)) { // not in shadow
        glm::vec3 lightDir = -glm::normalize(m_LightDirection);
        float lambert = glm::dot(hit.WorldNormal, lightDir);
        glm::vec3 halfVector = glm::normalize(lightDir - ray.Direction);
        float specular = m_LightSpecularIntensity *
                         glm::pow(glm::dot(hit.WorldNormal, halfVector), m_LightSpecularHardness);
        float intensity = glm::clamp(lambert + specular, 0.0f, 1.0f);

        colour += m_LightColour * intensity;
    }

    glm::vec3 albedo =
        m_ActiveScene->Shapes[hit.Intersection.GeometryIndex]->GetAlbedo(hit.WorldPosition);
    return glm::vec4(colour * albedo, 1.0f);
}

Renderer::HitPayload Renderer::TraceRay(const Ray &ray) {
    Intersection closestHit;
    closestHit.T = std::numeric_limits<float>::max();

    for (uint32_t i = 0; i < m_ActiveScene->Shapes.size(); i++) {
        const Geometry *geometry = m_ActiveScene->Shapes[i];
        float t = geometry->Intersect(ray);

        if (t > 0.0f && t < closestHit.T) {
            closestHit.T = t;
            closestHit.GeometryIndex = i;
        }
    }

    // no hit
    if (closestHit.GeometryIndex == -1) {
        return Miss(ray);
    }

    return ClosestHit(ray, closestHit);
}

Renderer::HitPayload Renderer::ClosestHit(const Ray &ray, Intersection intersection) {
    const Geometry &closestGeometry = *m_ActiveScene->Shapes[intersection.GeometryIndex];

    Renderer::HitPayload payload;
    payload.Intersection = intersection;
    payload.WorldPosition = ray.Origin + intersection.T * ray.Direction;
    payload.WorldNormal = closestGeometry.GetNormal(payload.WorldPosition);

    return payload;
}

Renderer::HitPayload Renderer::Miss(const Ray &ray) {
    return HitPayload{Intersection{-1.0f, -1}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}};
}

bool Renderer::TraceShadowRay(const Ray &ray) {
    for (const auto geometry : m_ActiveScene->Shapes) {
        float t = geometry->Intersect(ray);

        if (t > 0.0f) {
            return true;
        }
    }

    return false;
}
