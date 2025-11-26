#include "App.hpp"
#include "renderer/implementation/RasterizationRenderer.hpp"

#include "Camera.hpp"
#include "GLFW/glfw3.h"

#include "imgui.h"
#include "ImGuizmo.h"
#include "GeometryGenerator.hpp"
#include <glm/geometric.hpp>
#include <glm/matrix.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "objects/Light.hpp"
#include "renderer/interface/ARenderer.hpp"

App::App()
{
    m_renderer = std::make_unique<RasterizationRenderer>();
    m_rasterRenderer = static_cast<RasterizationRenderer *>(m_renderer.get());

    // Initialize managers
    m_geometryManager
        = std::make_unique<GeometryManager>(m_sceneGraph, m_renderer);
    m_transformManager
        = std::make_unique<TransformManager>(m_sceneGraph, m_renderer);
    m_cameraController
        = std::make_unique<CameraController>(m_camera, m_renderer);

    if (m_rasterRenderer) {
        m_textureManager = std::make_unique<TextureManager>(
            m_sceneGraph, *m_transformManager, *m_rasterRenderer);
    }

    m_image = std::make_unique<Image>(m_renderer, m_sceneGraph, m_camera);

    // Set callback for when Image creates a new object (drag-drop or sampled
    // image)
    m_image->setOnImageObjectCreatedCallback([this](
                                                 SceneGraph::Node *newNode) {
        if (newNode) {
            // Select the newly created object
            m_transformManager->clearSelection();
            m_transformManager->selectNode(newNode);

            // Update renderer transform so gizmo centers on the image
            auto &obj = newNode->getData();
            m_renderer->updateTransform(obj.rendererId, obj.getModelMatrix());
            m_cameraController->resetAllCameraPoses();
        }
    });

    m_renderer->setCameraOverlayCallback(
        [this](int id, const Camera &camera, ImVec2 imagePos, ImVec2 imageSize,
            bool isHovered) {
            m_transformManager->renderCameraGizmo(
                id, camera, imagePos, imageSize, isHovered);
        });

    m_renderer->setBoundingBoxDrawCallback(
        [this]() { m_transformManager->drawBoundingBoxes(); });
}

App::~App() {}

void App::init()
{
    m_sceneGraph.setRoot(std::make_unique<SceneGraph::Node>());
    m_sceneGraph.getRoot()->setData(GameObject());
    m_sceneGraph.getRoot()->getData().rendererId = -1; // No renderer
    m_sceneGraph.getRoot()->getData().setName("Scene Root");

    GData lightGeometry = GeometryGenerator::generateSphere(0.5f, 16, 16);
    std::unique_ptr<SceneGraph::Node> lightNode
        = std::make_unique<SceneGraph::Node>();
    lightNode->setData(GameObject());

    auto light = std::make_unique<Light>(
            lightGeometry.vertices, std::vector<unsigned int> {});
    // light->setPoint({1,1,1},1.0, 0.09, 0.032);
    lightNode->getData().setName(light->getNameStr());

    lightNode->getData().rendererId = m_renderer->registerObject(
        std::move(light),
        "../assets/wish-you-where-here.jpg");
    lightNode->getData().setAABB(
        lightGeometry.aabbCorner1, lightGeometry.aabbCorner2);
    lightNode->getData().setPosition(glm::vec3(3.0f, 3.0f, 3.0f));
    lightNode->getData().setScale(glm::vec3(0.2f));

    m_sceneGraph.getRoot()->addChild(std::move(lightNode));


     std::unique_ptr<SceneGraph::Node> lightNode2
        = std::make_unique<SceneGraph::Node>();
    lightNode2->setData(GameObject());

    auto light2 = std::make_unique<Light>(
            lightGeometry.vertices, std::vector<unsigned int> {});
    light2->setPoint({1,0,0},1.0, 0.09, 0.032);
    lightNode2->getData().setName(light2->getNameStr());

    lightNode2->getData().rendererId = m_renderer->registerObject(
        std::move(light2),
        "../assets/wish-you-where-here.jpg");
    lightNode2->getData().setAABB(
        lightGeometry.aabbCorner1, lightGeometry.aabbCorner2);
    lightNode2->getData().setPosition(glm::vec3(3.0f, 3.0f, 3.0f));
    lightNode2->getData().setScale(glm::vec3(0.2f));

    m_sceneGraph.getRoot()->addChild(std::move(lightNode2));

    std::unique_ptr<SceneGraph::Node> lightNode3
        = std::make_unique<SceneGraph::Node>();
    lightNode3->setData(GameObject());

    auto light3 = std::make_unique<Light>(
            lightGeometry.vertices, std::vector<unsigned int> {});
    light3->setSpot(
        glm::vec3(0.0f, 1.0f, 0.0f),
        1.0f,
        0.09f,
        0.032f,
        10.0f
    );
    lightNode3->getData().setName(light3->getNameStr());

    lightNode3->getData().rendererId = m_renderer->registerObject(
        std::move(light3),
        "../assets/wish-you-where-here.jpg");
    lightNode3->getData().setAABB(
        lightGeometry.aabbCorner1, lightGeometry.aabbCorner2);
    lightNode3->getData().setPosition(glm::vec3(3.0f, 3.0f, 3.0f));
    lightNode3->getData().setScale(glm::vec3(0.2f));

    m_sceneGraph.getRoot()->addChild(std::move(lightNode3));

    m_sceneGraph.traverseWithTransform(
        [&](GameObject &obj, const glm::mat4 &worldTransform, int depth) {
            (void)depth;
            if (obj.rendererId >= 0) {
                m_renderer->updateTransform(obj.rendererId, worldTransform);
            }
        });

    // Initialize geometry manager with callback to select and reset camera
    m_geometryManager->initGeometryWindow([this]() {
        // Select the newly created object
        m_transformManager->clearSelection();
        m_transformManager->selectNode(m_sceneGraph.getRoot()->getChild(
            m_sceneGraph.getRoot()->getChildCount() - 1));
        m_cameraController->resetAllCameraPoses();
    });

    // Register camera controller input callbacks
    m_cameraController->registerInputCallbacks();

    // Register left shift key for multi-selection
    m_renderer->addKeyCallback(
        GLFW_KEY_LEFT_SHIFT, GLFW_PRESS, [&]() { leftShiftPressed = true; });
    m_renderer->addKeyCallback(GLFW_KEY_LEFT_SHIFT, GLFW_RELEASE,
        [&]() { leftShiftPressed = false; });

    // Register delete key callback
    m_renderer->addKeyCallback(GLFW_KEY_DELETE, GLFW_PRESS,
        [this]() { m_transformManager->deleteSelectedObjects(); });

    // Register B key to toggle bounding boxes
    m_renderer->addKeyCallback(GLFW_KEY_B, GLFW_PRESS,
        [this]() { m_transformManager->toggleBoundingBoxes(); });

    // File drop import
    m_renderer->addDropCallback([this](const std::vector<std::string> &paths,
                                    double mouseX, double mouseY) {
        for (const auto &p : paths) {
            // addImageObjectAtScreenPos will trigger the callback which
            // handles selection
            m_image->addImageObjectAtScreenPos(p, mouseX, mouseY);
        }
    });

    m_renderer->init();
    // Create two default cameras to showcase multi-view
    int cam1 = m_camera.createCamera();
    m_camera.setFocused(cam1);
    m_camera.setPosition({ 0.0f, 0.0f, 3.0f });
    m_camera.setProjection(45.0f, 1920.0f / 1080.0f, 0.1f, 100.0f);
    m_renderer->createCameraViews(cam1, 640, 360);
}

