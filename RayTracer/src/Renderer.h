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
    struct Settings {
        bool Accumulate = true;
        int MaxBounces = 5;
        float RenderScale = 0.5f;
        bool Jitter = true;
    };

  public:
    Renderer() = default;

    void Render(const Scene &scene, const Camera &camera);
    void OnResize(uint32_t width, uint32_t height);

    std::shared_ptr<Walnut::Image> GetFinalImage() const { return m_FinalImage; }

    void ResetFrameIndex() { m_FrameIndex = 1; }

    Settings &GetSettings() { return m_Settings; }
    uint32_t *GetImageData() { return m_ImageData; }

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
    glm::vec3 CalculateLighting(const HitPayload &hit, const Light &light);

  private:
    Settings m_Settings;

    std::shared_ptr<Walnut::Image> m_FinalImage;
    uint32_t *m_ImageData = nullptr;
    glm::vec4 *m_AccumulationData = nullptr;

    uint32_t m_FrameIndex = 1;

    std::vector<uint32_t> m_ImageVerticalterator;

    const Scene *m_ActiveScene = nullptr;
    const Camera *m_ActiveCamera = nullptr;
};
