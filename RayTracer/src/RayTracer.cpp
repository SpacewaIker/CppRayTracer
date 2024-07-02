#include "Renderer.h"
#include "Walnut/Application.h"
#include "Walnut/EntryPoint.h"
#include "Walnut/Image.h"
#include "Walnut/Timer.h"

class ExampleLayer : public Walnut::Layer {
  public:
    virtual void OnUIRender() override {
        // settings window
        ImGui::Begin("Settings");
        ImGui::Text("Last Render Time: %.3f ms", m_LastRenderTime);
        ImGui::Text("%.1f FPS", 1000.0f / m_LastRenderTime);
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
                         {(float)image->GetWidth(), (float)image->GetHeight()});
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
