/*
** EPITECH PROJECT, 2024
** Raytracer
** File description:
** App
*/

#pragma once

#include <cstdint>
#include <memory>
#include <utility>
#include <string>

#include "GameObject.hpp"
#include "Camera.hpp"
#include "CameraManager.hpp"
#include "GeometryImguiWindow.hpp"
#include "Vectoriel.hpp"
#include "Image.hpp"
#include "renderer/interface/ARenderer.hpp"
#include "imgui.h"

class App {

    bool wPressed = false;
    bool sPressed = false;
    bool aPressed = false;
    bool dPressed = false;
    bool spacePressed = false;
    bool leftCtrlPressed = false;
    glm::vec2 m_currentMousePos;
    bool firstMouse = false;

    glm::vec2 mouseDelta { 0.0f };
    glm::vec2 prevMousePos { 0.0f };

    bool m_showAllBoundingBoxes { false };
    int64_t selectedObjectIndex = -1;

private:
    std::vector<GameObject> m_gameObjects;
    CameraManager m_camera;
    GeometryImguiWindow m_GeometryImguiWindow;

    void init();
    void update();
    void render();

    void initGeometryWindow();
    void selectedTransformUI();
    void updateCursor();
    void resetAllCameraPoses();
    void renderCameraGizmo(int cameraId, const Camera &camera, ImVec2 imagePos, ImVec2 imageSize, bool isHovered);

    Vect::UIDrawer vectorial_ui;

public:
    explicit App();
    ~App();

    App(const App &) = delete;
    App &operator=(const App &) = delete;

    void run();
    GameObject &registerObject(GameObject &object);

    std::unique_ptr<ARenderer> m_renderer;
    std::unique_ptr<Image> m_image;

    // Current gizmo operation (for cursor state)
    enum class GizmoOp { Translate = 0, Rotate = 1, Scale = 2 };
    GizmoOp m_currentGizmoOperation = GizmoOp::Translate;
};
