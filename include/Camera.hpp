#pragma once

#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/fwd.hpp>
#include <glm/glm.hpp>

class Camera {
    private:
        glm::vec3 m_position{0.0f, 0.0f, 0.0f};
        glm::vec3 m_rotation{0.0f, 0.0f, 0.0f}; // pitch, yaw, roll in degrees

        // Projection parameters
        float m_fov{45.0f};
        float m_aspectRatio{1.0f};
        float m_nearPlane{0.1f};
        float m_farPlane{100.0f};

    public:
        Camera() = default;
        ~Camera() = default;

        void setPosition(const glm::vec3& position) {
            m_position = position;
        }

        void setRotation(float pitch, float yaw, float roll) {
            m_rotation = glm::vec3(pitch, yaw, roll);
        }

        void setProjection(float fov, float aspectRatio, float nearPlane, float farPlane) {
            m_fov = fov;
            m_aspectRatio = aspectRatio;
            m_nearPlane = nearPlane;
            m_farPlane = farPlane;
        }

        const glm::vec3& getPosition() const { return m_position; }
        const glm::vec3& getRotation() const { return m_rotation; }

        glm::mat4 getViewMatrix() const {
            glm::mat4 viewMatrix = glm::mat4(1.0f);
            viewMatrix = glm::rotate(viewMatrix, glm::radians(m_rotation.x), glm::vec3(1, 0, 0));
            viewMatrix = glm::rotate(viewMatrix, glm::radians(m_rotation.y), glm::vec3(0, 1, 0));
            viewMatrix = glm::rotate(viewMatrix, glm::radians(m_rotation.z), glm::vec3(0, 0, 1));
            viewMatrix = glm::translate(viewMatrix, -m_position);
            return viewMatrix;
        }

        glm::mat4 getProjectionMatrix() const {
            return glm::perspective(glm::radians(m_fov), m_aspectRatio, m_nearPlane, m_farPlane);
        }
};