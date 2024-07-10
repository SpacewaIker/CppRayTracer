#pragma once

#include <glm/glm.hpp>
#include <vector>

/// Camera class that handles the camera movement and projection matrices.
class Camera {
  public:
    /**
     * @param verticalFOV Vertical field of view in degrees.
     * @param nearPlane Near plane distance.
     * @param farPlane Far plane distance.
     * @param position Camera position.
     * @param forwardDirection Camera forward direction.
     */
    Camera(float verticalFOV, float nearPlane, float farPlane, glm::vec3 position,
           glm::vec3 forwardDirection)
        : m_VerticalFOV(verticalFOV), m_NearPlane(nearPlane), m_FarPlane(farPlane),
          m_Position(position), m_ForwardDirection(forwardDirection) {}

    /**
     * @param sensorHeight Sensor height in mm.
     * @param focalLength Focal length in mm.
     * @param nearPlane Near plane distance.
     * @param farPlane Far plane distance.
     * @param position Camera position.
     * @param forwardDirection Camera forward direction.
     */
    Camera(float sensorHeight, float focalLength, float nearPlane, float farPlane,
           glm::vec3 position, glm::vec3 forwardDirection)
        : m_NearPlane(nearPlane), m_FarPlane(farPlane), m_Position(position),
          m_ForwardDirection(forwardDirection) {
        m_VerticalFOV = 2.0f * glm::degrees(atan(sensorHeight / (2.0f * focalLength)));
    }

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

    const glm::mat4 &GetProjectionMatrix() const { return m_ProjectionMatrix; }
    const glm::mat4 &GetViewMatrix() const { return m_ViewMatrix; }
    const glm::mat4 &GetInverseProjection() const { return m_InverseProjection; }
    const glm::mat4 &GetInverseView() const { return m_InverseView; }

    const glm::vec3 &GetPosition() const { return m_Position; }
    const glm::vec3 &GetForwardDirection() const { return m_ForwardDirection; }

    const std::vector<glm::vec3> &GetRayDirections() const { return m_RayDirections; }

    float GetRotationSpeed();

  private:
    void RecalculateProjectionMatrix();
    void RecalculateViewMatrix();
    void RecalculateRayDirections();

  private:
    glm::mat4 m_ProjectionMatrix{1.0f};
    glm::mat4 m_ViewMatrix{1.0f};
    glm::mat4 m_InverseProjection{1.0f};
    glm::mat4 m_InverseView{1.0f};

    float m_VerticalFOV = 45.0f; // in degrees
    float m_NearPlane = 0.1f;
    float m_FarPlane = 100.0f;

    glm::vec3 m_Position{0.0f, 0.0f, 0.0f};
    glm::vec3 m_ForwardDirection{0.0f, 0.0f, -1.0f};

    std::vector<glm::vec3> m_RayDirections;

    glm::vec2 m_LastMousePosition{0.0f, 0.0f};

    uint32_t m_ViewportWidth = 0;
    uint32_t m_ViewportHeight = 0;

  public:
    float m_MovementSpeed = 5.0f;
    float m_MouseSensitivity = 0.008f;
};
