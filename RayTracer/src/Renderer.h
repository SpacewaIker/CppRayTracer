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
    glm::vec4 TraceRay(const Scene &scene, const Ray &ray);

  private:
    std::shared_ptr<Walnut::Image> m_FinalImage;
    uint32_t *m_ImageData = nullptr;
    std::shared_ptr<toml::table> m_Scene;

  public:
    glm::vec3 m_LightColour{1.0f, 1.0f, 1.0f};
    glm::vec3 m_LightDirection{1.0f, -1.0f, -1.0f};
    float m_LightSpecularIntensity = 0.5;
    float m_LightSpecularHardness = 32.0f;

    float m_RenderScale = 0.5f;
};
