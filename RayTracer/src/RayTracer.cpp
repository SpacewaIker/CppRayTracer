#include "Camera.h"
#include "Renderer.h"
#include "Scene.h"
#include "Walnut/Application.h"
#include "Walnut/EntryPoint.h"
#include "Walnut/Image.h"
#include "Walnut/Timer.h"
#include "kompute/Algorithm.hpp"
#include "kompute/Core.hpp"
#include "kompute/Manager.hpp"
#include "kompute/operations/OpAlgoDispatch.hpp"
#include "kompute/operations/OpTensorSyncDevice.hpp"
#include "kompute/operations/OpTensorSyncLocal.hpp"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include <filesystem>
#include <glm/gtc/type_ptr.hpp>
#include <memory>

#define KOMPUTE_DEVICE_INDEX 1

std::vector<uint32_t> compileSource(const std::string &source) {
    std::ofstream fileOut("tmp_kp_shader.comp");
    fileOut << source;
    fileOut.close();
    if (system(std::string("glslangValidator -V tmp_kp_shader.comp -o tmp_kp_shader.comp.spv").c_str())) {
        throw std::runtime_error("Error running glslangValidator command");
    }
    std::ifstream fileStream("tmp_kp_shader.comp.spv", std::ios::binary);
    std::vector<char> buffer;
    buffer.insert(buffer.begin(), std::istreambuf_iterator<char>(fileStream), {});
    return {reinterpret_cast<uint32_t *>(buffer.data()), reinterpret_cast<uint32_t *>(buffer.data() + buffer.size())};
}

class MainLayer : public Walnut::Layer {
  public:
    MainLayer() : m_Camera(SceneLoader::LoadCameraSettings("camera.toml")), m_Scene(SceneLoader::LoadScene("scene.toml")) {
        m_SpirVShader = compileSource(R"(
            #version 450

            layout (local_size_x = 1) in;

            layout(set = 0, binding = 0) buffer buf_out_a { uint out_a[]; };

            layout(constant_id = 0) const float viewportWidth = 0;
            layout(constant_id = 1) const float viewportHeight = 0;

            void main() {
                for (uint i = 0; i < uint(viewportWidth); i++) {
                    for (uint j = 0; j < uint(viewportHeight); j++) {
                        uint index = i + j * uint(viewportWidth);
                        uint red = uint(float(i) / float(viewportWidth) * 255.0);
                        uint green = uint(float(j) / float(viewportHeight) * 255.0);
                        out_a[index] = 0xFF000000 | (green << 8) | red;
                    }
                }
            }
        )");

        // initialize vector of size m_ViewportWidth *m_ViewportHeight
        std::vector<uint32_t> data = std::vector<uint32_t>(m_ViewportWidth * m_ViewportHeight, 0);
        m_TensorOut = m_KomputeManager.tensorT<uint32_t>(data);

        m_TensorParams = {m_TensorOut};

        kp::Workgroup workgroup({m_TensorOut->size(), 1, 1});
        std::vector<float> specConsts({(float)m_ViewportWidth, (float)m_ViewportHeight});

        m_Algorithm = m_KomputeManager.algorithm(m_TensorParams, m_SpirVShader, workgroup, specConsts);

        m_KomputeManager.sequence()
            ->record<kp::OpTensorSyncDevice>(m_TensorParams)
            ->record<kp::OpAlgoDispatch>(m_Algorithm)
            ->eval();
    }

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

        // scene window
        ImGui::Begin("Scene");

        bool sceneChanged = false;

        // global scene elements
        sceneChanged |= ImGui::SliderFloat("Vertical FOV", &m_Camera.GetSettings().VerticalFOV, 1.0f, 179.0f);
        sceneChanged |= ImGui::SliderFloat("Near Plane", &m_Camera.GetSettings().NearPlane, 0.01f, 1.0f);
        sceneChanged |= ImGui::SliderFloat("Far Plane", &m_Camera.GetSettings().FarPlane, 10.0f, 1000.0f);
        sceneChanged |= ImGui::DragFloat3("Camera Position", glm::value_ptr(m_Camera.GetSettings().Position), 0.1f);
        sceneChanged |=
            ImGui::DragFloat3("Camera Forward Direction", glm::value_ptr(m_Camera.GetSettings().ForwardDirection), 0.1f);
        sceneChanged |= ImGui::ColorEdit3("Sky Colour", glm::value_ptr(m_Scene.SkyColour));

        // save scene on ui change
        if (sceneChanged) {
            m_Renderer.ResetFrameIndex();
            m_Camera.OnChangeSettings();
            SceneLoader::SaveScene("saved_scene.toml", m_Scene, m_Camera);
        }

        ImGui::End();

        Render();
    }

    void Render() {
        if (!m_ShouldRender) {
            return;
        }

        Walnut::Timer timer;

        // m_Renderer.OnResize(m_ViewportWidth, m_ViewportHeight);
        // m_Camera.OnResize(m_ViewportWidth, m_ViewportHeight);
        // m_Renderer.Render(m_Scene, m_Camera);

        auto sq = m_KomputeManager.sequence();
        sq->evalAsync<kp::OpTensorSyncLocal>(m_TensorParams);
        sq->evalAwait();

        m_Renderer.SetImageData(m_TensorOut->data());

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

    uint32_t m_ViewportWidth = 594;
    uint32_t m_ViewportHeight = 424;

    float m_LastRenderTime = 0.0f;

    char m_SaveFilename[256] = "output";

    std::vector<uint32_t> m_SpirVShader = {};
    std::shared_ptr<kp::TensorT<uint32_t>> m_TensorOut = nullptr;
    kp::Manager m_KomputeManager{KOMPUTE_DEVICE_INDEX};
    std::vector<std::shared_ptr<kp::Tensor>> m_TensorParams = {};
    std::shared_ptr<kp::Algorithm> m_Algorithm = nullptr;
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
