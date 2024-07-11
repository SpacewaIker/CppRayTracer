#include "Camera.h"
#include "Renderer.h"
#include "Scene.h"
#include "Walnut/Application.h"
#include "Walnut/EntryPoint.h"
#include "Walnut/Image.h"
#include "Walnut/Timer.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include <filesystem>
#include <glm/gtc/type_ptr.hpp>
#include <memory>

class MainLayer : public Walnut::Layer {
  public:
    MainLayer() : m_Camera(SceneLoader::LoadCameraSettings("camera.toml")), m_Scene(SceneLoader::LoadScene("scene.toml")) {}

    virtual void OnUpdate(float deltaTime) override {
        if (m_Camera.OnUpdate(deltaTime)) { // if camera moved
            m_Renderer.ResetFrameIndex();
        }
    }

    virtual void OnUIRender() override {
        // settings window
        ImGui::Begin("Settings");
        ImGui::Checkbox("Render", &m_ShouldRender);
        ImGui::Text("Last Render Time: %.3f ms", m_LastRenderTime);
        ImGui::Text("%.1f FPS", 1000.0f / m_LastRenderTime);
        ImGui::Text("Render Resolution: %dx%d", m_ViewportWidth, m_ViewportHeight);
        ImGui::SliderFloat("Render Scale", &m_Renderer.GetSettings().RenderScale, 0.1f, 1.0f);
        ImGui::SliderInt("Max Bounces", &m_Renderer.GetSettings().MaxBounces, 1, 10);
        ImGui::Checkbox("Accumulate", &m_Renderer.GetSettings().Accumulate);
        ImGui::Checkbox("Jitter", &m_Renderer.GetSettings().Jitter);
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
            SaveImage();
        }

        ImGui::End();

        // viewport window
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
        ImGui::Begin("Viewport");

        m_ViewportWidth = (uint32_t)(m_Renderer.GetSettings().RenderScale * ImGui::GetContentRegionAvail().x);
        m_ViewportHeight = (uint32_t)(m_Renderer.GetSettings().RenderScale * ImGui::GetContentRegionAvail().y);

        auto image = m_Renderer.GetFinalImage();
        if (image) {
            ImGui::Image(image->GetDescriptorSet(), ImGui::GetContentRegionAvail(), ImVec2(0, 1), ImVec2(1, 0));
        }

        ImGui::End();
        ImGui::PopStyleVar();

        Render();
    }

    void Render() {
        if (!m_ShouldRender) {
            return;
        }

        Walnut::Timer timer;

        m_Renderer.OnResize(m_ViewportWidth, m_ViewportHeight);
        m_Camera.OnResize(m_ViewportWidth, m_ViewportHeight);
        m_Renderer.Render(m_Scene, m_Camera);

        m_LastRenderTime = timer.ElapsedMillis();
    }

  private:
    void SaveImage() {
        auto image = m_Renderer.GetImageData();
        if (image) {
            // flip image vertically
            uint32_t *flippedImage = new uint32_t[m_ViewportWidth * m_ViewportHeight];
            for (uint32_t y = 0; y < m_ViewportHeight; y++) {
                memcpy(&flippedImage[y * m_ViewportWidth], &image[(m_ViewportHeight - y - 1) * m_ViewportWidth],
                       m_ViewportWidth * 4);
            }

            std::filesystem::create_directory("snapshots");

            const std::string filename = "snapshots/" + std::string(m_SaveFilename) + ".png";
            stbi_write_png(filename.c_str(), m_ViewportWidth, m_ViewportHeight, 4, flippedImage, m_ViewportWidth * 4);
        }
    }

  private:
    bool m_ShouldRender = true;

    Renderer m_Renderer;
    Camera m_Camera;
    Scene m_Scene;

    uint32_t m_ViewportWidth = 0;
    uint32_t m_ViewportHeight = 0;

    float m_LastRenderTime = 0.0f;

    char m_SaveFilename[256] = "output";
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
