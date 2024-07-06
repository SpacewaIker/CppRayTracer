#include "Camera.h"
#include "Geometry/AABB.h"
#include "Geometry/Plane.h"
#include "Geometry/Sphere.h"
#include "Renderer.h"
#include "Scene.h"
#include "Walnut/Application.h"
#include "Walnut/EntryPoint.h"
#include "Walnut/Image.h"
#include "Walnut/Timer.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#include "toml++/toml.hpp"

#include <glm/gtc/type_ptr.hpp>

class MainLayer : public Walnut::Layer {
  public:
    MainLayer() : m_Camera(45.0f, 0.1f, 100.0f) {
        m_Scene.Materials.push_back(Material{{1.0f, 0.3f, 0.3f}, 0.8f, 0.0f}); // red
        m_Scene.Materials.push_back(Material{{0.3f, 1.0f, 0.3f}, 0.1f, 0.0f}); // green
        m_Scene.Materials.push_back(Material{{0.5f, 0.5f, 0.5f}, 0.5f, 0.0f}); // dark grey
        m_Scene.Materials.push_back(Material{{0.7f, 0.7f, 0.7f}, 0.3f, 0.0f}); // light grey

        m_Scene.Geometry.push_back(new Sphere({1.0f, 0.0f, -1.0f}, 0.5f, 0));
        m_Scene.Geometry.push_back(new AABB({-0.5f, -0.5f, -0.5f}, {0.5f, 0.5f, 0.5f}, 1));
        m_Scene.Geometry.push_back(new Plane({0.0f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}, 2, 3));

        {
            Light light;
            light.Type = LightType::Directional;
            light.Direction = glm::normalize(glm::vec3{-0.7f, -1.0f, 0.4f});
            light.Intensity = 0.7;
            m_Scene.Lights.push_back(light);
        }
        {
            Light light;
            light.Type = LightType::Point;
            light.Position = glm::normalize(glm::vec3{1.0f, 1.0f, 1.0f});
            light.Intensity = 0.7;
            m_Scene.Lights.push_back(light);
        }
    }

    virtual void OnUpdate(float deltaTime) override {
        if (m_Camera.OnUpdate(deltaTime)) {
            m_Renderer.ResetFrameIndex();
        }
    }

    virtual void OnUIRender() override {
        // load scene
        // try {
        //     toml::table table = toml::parse_file("scene.toml");
        //     m_Scene = std::make_shared<toml::table>(table);
        // } catch (const toml::parse_error &err) {
        //     std::cerr << "Failed to parse scene.toml: " << err << std::endl;
        //     exit;
        // }

        // m_Renderer.SetScene(m_Scene);

        // settings window
        ImGui::Begin("Settings");
        ImGui::Text("Last Render Time: %.3f ms", m_LastRenderTime);
        ImGui::Text("%.1f FPS", 1000.0f / m_LastRenderTime);
        ImGui::Text("Render Resolution: %dx%d", m_ViewportWidth, m_ViewportHeight);
        ImGui::SliderFloat("Render Scale", &m_Renderer.GetSettings().RenderScale, 0.1f, 1.0f);
        ImGui::SliderInt("Max Bounces", &m_Renderer.GetSettings().MaxBounces, 1, 10);
        ImGui::Checkbox("Accumulate", &m_Renderer.GetSettings().Accumulate);
        if (ImGui::Button("Reset")) {
            m_Renderer.ResetFrameIndex();
        }

        ImGui::Separator();

        ImGui::Text("Lighting");
        ImGui::ColorEdit3("Sky Colour", glm::value_ptr(m_Scene.SkyColour));

        ImGui::Separator();

        ImGui::Text("Camera");
        ImGui::SliderFloat("Movement Speed", &m_Camera.m_MovementSpeed, 0.0f, 10.0f);
        ImGui::SliderFloat("Mouse Sensitivity", &m_Camera.m_MouseSensitivity, 0.0f, 0.02f);

        ImGui::Separator();

        ImGui::Text("Save Image");
        ImGui::InputText("Filename", m_SaveFilename, 256);
        if (ImGui::Button("Save Image")) {
            auto image = m_Renderer.GetImageData();
            if (image) {
                uint32_t *flippedImage = new uint32_t[m_ViewportWidth * m_ViewportHeight];
                for (uint32_t y = 0; y < m_ViewportHeight; y++) {
                    memcpy(&flippedImage[y * m_ViewportWidth],
                           &image[(m_ViewportHeight - y - 1) * m_ViewportWidth],
                           m_ViewportWidth * 4);
                }
                stbi_write_png(m_SaveFilename, m_ViewportWidth, m_ViewportHeight, 4, flippedImage,
                               m_ViewportWidth * 4);
            }
        }

        ImGui::End();

        // viewport window
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
        ImGui::Begin("Viewport");

        m_ViewportWidth =
            (uint32_t)(m_Renderer.GetSettings().RenderScale * ImGui::GetContentRegionAvail().x);
        m_ViewportHeight =
            (uint32_t)(m_Renderer.GetSettings().RenderScale * ImGui::GetContentRegionAvail().y);

        auto image = m_Renderer.GetFinalImage();
        if (image) {
            ImGui::Image(image->GetDescriptorSet(), ImGui::GetContentRegionAvail(), ImVec2(0, 1),
                         ImVec2(1, 0));
        }

        ImGui::End();
        ImGui::PopStyleVar();

        Render();
    }

    void Render() {
        Walnut::Timer timer;

        m_Renderer.OnResize(m_ViewportWidth, m_ViewportHeight);
        m_Camera.OnResize(m_ViewportWidth, m_ViewportHeight);
        m_Renderer.Render(m_Scene, m_Camera);

        m_LastRenderTime = timer.ElapsedMillis();
    }

  private:
    Renderer m_Renderer;
    Camera m_Camera;
    Scene m_Scene;

    uint32_t m_ViewportWidth = 0;
    uint32_t m_ViewportHeight = 0;

    float m_LastRenderTime = 0.0f;

    char m_SaveFilename[256] = "output.png";

    // std::shared_ptr<toml::table> m_Scene;
};

Walnut::Application *Walnut::CreateApplication(int argc, char **argv) {
    Walnut::ApplicationSpecification spec;
    spec.Name = "Ray Tracer";

    Walnut::Application *app = new Walnut::Application(spec);
    app->PushLayer<MainLayer>();
    app->SetMenubarCallback([app]() {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Exit")) {
                app->Close();
            }
            ImGui::EndMenu();
        }
    });
    return app;
}
