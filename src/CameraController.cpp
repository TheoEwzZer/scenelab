#include "CameraController.hpp"
#include "imgui.h"
#include "GLFW/glfw3.h"
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>

CameraController::CameraController(
    CameraManager &cameraManager, std::unique_ptr<IRenderer> &renderer) :
    m_cameraManager(cameraManager),
    m_renderer(renderer)
{
}

void CameraController::registerInputCallbacks()
{
    // Register key callbacks
    m_renderer->addKeyCallback(
        GLFW_KEY_W, GLFW_PRESS, [this]() { wPressed = true; });
    m_renderer->addKeyCallback(
        GLFW_KEY_W, GLFW_RELEASE, [this]() { wPressed = false; });
    m_renderer->addKeyCallback(
        GLFW_KEY_S, GLFW_PRESS, [this]() { sPressed = true; });
    m_renderer->addKeyCallback(
        GLFW_KEY_S, GLFW_RELEASE, [this]() { sPressed = false; });
    m_renderer->addKeyCallback(
        GLFW_KEY_A, GLFW_PRESS, [this]() { aPressed = true; });
    m_renderer->addKeyCallback(
        GLFW_KEY_A, GLFW_RELEASE, [this]() { aPressed = false; });
    m_renderer->addKeyCallback(
        GLFW_KEY_D, GLFW_PRESS, [this]() { dPressed = true; });
    m_renderer->addKeyCallback(
        GLFW_KEY_D, GLFW_RELEASE, [this]() { dPressed = false; });
    m_renderer->addKeyCallback(
        GLFW_KEY_SPACE, GLFW_PRESS, [this]() { spacePressed = true; });
    m_renderer->addKeyCallback(
        GLFW_KEY_SPACE, GLFW_RELEASE, [this]() { spacePressed = false; });
    m_renderer->addKeyCallback(GLFW_KEY_LEFT_CONTROL, GLFW_PRESS,
        [this]() { leftCtrlPressed = true; });
    m_renderer->addKeyCallback(GLFW_KEY_LEFT_CONTROL, GLFW_RELEASE,
        [this]() { leftCtrlPressed = false; });

    // Register mouse button callbacks for camera rotation
    m_renderer->addKeyCallback(
        GLFW_MOUSE_BUTTON_2, GLFW_PRESS, [this]() { firstMouse = true; });
    m_renderer->addKeyCallback(
        GLFW_MOUSE_BUTTON_2, GLFW_RELEASE, [this]() { firstMouse = false; });

    // Register mouse movement callback
    m_renderer->addCursorCallback(
        [this](double x, double y) { handleMouseMovement(x, y, firstMouse); });
}

void CameraController::update()
{
    if (wPressed) {
        auto pos = m_cameraManager.getPosition();
        auto rot = m_cameraManager.getRotation();
        pos.x += 0.05f * sin(glm::radians(rot.y));
        pos.z -= 0.05f * cos(glm::radians(rot.y));
        m_cameraManager.setPosition(pos);
    }
    if (sPressed) {
        auto pos = m_cameraManager.getPosition();
        auto rot = m_cameraManager.getRotation();
        pos.x -= 0.05f * sin(glm::radians(rot.y));
        pos.z += 0.05f * cos(glm::radians(rot.y));
        m_cameraManager.setPosition(pos);
    }
    if (aPressed) {
        auto pos = m_cameraManager.getPosition();
        auto rot = m_cameraManager.getRotation();
        pos.x -= 0.05f * cos(glm::radians(rot.y));
        pos.z -= 0.05f * sin(glm::radians(rot.y));
        m_cameraManager.setPosition(pos);
    }
    if (dPressed) {
        auto pos = m_cameraManager.getPosition();
        auto rot = m_cameraManager.getRotation();
        pos.x += 0.05f * cos(glm::radians(rot.y));
        pos.z += 0.05f * sin(glm::radians(rot.y));
        m_cameraManager.setPosition(pos);
    }
    if (spacePressed) {
        auto pos = m_cameraManager.getPosition();
        pos.y += 0.05f;
        m_cameraManager.setPosition(pos);
    }
    if (leftCtrlPressed) {
        auto pos = m_cameraManager.getPosition();
        pos.y -= 0.05f;
        m_cameraManager.setPosition(pos);
    }
}

