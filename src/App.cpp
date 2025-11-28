#include "App.hpp"
#include "illumination/Illumination.hpp"
#include "renderer/implementation/PathTracingRenderer.hpp"
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
#include "objects/AnalyticalSphere.hpp"
#include "objects/AnalyticalPlane.hpp"
#include "objects/Object3D.hpp"

App::App() : m_window(1920, 1080, "SceneLab")
{
    m_window.initImGui();
    m_renderer = std::make_unique<RasterizationRenderer>(m_window);

    // Initialize managers
    m_geometryManager
        = std::make_unique<GeometryManager>(m_sceneGraph, m_renderer);
    m_transformManager
        = std::make_unique<TransformManager>(m_sceneGraph, m_renderer);
    m_cameraController
        = std::make_unique<CameraController>(m_camera, m_renderer);

    illumination_ui = std::make_unique<Illumination::UIIllumination>(*m_transformManager, m_sceneGraph);
    if (auto *rasterRenderer
        = dynamic_cast<RasterizationRenderer *>(m_renderer.get())) {
        m_textureManager = std::make_unique<TextureManager>(
            m_sceneGraph, *m_transformManager, *rasterRenderer);
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
    lightNode->getData().rendererId = m_renderer->registerObject(
        std::make_unique<Light>(
            lightGeometry.vertices, std::vector<unsigned int> {}),
        "../assets/wish-you-where-here.jpg");
    lightNode->getData().setAABB(
        lightGeometry.aabbCorner1, lightGeometry.aabbCorner2);
    lightNode->getData().setName("Point Light");
    lightNode->getData().setPosition(glm::vec3(3.0f, 3.0f, 3.0f));
    lightNode->getData().setScale(glm::vec3(0.2f));

    m_sceneGraph.getRoot()->addChild(std::move(lightNode));

    // === PATH TRACING DEMO SCENE ===

    // Helper lambda to create spheres easily
    auto createSphere
        = [this](float radius, glm::vec3 color, glm::vec3 pos,
              const std::string &name, float specular = 0.0f,
              float roughness = 1.0f, glm::vec3 specColor = glm::vec3(1.0f),
              glm::vec3 emissive = glm::vec3(0.0f), float ior = 1.0f,
              float refractionChance = 0.0f) {
              auto sphere
                  = std::make_unique<AnalyticalSphere>(radius, 32, 16, color);
              sphere->setPercentSpecular(specular);
              sphere->setRoughness(roughness);
              sphere->setSpecularColor(specColor);
              sphere->setEmissive(emissive);
              sphere->setIndexOfRefraction(ior);
              sphere->setRefractionChance(refractionChance);

              std::unique_ptr<SceneGraph::Node> node
                  = std::make_unique<SceneGraph::Node>();
              node->setData(GameObject());
              node->getData().rendererId
                  = m_renderer->registerObject(std::move(sphere));
              node->getData().setName(name);
              node->getData().setPosition(pos);
              node->getData().setAABB(glm::vec3(-radius), glm::vec3(radius));
              m_sceneGraph.getRoot()->addChild(std::move(node));
          };

    // Ground plane (reflective dark surface)
    {
        auto plane = std::make_unique<AnalyticalPlane>(
            50.0f, 50.0f, glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.15f));
        plane->setPercentSpecular(0.4f);
        plane->setRoughness(0.05f);
        plane->setSpecularColor(glm::vec3(0.8f));

        std::unique_ptr<SceneGraph::Node> node
            = std::make_unique<SceneGraph::Node>();
        node->setData(GameObject());
        node->getData().rendererId
            = m_renderer->registerObject(std::move(plane));
        node->getData().setName("Ground Plane");
        node->getData().setPosition(glm::vec3(0.0f, -1.0f, 0.0f));
        node->getData().setAABB(
            glm::vec3(-25.0f, -0.01f, -25.0f), glm::vec3(25.0f, 0.01f, 25.0f));
        m_sceneGraph.getRoot()->addChild(std::move(node));
    }

    // Back wall (white for GI bounces)
    {
        auto plane = std::make_unique<AnalyticalPlane>(30.0f, 15.0f,
            glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.9f, 0.9f, 0.9f));
        plane->setPercentSpecular(0.0f);
        plane->setRoughness(1.0f);

        std::unique_ptr<SceneGraph::Node> node
            = std::make_unique<SceneGraph::Node>();
        node->setData(GameObject());
        node->getData().rendererId
            = m_renderer->registerObject(std::move(plane));
        node->getData().setName("Back Wall");
        node->getData().setPosition(glm::vec3(0.0f, 3.0f, -8.0f));
        node->getData().setAABB(
            glm::vec3(-15.0f, -7.5f, -0.01f), glm::vec3(15.0f, 7.5f, 0.01f));
        m_sceneGraph.getRoot()->addChild(std::move(node));
    }

    // === CORNELL BOX WALLS FOR GLOBAL ILLUMINATION ===

    // Left wall (RED - will bleed red light onto nearby objects)
    {
        auto plane = std::make_unique<AnalyticalPlane>(15.0f, 10.0f,
            glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.9f, 0.1f, 0.1f));
        plane->setPercentSpecular(0.0f);
        plane->setRoughness(1.0f);

        std::unique_ptr<SceneGraph::Node> node
            = std::make_unique<SceneGraph::Node>();
        node->setData(GameObject());
        node->getData().rendererId
            = m_renderer->registerObject(std::move(plane));
        node->getData().setName("Red Wall (GI)");
        node->getData().setPosition(glm::vec3(-6.0f, 2.0f, -2.0f));
        node->getData().setAABB(
            glm::vec3(-0.01f, -5.0f, -7.5f), glm::vec3(0.01f, 5.0f, 7.5f));
        m_sceneGraph.getRoot()->addChild(std::move(node));
    }

    // Right wall (GREEN - will bleed green light onto nearby objects)
    {
        auto plane = std::make_unique<AnalyticalPlane>(15.0f, 10.0f,
            glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.1f, 0.9f, 0.1f));
        plane->setPercentSpecular(0.0f);
        plane->setRoughness(1.0f);

        std::unique_ptr<SceneGraph::Node> node
            = std::make_unique<SceneGraph::Node>();
        node->setData(GameObject());
        node->getData().rendererId
            = m_renderer->registerObject(std::move(plane));
        node->getData().setName("Green Wall (GI)");
        node->getData().setPosition(glm::vec3(6.0f, 2.0f, -2.0f));
        node->getData().setAABB(
            glm::vec3(-0.01f, -5.0f, -7.5f), glm::vec3(0.01f, 5.0f, 7.5f));
        m_sceneGraph.getRoot()->addChild(std::move(node));
    }

    // Ceiling (white - reflects light downward)
    {
        auto plane = std::make_unique<AnalyticalPlane>(15.0f, 15.0f,
            glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.95f, 0.95f, 0.95f));
        plane->setPercentSpecular(0.0f);
        plane->setRoughness(1.0f);

        std::unique_ptr<SceneGraph::Node> node
            = std::make_unique<SceneGraph::Node>();
        node->setData(GameObject());
        node->getData().rendererId
            = m_renderer->registerObject(std::move(plane));
        node->getData().setName("Ceiling (GI)");
        node->getData().setPosition(glm::vec3(0.0f, 6.0f, -2.0f));
        node->getData().setAABB(
            glm::vec3(-7.5f, -0.01f, -7.5f), glm::vec3(7.5f, 0.01f, 7.5f));
        m_sceneGraph.getRoot()->addChild(std::move(node));
    }

    // === WHITE SPHERES TO SHOW COLOR BLEEDING ===

    // White sphere near red wall (will appear reddish from indirect light)
    createSphere(0.6f, glm::vec3(0.95f, 0.95f, 0.95f),
        glm::vec3(-4.0f, -0.4f, -1.0f), "White Sphere (Red GI)", 0.05f, 0.9f,
        glm::vec3(1.0f));

    // White sphere near green wall (will appear greenish from indirect light)
    createSphere(0.6f, glm::vec3(0.95f, 0.95f, 0.95f),
        glm::vec3(4.0f, -0.4f, -1.0f), "White Sphere (Green GI)", 0.05f, 0.9f,
        glm::vec3(1.0f));

    // White sphere in center (will show mixed color bleeding)
    createSphere(0.5f, glm::vec3(0.98f, 0.98f, 0.98f),
        glm::vec3(0.0f, -0.5f, -3.0f), "White Sphere (Mixed GI)", 0.0f, 1.0f,
        glm::vec3(1.0f));

    // === MAIN LIGHT SOURCES ===

    // Main area light (warm)
    createSphere(0.8f, glm::vec3(1.0f, 0.95f, 0.9f),
        glm::vec3(0.0f, 4.0f, 0.0f), "Main Light", 0.0f, 1.0f, glm::vec3(1.0f),
        glm::vec3(100.0f, 100.0f, 100.0f));

    // Accent light (cool blue)
    createSphere(0.3f, glm::vec3(0.7f, 0.85f, 1.0f),
        glm::vec3(-4.0f, 2.5f, 2.0f), "Blue Accent Light", 0.0f, 1.0f,
        glm::vec3(1.0f), glm::vec3(3.0f, 4.0f, 8.0f));

    // Accent light (warm orange)
    createSphere(0.25f, glm::vec3(1.0f, 0.6f, 0.3f),
        glm::vec3(4.0f, 2.0f, 1.0f), "Orange Accent Light", 0.0f, 1.0f,
        glm::vec3(1.0f), glm::vec3(8.0f, 4.0f, 1.0f));

    // === CENTER PIECE - Large mirror sphere ===
    createSphere(1.0f, glm::vec3(0.95f), glm::vec3(0.0f, 0.0f, 0.0f),
        "Giant Mirror Sphere", 1.0f, 0.0f, glm::vec3(0.98f));

    // === METALLIC SPHERES ===

    // Gold sphere
    createSphere(0.5f, glm::vec3(1.0f, 0.84f, 0.0f),
        glm::vec3(-2.0f, -0.5f, 1.0f), "Gold Sphere", 0.95f, 0.15f,
        glm::vec3(1.0f, 0.84f, 0.0f));

    // Copper sphere
    createSphere(0.45f, glm::vec3(0.95f, 0.64f, 0.54f),
        glm::vec3(2.2f, -0.55f, 0.8f), "Copper Sphere", 0.9f, 0.25f,
        glm::vec3(0.95f, 0.64f, 0.54f));

    // Chrome sphere
    createSphere(0.35f, glm::vec3(0.9f), glm::vec3(-1.0f, -0.65f, 2.0f),
        "Chrome Sphere", 1.0f, 0.02f, glm::vec3(0.95f));

    // Brushed steel
    createSphere(0.4f, glm::vec3(0.7f, 0.7f, 0.75f),
        glm::vec3(1.5f, -0.6f, 2.2f), "Brushed Steel", 0.8f, 0.4f,
        glm::vec3(0.8f));

    // === COLORED DIFFUSE SPHERES ===

    // Deep red
    createSphere(0.55f, glm::vec3(0.85f, 0.05f, 0.05f),
        glm::vec3(-3.0f, -0.45f, -0.5f), "Red Sphere", 0.1f, 0.8f,
        glm::vec3(1.0f));

    // Royal blue
    createSphere(0.5f, glm::vec3(0.1f, 0.2f, 0.9f),
        glm::vec3(3.0f, -0.5f, -0.3f), "Blue Sphere", 0.15f, 0.7f,
        glm::vec3(1.0f));

    // Emerald green
    createSphere(0.45f, glm::vec3(0.05f, 0.75f, 0.3f),
        glm::vec3(0.0f, -0.55f, 2.5f), "Green Sphere", 0.2f, 0.6f,
        glm::vec3(1.0f));

    // Purple
    createSphere(0.4f, glm::vec3(0.6f, 0.1f, 0.8f),
        glm::vec3(-1.5f, -0.6f, -1.5f), "Purple Sphere", 0.1f, 0.9f,
        glm::vec3(1.0f));

    // === GLASS SPHERES WITH REFRACTION ===

    // Clear glass sphere (IOR 1.5 = standard glass)
    createSphere(0.7f, glm::vec3(0.98f, 0.98f, 1.0f),
        glm::vec3(1.8f, -0.3f, 1.5f), "Clear Glass Sphere", 0.1f, 0.02f,
        glm::vec3(1.0f), glm::vec3(0.0f), 1.5f, 0.95f);

    // Tinted blue glass (like colored glass)
    createSphere(0.55f, glm::vec3(0.7f, 0.85f, 1.0f),
        glm::vec3(-1.8f, -0.45f, 1.8f), "Blue Glass Sphere", 0.1f, 0.02f,
        glm::vec3(0.8f, 0.9f, 1.0f), glm::vec3(0.0f), 1.52f, 0.9f);

    // Diamond-like sphere (high IOR = 2.4)
    createSphere(0.4f, glm::vec3(1.0f, 1.0f, 1.0f),
        glm::vec3(0.0f, -0.6f, -2.0f), "Diamond Sphere", 0.15f, 0.0f,
        glm::vec3(1.0f), glm::vec3(0.0f), 2.4f, 0.85f);

    // Green tinted glass (like a bottle)
    createSphere(0.45f, glm::vec3(0.6f, 0.95f, 0.7f),
        glm::vec3(2.5f, -0.55f, -1.0f), "Green Glass Sphere", 0.08f, 0.03f,
        glm::vec3(0.7f, 1.0f, 0.8f), glm::vec3(0.0f), 1.5f, 0.92f);

    // Water-like sphere (IOR 1.33)
    createSphere(0.5f, glm::vec3(0.85f, 0.95f, 1.0f),
        glm::vec3(-2.5f, -0.5f, -1.2f), "Water Sphere", 0.05f, 0.01f,
        glm::vec3(0.9f, 0.95f, 1.0f), glm::vec3(0.0f), 1.33f, 0.88f);

    // Pink pearl (not refractive, just reflective)
    createSphere(0.35f, glm::vec3(1.0f, 0.75f, 0.8f),
        glm::vec3(-0.5f, -0.65f, 3.0f), "Pink Pearl", 0.5f, 0.3f,
        glm::vec3(1.0f, 0.9f, 0.95f));

    // === SMALL DETAIL SPHERES ===

    // Cluster of small spheres
    createSphere(0.15f, glm::vec3(1.0f, 1.0f, 0.0f),
        glm::vec3(-2.5f, -0.85f, 2.5f), "Yellow Mini", 0.3f, 0.5f,
        glm::vec3(1.0f));

    createSphere(0.12f, glm::vec3(1.0f, 0.5f, 0.0f),
        glm::vec3(-2.2f, -0.88f, 2.7f), "Orange Mini", 0.3f, 0.5f,
        glm::vec3(1.0f));

    createSphere(0.1f, glm::vec3(1.0f, 0.0f, 0.5f),
        glm::vec3(-2.7f, -0.9f, 2.3f), "Pink Mini", 0.3f, 0.5f,
        glm::vec3(1.0f));

    // Small emissive accent spheres
    createSphere(0.08f, glm::vec3(0.0f, 1.0f, 0.5f),
        glm::vec3(2.5f, -0.92f, 2.8f), "Green Glow", 0.0f, 1.0f,
        glm::vec3(1.0f), glm::vec3(2.0f, 5.0f, 3.0f));

    createSphere(0.08f, glm::vec3(1.0f, 0.0f, 0.3f),
        glm::vec3(2.8f, -0.92f, 2.5f), "Magenta Glow", 0.0f, 1.0f,
        glm::vec3(1.0f), glm::vec3(5.0f, 1.0f, 3.0f));

    // === END DEMO SCENE ===

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

    // Register change renderer callback on the R key
    m_renderer->addKeyCallback(
        GLFW_KEY_R, GLFW_PRESS, [this]() { switchRenderer(); });

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

