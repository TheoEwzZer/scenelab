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
#include "renderer/interface/ARenderer.hpp"
#include "TextureManager.hpp"
#include "imgui.h"

class RasterizationRenderer;
class TextureManager;

class App {

    bool leftShiftPressed = false;

private:
    SceneGraph m_sceneGraph;
    CameraManager m_camera;

    // Feature managers
    std::unique_ptr<GeometryManager> m_geometryManager;
    std::unique_ptr<TransformManager> m_transformManager;
    std::unique_ptr<CameraController> m_cameraController;
    std::unique_ptr<TextureManager> m_textureManager;

    void init();
    void update();
    void render();
    void updateCursor();

    Vect::UIDrawer vectorial_ui;
    Illumination::UIIllumination illumination_ui;

public:
    explicit App();
    ~App();

    App(const App &) = delete;
    App &operator=(const App &) = delete;

    void run();
    GameObject &registerObject(GameObject &object);

    std::unique_ptr<ARenderer> m_renderer;
    std::unique_ptr<Image> m_image;
    RasterizationRenderer *m_rasterRenderer = nullptr;
};
