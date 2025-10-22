//
// Created by clmonn on 10/10/25.
//

#include "../include/CameraManager.hpp"

#include <ranges>

int CameraManager::getCameraCount() const { return m_cameras.size(); }

int CameraManager::createCamera()
{
    const int id = m_nextID++;
    m_cameras[id] = Camera {};
    return id;
}

void CameraManager::destroyCamera(const int id)
{
    m_cameras.erase(id);

    if (m_FocusedCamID && *m_FocusedCamID == id) {
        if (!m_cameras.empty()) {
            m_FocusedCamID = m_cameras.begin()->first;
        } else {
            m_FocusedCamID.reset();
        }
    }
}

void CameraManager::setFocused(int id) { m_FocusedCamID = id; }

Camera *CameraManager::getCamera(const int id)
{
    const auto it = m_cameras.find(id);
    return (it != m_cameras.end()) ? &it->second : nullptr;
}

Camera *CameraManager::getFocusedCamera()
{
    if (!m_FocusedCamID) {
        return nullptr;
    }
    const auto it = m_cameras.find(*m_FocusedCamID);
    return (it != m_cameras.end()) ? &it->second : nullptr;
}

const Camera *CameraManager::getFocusedCamera() const
{
    if (!m_FocusedCamID) {
        return nullptr;
    }
    const auto it = m_cameras.find(*m_FocusedCamID);
    return (it != m_cameras.end()) ? &it->second : nullptr;
}

std::vector<int> CameraManager::getCameraIds() const
{
    std::vector<int> ids;
    ids.reserve(m_cameras.size());
    for (const auto &kv : m_cameras) {
        ids.push_back(kv.first);
    }
    return ids;
}

void CameraManager::setPosition(const glm::vec3 &position)
{
    if (auto *cam = getFocusedCamera()) {
        cam->setPosition(position);
    }
}

void CameraManager::setRotation(
    const float pitch, const float yaw, const float roll)
{
    if (auto *cam = getFocusedCamera()) {
        cam->setRotation(pitch, yaw, roll);
    }
}

void CameraManager::setProjection(const float fov, const float aspectRatio,
    const float nearPlane, const float farPlane)
{
    if (auto *cam = getFocusedCamera()) {
        cam->setProjection(fov, aspectRatio, nearPlane, farPlane);
    }
}

glm::vec3 CameraManager::getPosition() const
{
    if (auto *cam = getFocusedCamera()) {
        return cam->getPosition();
    }
    return glm::vec3(0, 0, 0);
}

glm::vec3 CameraManager::getRotation() const
{
    if (auto *cam = getFocusedCamera()) {
        return cam->getRotation();
    }
    return glm::vec3(0, 0, 0);
}

glm::mat4 CameraManager::getViewMatrix() const
{
    if (auto *cam = getFocusedCamera()) {
        return cam->getViewMatrix();
    }
    return glm::mat4(1.0f);
}

glm::mat4 CameraManager::getProjectionMatrix() const
{
    if (auto *cam = getFocusedCamera()) {
        return cam->getProjectionMatrix();
    }
    return glm::mat4(1.0f);
}

void CameraManager::setPosition(const int id, const glm::vec3 &position)
{
    if (auto *cam = getCamera(id)) {
        cam->setPosition(position);
    }
}

void CameraManager::setRotation(
    const int id, const float pitch, const float yaw, const float roll)
{
    if (auto *cam = getCamera(id)) {
        cam->setRotation(pitch, yaw, roll);
    }
}

glm::vec3 CameraManager::getPosition(const int id) const
{
    if (const_cast<CameraManager *>(this)->getCamera(id)) {
        return const_cast<CameraManager *>(this)->getCamera(id)->getPosition();
    }
    return glm::vec3(0, 0, 0);
}

glm::vec3 CameraManager::getRotation(const int id) const
{
    if (const_cast<CameraManager *>(this)->getCamera(id)) {
        return const_cast<CameraManager *>(this)->getCamera(id)->getRotation();
    }
    return glm::vec3(0, 0, 0);
}

glm::mat4 CameraManager::getViewMatrix(const int id) const
{
    if (const_cast<CameraManager *>(this)->getCamera(id)) {
        return const_cast<CameraManager *>(this)
            ->getCamera(id)
            ->getViewMatrix();
    }
    return glm::mat4(1.0f);
}

glm::mat4 CameraManager::getProjectionMatrix(const int id) const
{
    if (const_cast<CameraManager *>(this)->getCamera(id)) {
        return const_cast<CameraManager *>(this)
            ->getCamera(id)
            ->getProjectionMatrix();
    }
    return glm::mat4(1.0f);
}

void CameraManager::setPerspective(const int id, float fov, float aspectRatio,
    float nearPlane, float farPlane)
{
    if (auto *cam = getCamera(id)) {
        cam->setPerspective(fov, aspectRatio, nearPlane, farPlane);
    }
}

void CameraManager::setOrthographic(const int id, float size,
    float aspectRatio, float nearPlane, float farPlane)
{
    if (auto *cam = getCamera(id)) {
        cam->setOrthographic(size, aspectRatio, nearPlane, farPlane);
    }
}

void CameraManager::setProjectionMode(
    const int id, Camera::ProjectionMode mode)
{
    if (auto *cam = getCamera(id)) {
        cam->setProjectionMode(mode);
    }
}

Camera::ProjectionMode CameraManager::getProjectionMode(const int id) const
{
    if (const_cast<CameraManager *>(this)->getCamera(id)) {
        return const_cast<CameraManager *>(this)
            ->getCamera(id)
            ->getProjectionMode();
    }
    return Camera::ProjectionMode::Perspective;
}

void CameraManager::setFov(const int id, float fov)
{
    if (auto *cam = getCamera(id)) {
        cam->setFov(fov);
    }
}

float CameraManager::getFov(const int id) const
{
    if (const_cast<CameraManager *>(this)->getCamera(id)) {
        return const_cast<CameraManager *>(this)->getCamera(id)->getFov();
    }
    return 45.0f;
}

void CameraManager::setOrthoSize(const int id, float size)
{
    if (auto *cam = getCamera(id)) {
        cam->setOrthoSize(size);
    }
}

float CameraManager::getOrthoSize(const int id) const
{
    if (const_cast<CameraManager *>(this)->getCamera(id)) {
        return const_cast<CameraManager *>(this)
            ->getCamera(id)
            ->getOrthoSize();
    }
    return 5.0f;
}

std::pair<float, float> CameraManager::getNearFar(const int id) const
{
    if (const_cast<CameraManager *>(this)->getCamera(id)) {
        return {
            const_cast<CameraManager *>(this)->getCamera(id)->getNearPlane(),
            const_cast<CameraManager *>(this)->getCamera(id)->getFarPlane()
        };
    }
    return { 0.1f, 100.0f };
}

void CameraManager::setAspect(const int id, float aspect)
{
    if (auto *cam = getCamera(id)) {
        cam->setAspect(aspect);
    }
}