void App::switchRenderer()
{
    // Extract all objects from the old renderer before destroying it
    auto objects = m_renderer->extractAllObjects();

    // Create new renderer
    if (dynamic_cast<RasterizationRenderer *>(m_renderer.get())) {
        m_renderer = std::make_unique<PathTracingRenderer>(m_window);
    } else {
        m_renderer = std::make_unique<RasterizationRenderer>(m_window);
    }

    // Re-register all objects with the new renderer
    m_sceneGraph.traverse([&](GameObject &obj, int depth) {
        (void)depth;
        if (obj.rendererId >= 0
            && obj.rendererId < static_cast<int>(objects.size())) {
            auto &renderObj = objects[obj.rendererId];
            if (renderObj) {
                obj.rendererId
                    = m_renderer->registerObject(std::move(renderObj));
            } else {
                obj.rendererId = -1;
            }
        }
    });

    // Update transforms for all re-registered objects
    m_sceneGraph.traverseWithTransform(
        [&](GameObject &obj, const glm::mat4 &worldTransform, int depth) {
            (void)depth;
            if (obj.rendererId >= 0) {
                m_renderer->updateTransform(obj.rendererId, worldTransform);
            }
        });

    // Recreate TextureManager with the new renderer
    if (dynamic_cast<RasterizationRenderer *>(m_renderer.get())) {
        m_textureManager = std::make_unique<TextureManager>(m_sceneGraph,
            *m_transformManager,
            dynamic_cast<RasterizationRenderer &>(*m_renderer));
    } else {
        m_textureManager.reset();
    }

    // Re-register renderer callbacks
    m_renderer->setCameraOverlayCallback(
        [this](int id, const Camera &camera, ImVec2 imagePos, ImVec2 imageSize,
            bool isHovered) {
            m_transformManager->renderCameraGizmo(
                id, camera, imagePos, imageSize, isHovered);
        });

    m_renderer->setBoundingBoxDrawCallback(
        [this]() { m_transformManager->drawBoundingBoxes(); });

    // Recreate camera views for all existing cameras
    for (int camId : m_camera.getCameraIds()) {
        m_renderer->createCameraViews(camId, 640, 360);
    }
}

