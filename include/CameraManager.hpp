//
// Created by clmonn on 10/10/25.
//

#ifndef SCENELAB_CAMERAMANAGER_H
#define SCENELAB_CAMERAMANAGER_H

#include "Camera.hpp"

#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

class CameraManager {
    public:
    explicit CameraManager() = default;
    int getCameraCount() const;
    int createCamera();
    void destroyCamera(int id);
    void setFocused(int id);
    Camera* getCamera(int id);
    Camera* getFocusedCamera();
    const Camera *getFocusedCamera() const;

    // Enumerate cameras
    std::vector<int> getCameraIds() const;

    void setPosition(const glm::vec3& position);
    void setRotation(float pitch, float yaw, float roll);
    void setProjection(float fov, float aspectRatio, float nearPlane, float farPlane);
    glm::vec3 getPosition() const;
    glm::vec3 getRotation() const;
    glm::mat4 getViewMatrix() const;
    glm::mat4 getProjectionMatrix() const;

    // Manipulate a specific camera
    void setPosition(int id, const glm::vec3& position);
    void setRotation(int id, float pitch, float yaw, float roll);
    glm::vec3 getPosition(int id) const;
    glm::vec3 getRotation(int id) const;
    glm::mat4 getViewMatrix(int id) const;
    glm::mat4 getProjectionMatrix(int id) const;

    // Projection configuration per camera
    void setPerspective(int id, float fov, float aspectRatio, float nearPlane, float farPlane);
    void setOrthographic(int id, float size, float aspectRatio, float nearPlane, float farPlane);
    void setProjectionMode(int id, Camera::ProjectionMode mode);
    Camera::ProjectionMode getProjectionMode(int id) const;
    void setFov(int id, float fov);
    float getFov(int id) const;
    void setOrthoSize(int id, float size);
    float getOrthoSize(int id) const;
    std::pair<float, float> getNearFar(int id) const;
    void setAspect(int id, float aspect);

    private:
    std::unordered_map<int, Camera> m_cameras;
    int m_nextID = 1;
    std::optional<int> m_FocusedCamID;
};

#endif // SCENELAB_CAMERAMANAGER_H