void App::update()
{
    m_image->updateMessageTimer(0.016f);
    m_cameraController->update();
}

// Move l'objet dans le vecteur
GameObject &App::registerObject(GameObject &obj)
{
    GameObject gameObj = std::move(obj);

    // Create a new scene graph node for this object
    std::unique_ptr<SceneGraph::Node> childNode
        = std::make_unique<SceneGraph::Node>();
    childNode->setData(gameObj);

    // Add to scene graph
    m_sceneGraph.getRoot()->addChild(std::move(childNode));

    // Update selection to the newly added node
    auto *newNode = m_sceneGraph.getRoot()->getChild(
        m_sceneGraph.getRoot()->getChildCount() - 1);
    m_transformManager->clearSelection();
    m_transformManager->selectNode(newNode);

    // Return reference to the GameObject data in the scene graph
    return m_transformManager->getSelectedNodes().back()->getData();
}

void App::render()
{
    m_renderer->beginFrame();

    vectorial_ui.renderUI(this);
    illumination_ui.renderUI(this);

    m_transformManager->renderTransformUI(leftShiftPressed);
    m_image->renderUI();

    glm::vec4 paletteColor;
    if (m_image->consumeSelectedPaletteColor(paletteColor)) {
        vectorial_ui.setCurrentColorRGBA(paletteColor, true, true);
    }

    // Geometry UI
    m_geometryManager->renderUI();

    if (m_textureManager) {
        m_textureManager->renderUI();
    }

    // Camera Manager UI
    m_cameraController->renderCameraManagerUI();

    m_sceneGraph.traverseWithTransform(
        [&](GameObject &obj, const glm::mat4 &worldTransform, int depth) {
            (void)depth;
            if (obj.rendererId >= 0) {
                m_renderer->updateTransform(obj.rendererId, worldTransform);
            }
        });

    m_renderer->renderAllViews(m_camera);

    // Update camera matrices
    m_renderer->endFrame();

    // Handle frame export if active
    m_image->handleFrameExport(m_renderer->getWindow());
}

void App::run()
{
    init();

    while (!m_renderer->shouldWindowClose()) {
        update();
        render();
    }
}

// Map current interaction state to cursor shape (5+ states)
void App::updateCursor()
{
    // Priority 2: ImGuizmo operation (only while actively manipulating)
    if (ImGuizmo::IsUsing()) {
        switch (m_transformManager->getCurrentGizmoOperation()) {
            case GizmoOp::Translate:
                ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
                return;
            case GizmoOp::Rotate:
                ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeNESW);
                return;
            case GizmoOp::Scale:
                ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeNS);
                return;
            default:
                break;
        }
    }

    // Priority 3: Text input editing in ImGui
    if (ImGui::GetIO().WantTextInput) {
        ImGui::SetMouseCursor(ImGuiMouseCursor_TextInput);
        return;
    }

    // Default: generic pointer
    ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
}
