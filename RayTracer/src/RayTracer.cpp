#include "Walnut/Application.h"
#include "Walnut/EntryPoint.h"

#include "Walnut/Image.h"
#include "Walnut/Random.h"
#include "Walnut/Timer.h"

class ExampleLayer : public Walnut::Layer {
private:
  std::shared_ptr<Walnut::Image> m_Image;
  uint32_t *m_ImageData = nullptr;

  uint32_t m_ViewportWidth = 0;
  uint32_t m_ViewportHeight = 0;

  float m_LastRenderTime = 0.0f;

public:
  virtual void OnUIRender() override {
    // settings window
    ImGui::Begin("Settings");
    ImGui::Text("Last Render Time: %.3f ms", m_LastRenderTime);
    if (ImGui::Button("Render")) {
      Render();
    }
    ImGui::End();

    // viewport window
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::Begin("Viewport");

    m_ViewportWidth = ImGui::GetContentRegionAvail().x;
    m_ViewportHeight = ImGui::GetContentRegionAvail().y;

    if (m_Image)
      ImGui::Image(m_Image->GetDescriptorSet(),
                   {(float)m_Image->GetWidth(), (float)m_Image->GetHeight()});

    ImGui::End();
    ImGui::PopStyleVar();

    Render();
  }

  void Render() {
    Walnut::Timer timer;

    if (!m_Image || m_ViewportWidth != m_Image->GetWidth() ||
        m_ViewportHeight != m_Image->GetHeight()) {
      m_Image = std::make_shared<Walnut::Image>(
          m_ViewportWidth, m_ViewportHeight, Walnut::ImageFormat::RGBA);
      delete[] m_ImageData;
      m_ImageData = new uint32_t[m_ViewportWidth * m_ViewportHeight];
    }

    for (uint32_t i = 0; i < m_ViewportWidth * m_ViewportHeight; i++) {
      m_ImageData[i] = Walnut::Random::UInt();
      m_ImageData[i] |= 0xff000000;
    }

    m_Image->SetData(m_ImageData);

    m_LastRenderTime = timer.ElapsedMillis();
  }
};

Walnut::Application *Walnut::CreateApplication(int argc, char **argv) {
  Walnut::ApplicationSpecification spec;
  spec.Name = "Walnut Example";

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