void App::resetScene()
{
    m_transformManager->clearSelection();

    // Remove all children from root
    auto *root = m_sceneGraph.getRoot();
    while (root->getChildCount() > 0) {
        auto *child = root->getChild(0);
        if (child->getData().rendererId >= 0) {
            m_renderer->removeObject(child->getData().rendererId);
        }
        root->removeChild(child);
    }

    m_cameraController->resetAllCameraPoses();
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

void App::renderMainMenuBar()
{
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("New Scene", "Ctrl+N")) {
                resetScene();
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Quit", "Alt+F4")) {
                glfwSetWindowShouldClose(m_renderer->getWindow(), GLFW_TRUE);
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Edit")) {
            ImGui::Separator();
            if (ImGui::MenuItem("Delete", "Del")) {
                m_transformManager->deleteSelectedObjects();
            }
            if (ImGui::MenuItem("Select All", "Ctrl+A")) {
                m_transformManager->selectAll();
            }
            if (ImGui::MenuItem("Deselect All", "Escape")) {
                m_transformManager->clearSelection();
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("View")) {
            bool isPathTracing
                = dynamic_cast<PathTracingRenderer *>(m_renderer.get())
                != nullptr;
            if (ImGui::MenuItem(isPathTracing ? "Switch to Rasterization"
                                              : "Switch to Path Tracing",
                    "R")) {
                switchRenderer();
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Toggle Bounding Boxes", "B")) {
                m_transformManager->toggleBoundingBoxes();
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Reset Camera")) {
                m_cameraController->resetAllCameraPoses();
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Window")) {
            ImGui::MenuItem("Hierarchy", nullptr, &m_showHierarchyWindow);
            ImGui::MenuItem("Transform", nullptr, &m_showTransformWindow);
            ImGui::MenuItem("Ray Tracing", nullptr, &m_showRayTracingWindow);
            ImGui::Separator();
            ImGui::MenuItem("Geometry", nullptr, &m_showGeometryWindow);
            ImGui::MenuItem("Texture", nullptr, &m_showTextureWindow);
            ImGui::MenuItem("Camera", nullptr, &m_showCameraWindow);
            ImGui::MenuItem("Image", nullptr, &m_showImageWindow);
            ImGui::MenuItem("Vector Drawing", nullptr, &m_showVectorWindow);
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Help")) {
            if (ImGui::MenuItem("About SceneLab")) {
                m_showAboutPopup = true;
            }
            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }

    // About popup
    if (m_showAboutPopup) {
        ImGui::OpenPopup("About SceneLab");
        m_showAboutPopup = false;
    }

    if (ImGui::BeginPopupModal(
            "About SceneLab", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("SceneLab");
        ImGui::Separator();
        ImGui::Text("A 3D/2D scene editor with OpenGL");
        ImGui::Text("Supports rasterization and path tracing rendering");
        ImGui::Spacing();
        ImGui::Text("Controls:");
        ImGui::BulletText("R - Switch renderer");
        ImGui::BulletText("B - Toggle bounding boxes");
        ImGui::BulletText("W/E/R - Translate/Rotate/Scale gizmo");
        ImGui::BulletText("Delete - Delete selected object");
        ImGui::BulletText("Shift+Click - Multi-select");
        ImGui::Spacing();
        if (ImGui::Button("Close", ImVec2(120, 0))) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}

void App::render()
{
    m_renderer->beginFrame();

    // Main menu bar
    renderMainMenuBar();

    // Illumination UI
    illumination_ui->renderUI(this);

    // Vector drawing UI
    vectorial_ui.renderUI(this, &m_showVectorWindow);

    // Transform and hierarchy UI
    m_transformManager->renderTransformUI(
        leftShiftPressed, &m_showTransformWindow, &m_showHierarchyWindow);
    m_transformManager->renderRayTracingUI(&m_showRayTracingWindow);

    // Image UI
    m_image->renderUI(&m_showImageWindow);

    glm::vec4 paletteColor;
    if (m_image->consumeSelectedPaletteColor(paletteColor)) {
        vectorial_ui.setCurrentColorRGBA(paletteColor, true, true);
    }

    // Geometry UI
    m_geometryManager->renderUI(&m_showGeometryWindow);

    // Texture UI
    if (m_textureManager) {
        m_textureManager->renderUI(&m_showTextureWindow);
    } else if (m_showTextureWindow) {
        if (auto *ptRenderer
            = dynamic_cast<PathTracingRenderer *>(m_renderer.get())) {
            // Simplified Texture window for path tracing mode
            if (ImGui::Begin("Texture", &m_showTextureWindow)) {
                ImGui::TextDisabled(
                    "Texture mapping not available in path tracing");

                ImGui::SeparatorText("Tone Mapping");
                int toneIndex
                    = static_cast<int>(ptRenderer->getToneMappingMode());
                if (ImGui::Combo("Operator", &toneIndex,
                        PathTracingRenderer::TONEMAP_LABELS.data(),
                        static_cast<int>(
                            PathTracingRenderer::TONEMAP_LABELS.size()))) {
                    ptRenderer->setToneMappingMode(
                        static_cast<ToneMappingMode>(toneIndex));
                }
                float exposure = ptRenderer->getToneMappingExposure();
                if (ImGui::SliderFloat(
                        "Exposure", &exposure, 0.1f, 5.0f, "%.2f")) {
                    ptRenderer->setToneMappingExposure(exposure);
                }

                ImGui::SeparatorText("Cubemap / Environment");
                const auto &handles = ptRenderer->getCubemapHandles();
                if (handles.empty()) {
                    ImGui::TextDisabled("No cubemaps available");
                } else {
                    std::vector<const char *> names;
                    int activeIdx = 0;
                    int activeHandle = ptRenderer->getActiveCubemap();
                    for (size_t i = 0; i < handles.size(); ++i) {
                        if (auto *res
                            = ptRenderer->getTextureResource(handles[i])) {
                            names.push_back(res->name.c_str());
                            if (handles[i] == activeHandle) {
                                activeIdx = static_cast<int>(i);
                            }
                        }
                    }
                    if (!names.empty()) {
                        if (ImGui::Combo("Skybox", &activeIdx, names.data(),
                                static_cast<int>(names.size()))) {
                            ptRenderer->setActiveCubemap(handles[activeIdx]);
                        }
                    }
                }
            }
            ImGui::End();
        }
    }

    // Camera Manager UI
    m_cameraController->renderCameraManagerUI(&m_showCameraWindow);

    m_sceneGraph.traverseWithTransform(
        [&](GameObject &obj, const glm::mat4 &worldTransform, int depth) {
            (void)depth;
            if (obj.hasTransformChanged()) {
                m_renderer->updateTransform(obj.rendererId, worldTransform);
                obj.setHasMoved(false);
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
