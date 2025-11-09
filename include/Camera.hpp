#pragma once

#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/fwd.hpp>
#include <glm/glm.hpp>

class Camera {
public:
    enum class ProjectionMode { Perspective, Orthographic };

private:
    glm::vec3 m_position { 0.0f, 0.0f, 0.0f };
    glm::vec3 m_rotation { 0.0f, 0.0f, 0.0f }; // pitch, yaw, roll in degrees

    // Projection parameters
    ProjectionMode m_mode { ProjectionMode::Perspective };
    float m_fov { 45.0f };
    float m_aspectRatio { 1.0f };
    float m_nearPlane { 0.1f };
    float m_farPlane { 100.0f };
    // For orthographic projection, size represents the half-height (top =
    // size)
    float m_orthoSize { 5.0f };

public:
    Camera() = default;
    ~Camera() = default;

    void setPosition(const glm::vec3 &position) { m_position = position; }

    void setRotation(float pitch, float yaw, float roll)
    {
        m_rotation = glm::vec3(pitch, yaw, roll);
    }

    void setProjection(
        float fov, float aspectRatio, float nearPlane, float farPlane)
    {
        m_mode = ProjectionMode::Perspective;
        m_fov = fov;
        m_aspectRatio = aspectRatio;
        m_nearPlane = nearPlane;
        m_farPlane = farPlane;
    }

    void setPerspective(
        float fov, float aspectRatio, float nearPlane, float farPlane)
    {
        setProjection(fov, aspectRatio, nearPlane, farPlane);
    }

    void setOrthographic(
        float size, float aspectRatio, float nearPlane, float farPlane)
    {
        m_mode = ProjectionMode::Orthographic;
        m_orthoSize = size;
        m_aspectRatio = aspectRatio;
        m_nearPlane = nearPlane;
        m_farPlane = farPlane;
    }

    void setAspect(float aspectRatio) { m_aspectRatio = aspectRatio; }

    void setFov(float fov) { m_fov = fov; }

    void setOrthoSize(float size) { m_orthoSize = size; }

    void setProjectionMode(ProjectionMode mode) { m_mode = mode; }

    const glm::vec3 &getPosition() const { return m_position; }

    const glm::vec3 &getRotation() const { return m_rotation; }

    float getFov() const { return m_fov; }

    float getAspectRatio() const { return m_aspectRatio; }

    float getNearPlane() const { return m_nearPlane; }

    float getFarPlane() const { return m_farPlane; }

    float getOrthoSize() const { return m_orthoSize; }

    ProjectionMode getProjectionMode() const { return m_mode; }

    glm::mat4 getViewMatrix() const
    {
        glm::mat4 viewMatrix = glm::mat4(1.0f);
        viewMatrix = glm::rotate(
            viewMatrix, glm::radians(m_rotation.x), glm::vec3(1, 0, 0));
        viewMatrix = glm::rotate(
            viewMatrix, glm::radians(m_rotation.y), glm::vec3(0, 1, 0));
        viewMatrix = glm::rotate(
            viewMatrix, glm::radians(m_rotation.z), glm::vec3(0, 0, 1));
        viewMatrix = glm::translate(viewMatrix, -m_position);
        return viewMatrix;
    }

    glm::mat4 getProjectionMatrix() const
    {
        if (m_mode == ProjectionMode::Perspective) {
            return glm::perspective(
                glm::radians(m_fov), m_aspectRatio, m_nearPlane, m_farPlane);
        }
        const float top = m_orthoSize;
        const float bottom = -m_orthoSize;
        const float right = m_orthoSize * m_aspectRatio;
        const float left = -right;
        return glm::ortho(left, right, bottom, top, m_nearPlane, m_farPlane);
    }
};