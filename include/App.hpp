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
#include "SceneGraph.hpp"
#include "CameraManager.hpp"
#include "GeometryManager.hpp"
#include "TransformManager.hpp"
#include "CameraController.hpp"
#include "Vectoriel.hpp"
#include "Image.hpp"
#include "illumination/Illumination.hpp"
#include "renderer/interface/IRenderer.hpp"
#include "RenderableObject.hpp"
#include "renderer/Window.hpp"
#include "TextureManager.hpp"
#include "shapes/ParametricCurve.hpp"
#include "DynamicGeometryManager.hpp"
#include "imgui.h"

class RasterizationRenderer;
class TextureManager;

class App {

    bool leftShiftPressed = false;

    // Menu bar state
    bool m_showAboutPopup = false;
    bool m_showGeometryWindow = true;
    bool m_showTransformWindow = true;
    bool m_showRayTracingWindow = true;
    bool m_showTextureWindow = true;
    bool m_showCameraWindow = true;
    bool m_showImageWindow = true;
    bool m_showVectorWindow = true;
    bool m_showHierarchyWindow = true;
    bool m_showIlluminationWindow = true;

private:
    Window m_window;
    SceneGraph m_sceneGraph;
    CameraManager m_camera;

    // Feature managers
    std::unique_ptr<DynamicGeometryManager> m_parametricCurveManager;
    std::unique_ptr<GeometryManager> m_geometryManager;
    std::unique_ptr<TransformManager> m_transformManager;
    std::unique_ptr<CameraController> m_cameraController;
    std::unique_ptr<TextureManager> m_textureManager;
    std::vector<std::pair<GameObject *, std::unique_ptr<RenderableObject>>>
        m_helperObjects; // Stored when switching to path tracing

    void init();
    void update();
    void render();
    void renderMainMenuBar();
    void updateCursor();
    void switchRenderer();
    void resetScene();

    Vect::UIDrawer vectorial_ui;
    std::unique_ptr<Illumination::UIIllumination> illumination_ui;

public:
    explicit App();
    ~App();

    App(const App &) = delete;
    App &operator=(const App &) = delete;

    void run();
    GameObject &registerObject(GameObject &object);

    std::unique_ptr<IRenderer> m_renderer;
    std::unique_ptr<Image> m_image;
};
