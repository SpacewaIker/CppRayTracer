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

void Renderer::Render() {
    for (uint32_t y = 0; y < m_FinalImage->GetHeight(); y++) {
        for (uint32_t x = 0; x < m_FinalImage->GetWidth(); x++) {
            glm::vec2 coords = {(float)x / (float)m_FinalImage->GetWidth(),
                                (float)y / (float)m_FinalImage->GetHeight()};

            // map to [-1, 1]
            coords = coords * 2.0f - 1.0f;

            glm::vec4 color = PerPixel(coords);
            m_ImageData[y * m_FinalImage->GetWidth() + x] = Utils::ConvertToRGBA(color);
        }
    }

    m_FinalImage->SetData(m_ImageData);
}

glm::vec4 Renderer::PerPixel(glm::vec2 coords) {
    glm::vec3 rayDir = glm::vec3(coords.x, coords.y, -1.0f);
    glm::vec3 rayOrigin = {0.0f, 0.0f, 2.0f};

    float radius = 1.0f;
    glm::vec3 sphereCenter = {0.0f, 0.0f, 0.0f};

    glm::vec3 oc = rayOrigin - sphereCenter;
    float a = glm::dot(rayDir, rayDir);
    float b = 2.0f * glm::dot(rayDir, oc);
    float c = glm::dot(oc, oc) - radius * radius;

    float discriminant = b * b - 4.0f * a * c;

    if (discriminant < 0.0f) {
        return glm::vec4(0);
    }

    float discriminantSqrt = glm::sqrt(discriminant);
    float t = (-b - discriminantSqrt) / (2.0f * a);
    glm::vec3 hitPoint = rayOrigin + t * rayDir;
    glm::vec3 normal = glm::normalize(hitPoint - sphereCenter);

    glm::vec3 lightDir = -glm::normalize(m_LightDirection);

    float lambert = glm::dot(normal, lightDir);
    glm::vec3 halfVector = glm::normalize(lightDir - rayDir);
    float specular =
        m_LightSpecularIntensity * glm::pow(glm::dot(normal, halfVector), m_LightSpecularHardness);
    float intensity = glm::clamp(lambert + specular, 0.0f, 1.0f);

    return glm::vec4(m_LightColour * intensity, 1.0f);
}
