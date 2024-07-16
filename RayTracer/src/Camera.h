#pragma once

#include <glm/glm.hpp>
#include <vector>

/// Camera class that handles the camera movement and projection matrices.
class Camera {
  public:
    struct CameraSettings {
        float VerticalFOV;
        float NearPlane;
        float FarPlane;
        glm::vec3 Position;
        glm::vec3 ForwardDirection;
    };

  public:
    /**
     * @param verticalFOV Vertical field of view in degrees.
     * @param nearPlane Near plane distance.
     * @param farPlane Far plane distance.
     * @param position Camera position.
     * @param forwardDirection Camera forward direction.
     */
    Camera(float verticalFOV, float nearPlane, float farPlane, glm::vec3 position, glm::vec3 forwardDirection)
        : m_Settings({verticalFOV, nearPlane, farPlane, position, forwardDirection}) {}

    /**
     * Updates the camera position and rotation based on the input.
     * @param deltaTime Time since the last frame.
     * @return true if the camera has moved.
     */
    bool OnUpdate(float deltaTime);
    /**
     * Updates the camera projection matrix and the ray directions when the window is resized.
     * @param width New window width.
     * @param height New window height.
     */
    void OnResize(uint32_t width, uint32_t height);
    /**
     * Updates the camera data when the settings are changed.
     */
    void OnChangeSettings();

    const glm::mat4 &GetProjectionMatrix() const { return m_ProjectionMatrix; }
    const glm::mat4 &GetViewMatrix() const { return m_ViewMatrix; }
    const glm::mat4 &GetInverseProjection() const { return m_InverseProjection; }
    const glm::mat4 &GetInverseView() const { return m_InverseView; }

    const std::vector<glm::vec3> &GetRayDirections() const { return m_RayDirections; }

    CameraSettings &GetSettings() { return m_Settings; }
    const CameraSettings &GetSettings() const { return m_Settings; }

    float GetRotationSpeed();

  private:
    void RecalculateProjectionMatrix();
    void RecalculateViewMatrix();
    void RecalculateRayDirections();

  private:
    CameraSettings m_Settings;

    glm::mat4 m_ProjectionMatrix{1.0f};
    glm::mat4 m_ViewMatrix{1.0f};
    glm::mat4 m_InverseProjection{1.0f};
    glm::mat4 m_InverseView{1.0f};

    std::vector<glm::vec3> m_RayDirections;

    glm::vec2 m_LastMousePosition{0.0f, 0.0f};

    uint32_t m_ViewportWidth = 0;
    uint32_t m_ViewportHeight = 0;

  public:
    float m_MovementSpeed = 5.0f;
    float m_MouseSensitivity = 0.008f;
};
