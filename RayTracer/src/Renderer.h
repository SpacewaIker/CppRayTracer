#pragma once

#include "Camera.h"
#include "Ray.h"
#include "Scene.h"
#include "Walnut/Image.h"

#include <glm/glm.hpp>
#include <memory>
#include <toml++/toml.hpp>

class Renderer {
  public:
    Renderer() = default;

    void Render(const Scene &scene, const Camera &camera);

    void OnResize(uint32_t width, uint32_t height);

    void SetScene(std::shared_ptr<toml::table> scene) { m_Scene = scene; }

    std::shared_ptr<Walnut::Image> GetFinalImage() const { return m_FinalImage; }

  private:
    struct HitPayload {
        Intersection Intersection;
        glm::vec3 WorldPosition;
        glm::vec3 WorldNormal;
    };

    glm::vec4 PerPixel(uint32_t x, uint32_t y); // ray gen shader
    HitPayload TraceRay(const Ray &ray);
    HitPayload ClosestHit(const Ray &ray, Intersection intersection);
    HitPayload Miss(const Ray &ray);
    bool TraceShadowRay(const Ray &ray);

  private:
    std::shared_ptr<Walnut::Image> m_FinalImage;
    uint32_t *m_ImageData = nullptr;
    std::shared_ptr<toml::table> m_Scene;

    const Scene *m_ActiveScene = nullptr;
    const Camera *m_ActiveCamera = nullptr;

  public:
    glm::vec3 m_SkyColour{0.5f, 0.7f, 0.9f};
    glm::vec3 m_LightColour{1.0f, 1.0f, 1.0f};
    glm::vec3 m_LightDirection{0.5f, -0.8f, -1.0f};
    float m_LightSpecularIntensity = 0.5;
    float m_LightSpecularHardness = 32.0f;

    int m_MaxBounces = 2;
    float m_RenderScale = 0.5f;
};
