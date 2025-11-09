#pragma once

#include "CameraManager.hpp"
#include "renderer/interface/ARenderer.hpp"
#include <memory>
#include <glm/glm.hpp>

class CameraController {
public:
    CameraController(
        CameraManager &cameraManager, std::unique_ptr<ARenderer> &renderer);

    // Register input callbacks
    void registerInputCallbacks();

    // Update camera position based on input
    void update();

    // Camera UI
    void renderCameraManagerUI();

    // Reset all camera poses
    void resetAllCameraPoses();

    // Mouse handling for camera rotation
    void handleMouseMovement(double x, double y, bool isRotating);

private:
    CameraManager &m_cameraManager;
    std::unique_ptr<ARenderer> &m_renderer;

    // Input states
    bool wPressed = false;
    bool sPressed = false;
    bool aPressed = false;
    bool dPressed = false;
    bool spacePressed = false;
    bool leftCtrlPressed = false;

    // Mouse state
    glm::vec2 m_currentMousePos = { 0.0f, 0.0f };
    glm::vec2 prevMousePos = { 0.0f, 0.0f };
    glm::vec2 mouseDelta = { 0.0f, 0.0f };
    bool firstMouse = false;
};
