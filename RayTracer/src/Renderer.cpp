#include "Renderer.h"

#include "glm/geometric.hpp"

#include <cstdint>

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

            m_ImageData[y * m_FinalImage->GetWidth() + x] = PerPixel(coords);
        }
    }

    m_FinalImage->SetData(m_ImageData);
}

uint32_t Renderer::PerPixel(glm::vec2 coords) {
    glm::vec3 rayDir = glm::normalize(glm::vec3{coords.x, coords.y, -1.0f});
    glm::vec3 rayOrigin = {0.0f, 0.0f, 2.0f};

    float radius = (*m_Scene)["radius"].value_or(1.0f);
    glm::vec3 sphereCenter = {0.0f, 0.0f, 0.0f};

    glm::vec3 oc = rayOrigin - sphereCenter;
    float a = glm::dot(rayDir, rayDir);
    float b = 2.0f * glm::dot(rayDir, oc);
    float c = glm::dot(oc, oc) - radius * radius;

    float discriminant = b * b - 4.0f * a * c;

    if (discriminant < 0.0f) {
        return 0;
    }

    float discriminantSqrt = glm::sqrt(discriminant);
    float t1 = (-b - discriminantSqrt) / (2.0f * a);
    float t2 = (-b + discriminantSqrt) / (2.0f * a);
    float t = t1 < t2 ? t1 : t2;
    glm::vec3 hitPoint = rayOrigin + t * rayDir;
    glm::vec3 normal = glm::normalize(hitPoint - sphereCenter);

    glm::vec3 lightDir = glm::normalize(glm::vec3{1.0f, -1.0f, 1.0f});

    float lambert = glm::dot(normal, lightDir);
    glm::vec3 halfVector = glm::normalize(lightDir - rayDir);
    float specular = glm::pow(glm::dot(normal, halfVector), 32.0f);
    float intensity = glm::clamp(lambert + specular, 0.0f, 1.0f) * 255.0f;

    return 0xFF000000 | (uint32_t)intensity << 16 | (uint32_t)intensity << 8 | (uint32_t)intensity;
}
