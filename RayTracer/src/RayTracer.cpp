#include "Renderer.h"
#include "Walnut/Application.h"
#include "Walnut/EntryPoint.h"
#include "Walnut/Image.h"
#include "Walnut/Timer.h"
#include "toml++/toml.hpp"

class ExampleLayer : public Walnut::Layer {
  public:
    virtual void OnUIRender() override {
        // load scene
        try {
            toml::table table = toml::parse_file("scene.toml");
            m_Scene = std::make_shared<toml::table>(table);
        } catch (const toml::parse_error &err) {
            std::cerr << "Failed to parse scene.toml: " << err << std::endl;
            exit;
        }

        m_Renderer.SetScene(m_Scene);

        // settings window
        ImGui::Begin("Settings");
        ImGui::Text("Last Render Time: %.3f ms", m_LastRenderTime);
        ImGui::Text("%.1f FPS", 1000.0f / m_LastRenderTime);
        ImGui::Separator();
        ImGui::Text("Lighting");
        ImGui::SliderFloat3("Colour", &m_Renderer.m_LightColour.x, 0.0f, 1.0f);
        ImGui::SliderFloat3("Direction", &m_Renderer.m_LightDirection.x, -1.0f, 1.0f);
        ImGui::SliderFloat("Specular Intensity", &m_Renderer.m_LightSpecularIntensity, 0.0f, 1.0f);
        ImGui::SliderFloat("Specular Hardness", &m_Renderer.m_LightSpecularHardness, 0.0f, 100.0f);
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
        m_Renderer.Render();

        m_LastRenderTime = timer.ElapsedMillis();
    }

  private:
    Renderer m_Renderer;

    uint32_t m_ViewportWidth = 0;
    uint32_t m_ViewportHeight = 0;

    float m_LastRenderTime = 0.0f;

    std::shared_ptr<toml::table> m_Scene;
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
