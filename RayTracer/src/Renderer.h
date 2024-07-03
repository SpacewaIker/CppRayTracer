#pragma once

#include "Walnut/Image.h"

#include <glm/glm.hpp>
#include <memory>
#include <toml++/toml.hpp>

class Renderer {
  public:
    Renderer() = default;

    void Render();

    void OnResize(uint32_t width, uint32_t height);

    void SetScene(std::shared_ptr<toml::table> scene) { m_Scene = scene; }

    std::shared_ptr<Walnut::Image> GetFinalImage() const { return m_FinalImage; }

  private:
    glm::vec4 PerPixel(glm::vec2 coords);

  private:
    std::shared_ptr<Walnut::Image> m_FinalImage;
    uint32_t *m_ImageData = nullptr;
    std::shared_ptr<toml::table> m_Scene;

  public:
    glm::vec3 m_LightColour;
    glm::vec3 m_LightDirection;
    float m_LightSpecularIntensity;
    float m_LightSpecularHardness;
};
