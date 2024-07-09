#include "Renderer.h"

#include "RTRandom.h"
#include "glm/geometric.hpp"

#include <cstdint>
#include <cstring>
#include <execution>

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

    delete[] m_AccumulationData;
    m_AccumulationData = new glm::vec4[width * height];

    ResetFrameIndex();

    m_ImageVerticalterator.resize(height);
    for (uint32_t i = 0; i < height; i++) {
        m_ImageVerticalterator[i] = i;
    }
}

void Renderer::Render(const Scene &scene, const Camera &camera) {
    m_ActiveScene = &scene;
    m_ActiveCamera = &camera;

    if (m_FrameIndex == 1) {
        memset(m_AccumulationData, 0,
               m_FinalImage->GetWidth() * m_FinalImage->GetHeight() * sizeof(glm::vec4));
    }

    // clang-format off
    std::for_each(std::execution::par, m_ImageVerticalterator.begin(), m_ImageVerticalterator.end(), [this](uint32_t y) {
        for (uint32_t x = 0; x < m_FinalImage->GetWidth(); x++) {
            glm::vec4 colour = PerPixel(x, y);

            m_AccumulationData[y * m_FinalImage->GetWidth() + x] += colour;

            glm::vec4 accumulatedColour =
              m_AccumulationData[y * m_FinalImage->GetWidth() + x] /
              (float)m_FrameIndex;

            m_ImageData[y * m_FinalImage->GetWidth() + x] =
              Utils::ConvertToRGBA(accumulatedColour);
        }
    });
    // clang-format on

    m_FinalImage->SetData(m_ImageData);

    if (m_Settings.Accumulate) {
        m_FrameIndex++;
    } else {
        m_FrameIndex = 1;
    }
}

glm::vec4 Renderer::PerPixel(uint32_t x, uint32_t y) {
    uint32_t seed = x + y * m_FinalImage->GetWidth();
    seed *= m_FrameIndex;
    auto now = std::chrono::system_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
    seed ^= ms;

    Ray ray;
    ray.Origin = m_ActiveCamera->GetPosition();
    if (m_Settings.Jitter) {
        ray.Origin += RTRandom::Vec3(seed, -0.003f, 0.003f);
    }
    ray.Direction = m_ActiveCamera->GetRayDirections()[y * m_FinalImage->GetWidth() + x];

    glm::vec3 light = glm::vec3(0.0f);
    glm::vec3 contribution{1.0f};

    for (int _i = 0; _i < m_Settings.MaxBounces; _i++) {
        seed++;

        Renderer::HitPayload hit = TraceRay(ray);

        // no hit
        if (hit.Intersection.GeometryIndex == -1) {
            light += m_ActiveScene->SkyColour * contribution;
            break;
        }

        for (const auto &lightSource : m_ActiveScene->Lights) {
            glm::vec3 lightColour = CalculateLighting(hit, lightSource);
            light += lightColour * contribution;
        }

        int materialIndex =
            m_ActiveScene->Geometry[hit.Intersection.GeometryIndex]->GetMaterialIndex(
                hit.WorldPosition);
        Material material = m_ActiveScene->Materials[materialIndex];

        light += material.GetEmission() * material.Albedo;
        contribution *= material.Albedo;

        ray.Origin = hit.WorldPosition + hit.WorldNormal * 0.0001f;
        ray.Direction = glm::normalize(RTRandom::InUnitSphere(seed) + hit.WorldNormal);
    }

    return glm::vec4(light, 1.0f);
}

Renderer::HitPayload Renderer::TraceRay(const Ray &ray) {
    Intersection closestHit;
    closestHit.T = std::numeric_limits<float>::max();

    for (uint32_t i = 0; i < m_ActiveScene->Geometry.size(); i++) {
        const Geometry *geometry = m_ActiveScene->Geometry[i];
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
    const Geometry &closestGeometry = *m_ActiveScene->Geometry[intersection.GeometryIndex];

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
    for (const auto geometry : m_ActiveScene->Geometry) {
        float t = geometry->Intersect(ray);

        if (t > 0.0f) {
            return true;
        }
    }

    return false;
}

glm::vec3 Renderer::CalculateLighting(const HitPayload &hit, const Light &light) {
    // shadow
    Ray shadowRay;
    shadowRay.Origin = hit.WorldPosition + hit.WorldNormal * 0.0001f;

    if (light.Type == LightType::Point) {
        shadowRay.Direction = glm::normalize(light.Position - hit.WorldPosition);
    } else if (light.Type == LightType::Directional) {
        shadowRay.Direction = -light.Direction;
    }

    if (TraceShadowRay(shadowRay)) {
        return glm::vec3(0.0f);
    }

    glm::vec3 lightDir = glm::normalize(shadowRay.Direction);
    float lambert = glm::max(0.0f, glm::dot(hit.WorldNormal, lightDir));

    int materialIndex = m_ActiveScene->Geometry[hit.Intersection.GeometryIndex]->GetMaterialIndex(
        hit.WorldPosition);
    Material material = m_ActiveScene->Materials[materialIndex];

    glm::vec3 colour = material.Albedo * light.Colour * light.Intensity * lambert;

    return colour;
}