void CameraController::handleMouseMovement(double x, double y, bool isRotating)
{
    m_currentMousePos = glm::vec2(x, y);
    mouseDelta = m_currentMousePos - prevMousePos;
    prevMousePos = m_currentMousePos;

    // Only rotate if right mouse button is held
    if (glm::length(mouseDelta) > 0.0f && isRotating) {
        auto rot = m_cameraManager.getRotation();
        rot.y += mouseDelta.x * 0.1f;
        rot.x += mouseDelta.y * 0.1f;
        m_cameraManager.setRotation(rot.x, rot.y, rot.z);
    }
}

void CameraController::renderCameraManagerUI()
{
    ImGui::Begin("Camera");

    // Create / Destroy
    if (ImGui::Button("Create Camera")) {
        const int id = m_cameraManager.createCamera();
        m_cameraManager.setFocused(id);
        m_cameraManager.setPosition({ 0.0f, 0.0f, 3.0f });
        m_cameraManager.setPerspective(id, 45.0f, 16.0f / 9.0f, 0.01f, 100.0f);
        m_renderer->createCameraViews(id, 512, 512);
    }

    ImGui::Separator();
    // List and select focus
    auto ids = m_cameraManager.getCameraIds();
    for (int id : ids) {
        ImGui::PushID(id);
        if (ImGui::SmallButton("Focus")) {
            m_cameraManager.setFocused(id);
        }
        ImGui::SameLine();
        ImGui::Text("Camera %d", id);
        ImGui::SameLine();
        if (ImGui::SmallButton("Destroy")) {
            m_renderer->destroyCameraViews(id);
            m_cameraManager.destroyCamera(id);
            ImGui::PopID();
            continue; // move to next id (this one is gone)
        }

        if (auto *cam = m_cameraManager.getCamera(id)) {
            // Pose controls
            glm::vec3 pos = cam->getPosition();
            glm::vec3 rot = cam->getRotation();
            if (ImGui::DragFloat3("Position##pos", &pos.x, 0.05f)) {
                m_cameraManager.setPosition(id, pos);
            }
            if (ImGui::DragFloat3("Rotation (deg)##rot", &rot.x, 0.1f)) {
                m_cameraManager.setRotation(id, rot.x, rot.y, rot.z);
            }

            // Projection controls
            bool isPerspective = cam->getProjectionMode()
                == Camera::ProjectionMode::Perspective;
            if (ImGui::Checkbox("Perspective##mode", &isPerspective)) {
                m_cameraManager.setProjectionMode(id,
                    isPerspective ? Camera::ProjectionMode::Perspective
                                  : Camera::ProjectionMode::Orthographic);
            }
            if (isPerspective) {
                float fov = cam->getFov();
                if (ImGui::DragFloat("FOV##fov_global", &fov, 0.1f, 10.0f,
                        160.0f, "%.1f")) {
                    m_cameraManager.setFov(id, fov);
                }
            } else {
                float size = cam->getOrthoSize();
                if (ImGui::DragFloat("Ortho Size##ortho_global", &size, 0.05f,
                        0.01f, 100.0f, "%.2f")) {
                    m_cameraManager.setOrthoSize(id, size);
                }
            }
        }
        ImGui::Separator();
        ImGui::PopID();
    }

    ImGui::End();
}

void CameraController::resetAllCameraPoses()
{
    for (int id : m_cameraManager.getCameraIds()) {
        if (auto *cam = m_cameraManager.getCamera(id)) {
            cam->setPosition(glm::vec3(0.0f, 0.0f, 3.0f));
            cam->setRotation(0.0f, 0.0f, 0.0f);
        }
    }
}
