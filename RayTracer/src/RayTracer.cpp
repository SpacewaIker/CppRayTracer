#include "Camera.h"
#include "Geometry/Plane.h"
#include "Geometry/Sphere.h"
#include "Renderer.h"
#include "Scene.h"
#include "Walnut/Application.h"
#include "Walnut/EntryPoint.h"
#include "Walnut/Image.h"
#include "Walnut/Timer.h"
#include "toml++/toml.hpp"

#include <glm/gtc/type_ptr.hpp>

class ExampleLayer : public Walnut::Layer {
  public:
    ExampleLayer() : m_Camera(45.0f, 0.1f, 100.0f) {
        m_Scene.Shapes.push_back(new Sphere({0.0f, 0.0f, 0.0f}, 0.5f, {1.0f, 0.0f, 0.0f}));
        m_Scene.Shapes.push_back(new Sphere({1.0f, 0.0f, 0.0f}, 0.5f, {0.0f, 1.0f, 0.0f}));
        m_Scene.Shapes.push_back(
            new Plane({0.0f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.5f, 0.5f, 0.5f}));
    }

    virtual void OnUpdate(float deltaTime) override { m_Camera.OnUpdate(deltaTime); }

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
        ImGui::Separator();
        ImGui::Text("Lighting");
        ImGui::ColorEdit3("Colour", glm::value_ptr(m_Renderer.m_LightColour));
        ImGui::SliderFloat3("Direction", glm::value_ptr(m_Renderer.m_LightDirection), -1.0f, 1.0f);
        ImGui::SliderFloat("Specular Intensity", &m_Renderer.m_LightSpecularIntensity, 0.0f, 1.0f);
        ImGui::SliderFloat("Specular Hardness", &m_Renderer.m_LightSpecularHardness, 0.0f, 100.0f);
        ImGui::Separator();
        ImGui::Text("Camera");
        ImGui::SliderFloat("Movement Speed", &m_Camera.m_MovementSpeed, 0.0f, 10.0f);
        ImGui::SliderFloat("Mouse Sensitivity", &m_Camera.m_MouseSensitivity, 0.0f, 0.02f);
        ImGui::Separator();
        if (ImGui::Button("Render")) {
            Render();
        }
        ImGui::End();

        // viewport window
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
        ImGui::Begin("Viewport");

        m_ViewportWidth = ImGui::GetContentRegionAvail().x;
        m_ViewportHeight = ImGui::GetContentRegionAvail().y;

        auto image = m_Renderer.GetFinalImage();
        if (image) {
            ImGui::Image(image->GetDescriptorSet(),
                         {(float)image->GetWidth(), (float)image->GetHeight()}, ImVec2(0, 1),
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

    // std::shared_ptr<toml::table> m_Scene;
};

Walnut::Application *Walnut::CreateApplication(int argc, char **argv) {
    Walnut::ApplicationSpecification spec;
    spec.Name = "Ray Tracer";

    Walnut::Application *app = new Walnut::Application(spec);
    app->PushLayer<ExampleLayer>();
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
