
#include "Camera.h"

#include "Walnut/Input/Input.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

bool Camera::OnUpdate(float deltaTime) {
    glm::vec2 mousePosition = Walnut::Input::GetMousePosition();
    glm::vec2 delta = (mousePosition - m_LastMousePosition) * m_MouseSensitivity;
    m_LastMousePosition = mousePosition;

    // if mouse is not pressed, do nothing
    if (!Walnut::Input::IsMouseButtonDown(Walnut::MouseButton::Right)) {
        Walnut::Input::SetCursorMode(Walnut::CursorMode::Normal);
        return false;
    }

    // restores cursor position when the button is released
    Walnut::Input::SetCursorMode(Walnut::CursorMode::Locked);

    bool moved = false;

    constexpr glm::vec3 UP(0.0f, 1.0f, 0.0f);
    glm::vec3 right = glm::cross(m_Settings.ForwardDirection, UP);

    // movement controls
    if (Walnut::Input::IsKeyDown(Walnut::Key::W)) { // forward
        m_Settings.Position += m_Settings.ForwardDirection * m_MovementSpeed * deltaTime;
        moved = true;
    } else if (Walnut::Input::IsKeyDown(Walnut::Key::S)) { // backward
        m_Settings.Position -= m_Settings.ForwardDirection * m_MovementSpeed * deltaTime;
        moved = true;
    } else if (Walnut::Input::IsKeyDown(Walnut::Key::A)) { // left
        m_Settings.Position -= right * m_MovementSpeed * deltaTime;
        moved = true;
    } else if (Walnut::Input::IsKeyDown(Walnut::Key::D)) { // right
        m_Settings.Position += right * m_MovementSpeed * deltaTime;
        moved = true;
    } else if (Walnut::Input::IsKeyDown(Walnut::Key::Q)) { // down
        m_Settings.Position -= UP * m_MovementSpeed * deltaTime;
        moved = true;
    } else if (Walnut::Input::IsKeyDown(Walnut::Key::E)) { // up
        m_Settings.Position += UP * m_MovementSpeed * deltaTime;
        moved = true;
    }

    // rotation controls
    if (delta.x != 0.0f || delta.y != 0.0f) {
        float pitchDelta = delta.y * GetRotationSpeed();
        float yawDelta = delta.x * GetRotationSpeed();

        glm::quat q = glm::normalize(glm::cross(glm::angleAxis(-pitchDelta, right), glm::angleAxis(-yawDelta, UP)));
        m_Settings.ForwardDirection = glm::rotate(q, m_Settings.ForwardDirection);

        moved = true;
    }

    if (moved) {
        RecalculateViewMatrix();
        RecalculateRayDirections();
    }

    return moved;
}

void Camera::OnResize(uint32_t width, uint32_t height) {
    // no change
    if (m_ViewportWidth == width && m_ViewportHeight == height) {
        return;
    }

    m_ViewportWidth = width;
    m_ViewportHeight = height;

    RecalculateProjectionMatrix();
    RecalculateViewMatrix();
    RecalculateRayDirections();
}

float Camera::GetRotationSpeed() { return 0.3f; }

void Camera::RecalculateProjectionMatrix() {
    m_ProjectionMatrix = glm::perspectiveFov(glm::radians(m_Settings.VerticalFOV), (float)m_ViewportWidth,
                                             (float)m_ViewportHeight, m_Settings.NearPlane, m_Settings.FarPlane);
    m_InverseProjection = glm::inverse(m_ProjectionMatrix);
}

void Camera::RecalculateViewMatrix() {
    m_ViewMatrix =
        glm::lookAt(m_Settings.Position, m_Settings.Position + m_Settings.ForwardDirection, glm::vec3(0.0f, 1.0f, 0.0f));
    m_InverseView = glm::inverse(m_ViewMatrix);
}

void Camera::RecalculateRayDirections() {
    m_RayDirections.resize(m_ViewportWidth * m_ViewportHeight);

    for (uint32_t y = 0; y < m_ViewportHeight; y++) {
        for (uint32_t x = 0; x < m_ViewportWidth; x++) {
            glm::vec2 coords = {(float)x / (float)m_ViewportWidth, (float)y / (float)m_ViewportHeight};
            coords = coords * 2.0f - 1.0f; // map to [-1, 1]

            glm::vec4 target = m_InverseProjection * glm::vec4(coords, -1.0, 1.0);
            glm::vec3 rayDir = glm::vec3(m_InverseView * glm::vec4(glm::normalize(glm::vec3(target) / target.w), 0));
            m_RayDirections[y * m_ViewportWidth + x] = rayDir;
        }
    }
}

void Camera::OnChangeSettings() {
    RecalculateProjectionMatrix();
    RecalculateViewMatrix();
    RecalculateRayDirections();
}
