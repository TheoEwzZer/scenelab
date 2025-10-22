#include "App.hpp"
#include "OBJLoader.hpp"
#include "GameObject.hpp"
#include "SceneGraph.hpp"
#include "renderer/implementation/RasterizationRenderer.hpp"
#include "GeometryGenerator.hpp"

#include "Camera.hpp"
#include "GLFW/glfw3.h"

#include "imgui.h"
#include "ImGuizmo.h"
#include <glm/geometric.hpp>
#include <glm/matrix.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <memory>
#include <string>
#include <utility>
#include <vector>
#include <format>
#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdio>
#include <limits>

App::App()
{
    m_renderer = std::make_unique<RasterizationRenderer>();

    m_image = std::make_unique<Image>(m_renderer, m_sceneGraph, m_camera);

    m_renderer->setCameraOverlayCallback([this](int id, const Camera &camera,
                                             ImVec2 imagePos, ImVec2 imageSize,
                                             bool isHovered) {
        this->renderCameraGizmo(id, camera, imagePos, imageSize, isHovered);
    });

    m_renderer->setBoundingBoxDrawCallback(
        [this]() { this->drawBoundingBoxes(); });
}

App::~App() {}

void App::initGeometryWindow()
{
    m_GeometryImguiWindow.onSpawnCube = [this](float size) {
        GameObject new_obj;
        auto data { GeometryGenerator::generateCube(size) };
        glm::vec3 randomColor { rand() / (float)RAND_MAX,
            rand() / (float)RAND_MAX, rand() / (float)RAND_MAX };

        new_obj.rendererId = m_renderer->registerObject(
            data.vertices, {}, randomColor, false);
        new_obj.setPosition({ 0.0f, 0.0f, 0.0f });
        new_obj.setAABB(data.aabbCorner1, data.aabbCorner2);
        new_obj.setName(
            std::format("Cube {}", m_GeometryImguiWindow.m_cubeCount));
        m_renderer->updateTransform(
            new_obj.rendererId, new_obj.getModelMatrix());

        std::unique_ptr<SceneGraph::Node> childNode
            = std::make_unique<SceneGraph::Node>();
        childNode->setData(new_obj);
        m_sceneGraph.getRoot()->addChild(std::move(childNode));
        m_selectedNodes.clear();
        m_selectedNodes.push_back(m_sceneGraph.getRoot()->getChild(
            m_sceneGraph.getRoot()->getChildCount() - 1));

        std::cout << std::format("[INFO] Spawned cube\n");
        resetAllCameraPoses();
    };

    m_GeometryImguiWindow.onSpawnSphere = [this](float radius, int sectors,
                                              int stacks) {
        GameObject new_obj;
        auto data { GeometryGenerator::generateSphere(
            radius, sectors, stacks) };
        glm::vec3 randomColor { rand() / (float)RAND_MAX,
            rand() / (float)RAND_MAX, rand() / (float)RAND_MAX };

        new_obj.rendererId = m_renderer->registerObject(
            data.vertices, {}, randomColor, false);
        new_obj.setPosition({ 0.0f, 0.0f, 0.0f });
        new_obj.setAABB(data.aabbCorner1, data.aabbCorner2);
        new_obj.setName(
            std::format("Sphere {}", m_GeometryImguiWindow.m_sphereCount));
        m_renderer->updateTransform(
            new_obj.rendererId, new_obj.getModelMatrix());

        std::unique_ptr<SceneGraph::Node> childNode
            = std::make_unique<SceneGraph::Node>();
        childNode->setData(new_obj);
        m_sceneGraph.getRoot()->addChild(std::move(childNode));
        m_selectedNodes.clear();
        m_selectedNodes.push_back(m_sceneGraph.getRoot()->getChild(
            m_sceneGraph.getRoot()->getChildCount() - 1));

        std::cout << std::format("[INFO] Spawned sphere\n");
        resetAllCameraPoses();
    };

    m_GeometryImguiWindow.onSpawnCylinder = [this](float radius, float height,
                                                int sectors) {
        GameObject new_obj;
        auto data { GeometryGenerator::generateCylinder(
            radius, height, sectors) };
        glm::vec3 randomColor { rand() / (float)RAND_MAX,
            rand() / (float)RAND_MAX, rand() / (float)RAND_MAX };

        new_obj.rendererId = m_renderer->registerObject(
            data.vertices, {}, randomColor, false);
        new_obj.setPosition({ 0.0f, 0.0f, 0.0f });
        new_obj.setAABB(data.aabbCorner1, data.aabbCorner2);
        new_obj.setName(
            std::format("Cylinder {}", m_GeometryImguiWindow.m_cylinderCount));
        m_renderer->updateTransform(
            new_obj.rendererId, new_obj.getModelMatrix());

        std::unique_ptr<SceneGraph::Node> childNode
            = std::make_unique<SceneGraph::Node>();
        childNode->setData(new_obj);
        m_sceneGraph.getRoot()->addChild(std::move(childNode));
        m_selectedNodes.clear();
        m_selectedNodes.push_back(m_sceneGraph.getRoot()->getChild(
            m_sceneGraph.getRoot()->getChildCount() - 1));

        std::cout << std::format("[INFO] Spawned cylinder\n");
        resetAllCameraPoses();
    };

    // not loaded as an object here yet
    m_GeometryImguiWindow.onLoadModel = [this](const std::string &objName,
                                            const std::string &objPath) {
        auto data { OBJLoader::loadOBJ(objName, objPath) };

        m_GeometryImguiWindow.m_modelLibrary.addModel(objName, objPath, data);

        std::cout << std::format(
            "[INFO] Loaded model {} into library\n", objName);
    };

    m_GeometryImguiWindow.onSpawnModelInstance =
        [this](const std::string &name, const std::string &filepath) {
            auto &modelLib = m_GeometryImguiWindow.m_modelLibrary;

            auto maybeGData = modelLib.getModelData(filepath);

            if (!maybeGData.has_value()) {
                std::cerr << std::format(
                    "[ERROR] Model not found in library: {}\n", name);
                return;
            }

            const GData &data = maybeGData.value();
            GameObject new_obj;
            glm::vec3 randomColor { rand() / (float)RAND_MAX,
                rand() / (float)RAND_MAX, rand() / (float)RAND_MAX };

            new_obj.rendererId = m_renderer->registerObject(
                data.vertices, {}, randomColor, false);
            new_obj.setPosition({ 0.0f, 0.0f, 0.0f });
            new_obj.setAABB(data.aabbCorner1, data.aabbCorner2);

            const auto &models = modelLib.getModels();
            auto it = models.find(filepath);
            if (it != models.end()) {
                modelLib.incrementInstanceCount(filepath);
                std::size_t instanceNum = modelLib.getInstanceCount(filepath);
                new_obj.setName(
                    std::format("{} {}", it->second.name, instanceNum));
            }

            m_renderer->updateTransform(
                new_obj.rendererId, new_obj.getModelMatrix());

            std::unique_ptr<SceneGraph::Node> childNode
                = std::make_unique<SceneGraph::Node>();
            childNode->setData(new_obj);
            m_sceneGraph.getRoot()->addChild(std::move(childNode));
            m_selectedNodes.clear();
            m_selectedNodes.push_back(m_sceneGraph.getRoot()->getChild(
                m_sceneGraph.getRoot()->getChildCount() - 1));

            std::cout << std::format(
                "[INFO] Spawned instance of {}\n", filepath);
            resetAllCameraPoses();
        };
}

void App::init()
{
    std::vector<float> vertices = {
        -0.5f,
        -0.5f,
        -0.5f,
        0.0f,
        0.0f,
        0.5f,
        -0.5f,
        -0.5f,
        1.0f,
        0.0f,
        0.5f,
        0.5f,
        -0.5f,
        1.0f,
        1.0f,
        0.5f,
        0.5f,
        -0.5f,
        1.0f,
        1.0f,
        -0.5f,
        0.5f,
        -0.5f,
        0.0f,
        1.0f,
        -0.5f,
        -0.5f,
        -0.5f,
        0.0f,
        0.0f,
        -0.5f,
        -0.5f,
        0.5f,
        0.0f,
        0.0f,
        0.5f,
        -0.5f,
        0.5f,
        1.0f,
        0.0f,
        0.5f,
        0.5f,
        0.5f,
        1.0f,
        1.0f,
        0.5f,
        0.5f,
        0.5f,
        1.0f,
        1.0f,
        -0.5f,
        0.5f,
        0.5f,
        0.0f,
        1.0f,
        -0.5f,
        -0.5f,
        0.5f,
        0.0f,
        0.0f,
        -0.5f,
        0.5f,
        0.5f,
        1.0f,
        0.0f,
        -0.5f,
        0.5f,
        -0.5f,
        1.0f,
        1.0f,
        -0.5f,
        -0.5f,
        -0.5f,
        0.0f,
        1.0f,
        -0.5f,
        -0.5f,
        -0.5f,
        0.0f,
        1.0f,
        -0.5f,
        -0.5f,
        0.5f,
        0.0f,
        0.0f,
        -0.5f,
        0.5f,
        0.5f,
        1.0f,
        0.0f,
        0.5f,
        0.5f,
        0.5f,
        1.0f,
        0.0f,
        0.5f,
        0.5f,
        -0.5f,
        1.0f,
        1.0f,
        0.5f,
        -0.5f,
        -0.5f,
        0.0f,
        1.0f,
        0.5f,
        -0.5f,
        -0.5f,
        0.0f,
        1.0f,
        0.5f,
        -0.5f,
        0.5f,
        0.0f,
        0.0f,
        0.5f,
        0.5f,
        0.5f,
        1.0f,
        0.0f,
        -0.5f,
        -0.5f,
        -0.5f,
        0.0f,
        1.0f,
        0.5f,
        -0.5f,
        -0.5f,
        1.0f,
        1.0f,
        0.5f,
        -0.5f,
        0.5f,
        1.0f,
        0.0f,
        0.5f,
        -0.5f,
        0.5f,
        1.0f,
        0.0f,
        -0.5f,
        -0.5f,
        0.5f,
        0.0f,
        0.0f,
        -0.5f,
        -0.5f,
        -0.5f,
        0.0f,
        1.0f,
        -0.5f,
        0.5f,
        -0.5f,
        0.0f,
        1.0f,
        0.5f,
        0.5f,
        -0.5f,
        1.0f,
        1.0f,
        0.5f,
        0.5f,
        0.5f,
        1.0f,
        0.0f,
        0.5f,
        0.5f,
        0.5f,
        1.0f,
        0.0f,
        -0.5f,
        0.5f,
        0.5f,
        0.0f,
        0.0f,
        -0.5f,
        0.5f,
        -0.5f,
        0.0f,
        1.0f,
    };

    std::vector<float> verticesAndNormal = {
        -0.5f,
        -0.5f,
        -0.5f,
        0.0f,
        0.0f,
        0.0f,
        0.0f,
        -1.0f,
        0.5f,
        -0.5f,
        -0.5f,
        1.0f,
        0.0f,
        0.0f,
        0.0f,
        -1.0f,
        0.5f,
        0.5f,
        -0.5f,
        1.0f,
        1.0f,
        0.0f,
        0.0f,
        -1.0f,
        0.5f,
        0.5f,
        -0.5f,
        1.0f,
        1.0f,
        0.0f,
        0.0f,
        -1.0f,
        -0.5f,
        0.5f,
        -0.5f,
        0.0f,
        1.0f,
        0.0f,
        0.0f,
        -1.0f,
        -0.5f,
        -0.5f,
        -0.5f,
        0.0f,
        0.0f,
        0.0f,
        0.0f,
        -1.0f,
        -0.5f,
        -0.5f,
        0.5f,
        0.0f,
        0.0f,
        0.0f,
        0.0f,
        1.0f,
        0.5f,
        -0.5f,
        0.5f,
        1.0f,
        0.0f,
        0.0f,
        0.0f,
        1.0f,
        0.5f,
        0.5f,
        0.5f,
        1.0f,
        1.0f,
        0.0f,
        0.0f,
        1.0f,
        0.5f,
        0.5f,
        0.5f,
        1.0f,
        1.0f,
        0.0f,
        0.0f,
        1.0f,
        -0.5f,
        0.5f,
        0.5f,
        0.0f,
        1.0f,
        0.0f,
        0.0f,
        1.0f,
        -0.5f,
        -0.5f,
        0.5f,
        0.0f,
        0.0f,
        0.0f,
        0.0f,
        1.0f,
        -0.5f,
        0.5f,
        0.5f,
        1.0f,
        0.0f,
        -1.0f,
        0.0f,
        0.0f,
        -0.5f,
        0.5f,
        -0.5f,
        1.0f,
        1.0f,
        -1.0f,
        0.0f,
        0.0f,
        -0.5f,
        -0.5f,
        -0.5f,
        0.0f,
        1.0f,
        -1.0f,
        0.0f,
        0.0f,
        -0.5f,
        -0.5f,
        -0.5f,
        0.0f,
        1.0f,
        -1.0f,
        0.0f,
        0.0f,
        -0.5f,
        -0.5f,
        0.5f,
        0.0f,
        0.0f,
        -1.0f,
        0.0f,
        0.0f,
        -0.5f,
        0.5f,
        0.5f,
        1.0f,
        0.0f,
        -1.0f,
        0.0f,
        0.0f,
        0.5f,
        0.5f,
        0.5f,
        1.0f,
        0.0f,
        1.0f,
        0.0f,
        0.0f,
        0.5f,
        0.5f,
        -0.5f,
        1.0f,
        1.0f,
        1.0f,
        0.0f,
        0.0f,
        0.5f,
        -0.5f,
        -0.5f,
        0.0f,
        1.0f,
        1.0f,
        0.0f,
        0.0f,
        0.5f,
        -0.5f,
        -0.5f,
        0.0f,
        1.0f,
        1.0f,
        0.0f,
        0.0f,
        0.5f,
        -0.5f,
        0.5f,
        0.0f,
        0.0f,
        1.0f,
        0.0f,
        0.0f,
        0.5f,
        0.5f,
        0.5f,
        1.0f,
        0.0f,
        1.0f,
        0.0f,
        0.0f,
        -0.5f,
        -0.5f,
        -0.5f,
        0.0f,
        1.0f,
        0.0f,
        -1.0f,
        0.0f,
        0.5f,
        -0.5f,
        -0.5f,
        1.0f,
        1.0f,
        0.0f,
        -1.0f,
        0.0f,
        0.5f,
        -0.5f,
        0.5f,
        1.0f,
        0.0f,
        0.0f,
        -1.0f,
        0.0f,
        0.5f,
        -0.5f,
        0.5f,
        1.0f,
        0.0f,
        0.0f,
        -1.0f,
        0.0f,
        -0.5f,
        -0.5f,
        0.5f,
        0.0f,
        0.0f,
        0.0f,
        -1.0f,
        0.0f,
        -0.5f,
        -0.5f,
        -0.5f,
        0.0f,
        1.0f,
        0.0f,
        -1.0f,
        0.0f,
        -0.5f,
        0.5f,
        -0.5f,
        0.0f,
        1.0f,
        0.0f,
        1.0f,
        0.0f,
        0.5f,
        0.5f,
        -0.5f,
        1.0f,
        1.0f,
        0.0f,
        1.0f,
        0.0f,
        0.5f,
        0.5f,
        0.5f,
        1.0f,
        0.0f,
        0.0f,
        1.0f,
        0.0f,
        0.5f,
        0.5f,
        0.5f,
        1.0f,
        0.0f,
        0.0f,
        1.0f,
        0.0f,
        -0.5f,
        0.5f,
        0.5f,
        0.0f,
        0.0f,
        0.0f,
        1.0f,
        0.0f,
        -0.5f,
        0.5f,
        -0.5f,
        0.0f,
        1.0f,
        0.0f,
        1.0f,
        0.0f,
    };

    m_sceneGraph.setRoot(std::make_unique<SceneGraph::Node>());
    m_sceneGraph.getRoot()->setData(GameObject());
    m_sceneGraph.getRoot()->getData().rendererId = m_renderer->registerObject(
        verticesAndNormal, {}, "../assets/wish-you-where-here.jpg", false);
    std::unique_ptr<SceneGraph::Node> childNode
        = std::make_unique<SceneGraph::Node>();
    childNode->setData(GameObject());
    childNode->getData().rendererId = m_renderer->registerObject(
        verticesAndNormal, {}, "../assets/wish-you-where-here.jpg", true);
    m_sceneGraph.getRoot()->addChild(std::move(childNode));
    m_selectedNodes.push_back(m_sceneGraph.getRoot());

    m_sceneGraph.getRoot()->getChild(0)->getData().setPosition(
        { 1.2f, 0.f, 0.0f });
    m_sceneGraph.getRoot()->getChild(0)->getData().setScale(
        glm::vec3 { 0.2f });

    m_sceneGraph.traverseWithTransform(
        [&](GameObject &obj, const glm::mat4 &worldTransform, int depth) {
            (void)depth;
            m_renderer->updateTransform(obj.rendererId, worldTransform);
        });

    this->initGeometryWindow();

    // Register key callbacks
    m_renderer->addKeyCallback(
        GLFW_KEY_W, GLFW_PRESS, [&]() { wPressed = true; });
    m_renderer->addKeyCallback(
        GLFW_KEY_W, GLFW_RELEASE, [&]() { wPressed = false; });
    m_renderer->addKeyCallback(
        GLFW_KEY_S, GLFW_PRESS, [&]() { sPressed = true; });
    m_renderer->addKeyCallback(
        GLFW_KEY_S, GLFW_RELEASE, [&]() { sPressed = false; });
    m_renderer->addKeyCallback(
        GLFW_KEY_A, GLFW_PRESS, [&]() { aPressed = true; });
    m_renderer->addKeyCallback(
        GLFW_KEY_A, GLFW_RELEASE, [&]() { aPressed = false; });
    m_renderer->addKeyCallback(
        GLFW_KEY_D, GLFW_PRESS, [&]() { dPressed = true; });
    m_renderer->addKeyCallback(
        GLFW_KEY_D, GLFW_RELEASE, [&]() { dPressed = false; });
    m_renderer->addKeyCallback(
        GLFW_KEY_SPACE, GLFW_PRESS, [&]() { spacePressed = true; });
    m_renderer->addKeyCallback(
        GLFW_KEY_SPACE, GLFW_RELEASE, [&]() { spacePressed = false; });
    m_renderer->addKeyCallback(
        GLFW_KEY_LEFT_CONTROL, GLFW_PRESS, [&]() { leftCtrlPressed = true; });
    m_renderer->addKeyCallback(GLFW_KEY_LEFT_CONTROL, GLFW_RELEASE,
        [&]() { leftCtrlPressed = false; });
    m_renderer->addKeyCallback(
        GLFW_KEY_LEFT_SHIFT, GLFW_PRESS, [&]() { leftShiftPressed = true; });
    m_renderer->addKeyCallback(GLFW_KEY_LEFT_SHIFT, GLFW_RELEASE,
        [&]() { leftShiftPressed = false; });

    // Register mouse button callbacks
    m_renderer->addKeyCallback(
        GLFW_MOUSE_BUTTON_2, GLFW_PRESS, [&]() { firstMouse = true; });
    m_renderer->addKeyCallback(
        GLFW_MOUSE_BUTTON_2, GLFW_RELEASE, [&]() { firstMouse = false; });

    m_renderer->addKeyCallback(GLFW_MOUSE_BUTTON_1, GLFW_PRESS, [&]() {
        glm::vec4 normalizedMousePos;

        normalizedMousePos.x = (m_currentMousePos.x / 1920.f) * 2.0f - 1.0f;
        normalizedMousePos.y = 1.0f - (m_currentMousePos.y / 1080.f) * 2.0f;
        normalizedMousePos.z = -1.0f;
        normalizedMousePos.w = 1.0f;

        // Ray cast to select objects

        // Go back in the pipeline
        glm::vec4 rayEye = glm::inverse(m_camera.getProjectionMatrix())
            * normalizedMousePos;
        rayEye.z = -1.0f;
        rayEye.w = 0.0f;

        glm::vec3 rayWor = glm::inverse(m_camera.getViewMatrix()) * rayEye;
        rayWor = glm::normalize(rayWor);
    });

    // File drop import
    m_renderer->addDropCallback([&](const std::vector<std::string> &paths,
                                    double mouseX, double mouseY) {
        for (const auto &p : paths) {
            const int beforeCount
                = m_sceneGraph.getRoot()->getChildCount();
            const bool added
                = m_image->addImageObjectAtScreenPos(p, mouseX, mouseY);
            if (added && m_sceneGraph.getRoot()->getChildCount() > 0
                && m_sceneGraph.getRoot()->getChildCount() != beforeCount) {
                m_selectedNodes.clear();
                m_selectedNodes.push_back(m_sceneGraph.getRoot()->getChild(
                    m_sceneGraph.getRoot()->getChildCount() - 1));

                // Ensure renderer transform matches immediately so gizmo
                // centers on the image
                auto &obj
                    = m_sceneGraph.getRoot()
                          ->getChild(
                              m_sceneGraph.getRoot()->getChildCount() - 1)
                          ->getData();
                m_renderer->updateTransform(
                    obj.rendererId, obj.getModelMatrix());
                resetAllCameraPoses();
            }
        }
    });

    // Register mouse movement callback
    m_renderer->addCursorCallback([&](double x, double y) {
        m_currentMousePos = glm::vec2(x, y);
        mouseDelta = m_currentMousePos - prevMousePos;
        prevMousePos = m_currentMousePos;

        // Only rotate if left mouse button is held
        if (glm::length(mouseDelta) > 0.0f && firstMouse) {
            auto rot = m_camera.getRotation();
            rot.y += mouseDelta.x * 0.1f;
            rot.x += mouseDelta.y * 0.1f;
            m_camera.setRotation(rot.x, rot.y, rot.z);
        }
    });

    m_renderer->init();
    // Create two default cameras to showcase multi-view
    int cam1 = m_camera.createCamera();
    m_camera.setFocused(cam1);
    m_camera.setPosition({ 0.0f, 0.0f, 3.0f });
    m_camera.setProjection(45.0f, 1920.0f / 1080.0f, 0.1f, 100.0f);
    m_renderer->createCameraViews(cam1, 640, 360);

    int cam2 = m_camera.createCamera();
    m_camera.setFocused(cam2);
    m_camera.setPosition({ 3.0f, 3.0f, 3.0f });
    m_camera.setProjection(45.0f, 1920.0f / 1080.0f, 0.1f, 100.0f);
    m_camera.setRotation(-20.0f, -45.0f, 0.0f);
    m_renderer->createCameraViews(cam2, 640, 360);
}

void App::update()
{
    m_image->updateMessageTimer(0.016f);
    if (wPressed) {
        auto pos = m_camera.getPosition();
        auto rot = m_camera.getRotation();
        pos.x += 0.05f * sin(glm::radians(rot.y));
        pos.z -= 0.05f * cos(glm::radians(rot.y));
        m_camera.setPosition(pos);
    }
    if (sPressed) {
        auto pos = m_camera.getPosition();
        auto rot = m_camera.getRotation();
        pos.x -= 0.05f * sin(glm::radians(rot.y));
        pos.z += 0.05f * cos(glm::radians(rot.y));
        m_camera.setPosition(pos);
    }
    if (aPressed) {
        auto pos = m_camera.getPosition();
        auto rot = m_camera.getRotation();
        pos.x -= 0.05f * cos(glm::radians(rot.y));
        pos.z -= 0.05f * sin(glm::radians(rot.y));
        m_camera.setPosition(pos);
    }
    if (dPressed) {
        auto pos = m_camera.getPosition();
        auto rot = m_camera.getRotation();
        pos.x += 0.05f * cos(glm::radians(rot.y));
        pos.z += 0.05f * sin(glm::radians(rot.y));
        m_camera.setPosition(pos);
    }
    if (spacePressed) {
        auto pos = m_camera.getPosition();
        pos.y += 0.05f;
        m_camera.setPosition(pos);
    }
    if (leftCtrlPressed) {
        auto pos = m_camera.getPosition();
        pos.y -= 0.05f;
        m_camera.setPosition(pos);
    }

    // ImGuizmo manipulation moved to render() function
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
    m_selectedNodes.clear();
    m_selectedNodes.push_back(m_sceneGraph.getRoot()->getChild(
        m_sceneGraph.getRoot()->getChildCount() - 1));

    // Return reference to the GameObject data in the scene graph
    return m_selectedNodes.back()->getData();
}

void App::selectedTransformUI()
{
    // Render scene graph hierarchy with selection
    m_sceneGraph.renderHierarchyUI(
        m_selectedNodes, leftShiftPressed, [this](SceneGraph::Node *node) {
            return this->canAddToSelection(node);
        });

    // Only early-exit if there are no objects at all. We still want the
    // Transforms window to show when nodes are selected (multi-selection
    // path), even if selectedObjectIndex is -1.
    if (m_sceneGraph.getRoot()->getChildCount() == 0
        && m_selectedNodes.empty()) {
        return;
    }

    ImGui::Begin("Transforms");

    if (m_selectedNodes.size() == 1) {
        ImGui::Text("1 object selected");
    } else {
        ImGui::Text("%zu objects selected", m_selectedNodes.size());
    }

    ImGui::Separator();

    ImGui::Text("Position");

    // Position

    static char xTransform[64];
    static char yTransform[64];
    static char zTransform[64];
    static glm::vec3 lastInputPosition = glm::vec3(0.0f);
    static bool positionInitialized = false;

    // For multiple selections, show the first object's values
    glm::vec3 currentPos = m_selectedNodes[0]->getData().getPosition();

    // Initialize or update display values
    if (!positionInitialized || m_selectedNodes.size() == 1) {
        snprintf(xTransform, sizeof(xTransform), "%.3f", currentPos.x);
        snprintf(yTransform, sizeof(yTransform), "%.3f", currentPos.y);
        snprintf(zTransform, sizeof(zTransform), "%.3f", currentPos.z);
        lastInputPosition = currentPos;
        positionInitialized = true;
    }

    ImGui::InputText("x", xTransform, IM_ARRAYSIZE(xTransform));
    ImGui::InputText("y", yTransform, IM_ARRAYSIZE(yTransform));
    ImGui::InputText("z", zTransform, IM_ARRAYSIZE(zTransform));

    try {
        glm::vec3 newInputPos = { std::stof(xTransform), std::stof(yTransform),
            std::stof(zTransform) };

        // Calculate delta from last input
        glm::vec3 delta = newInputPos - lastInputPosition;

        // Only apply if there's an actual change
        if (glm::length(delta) > 0.0001f) {
            // Apply delta to all selected objects
            for (auto *node : m_selectedNodes) {
                node->getData().setPosition(
                    node->getData().getPosition() + delta);
            }
            lastInputPosition = newInputPos;
        }
    } catch (const std::invalid_argument &) {
    }

    // Rotation

    ImGui::Text("Rotation (degrees)");
    char xRot[64];
    char yRot[64];
    char zRot[64];
    static glm::vec3 lastInputRotation = glm::vec3(0.0f);
    static bool rotationInitialized = false;

    // Convert radians to degrees for display
    glm::vec3 currentRotDeg
        = glm::degrees(m_selectedNodes[0]->getData().getRotation());

    // Initialize or update display values
    if (!rotationInitialized || m_selectedNodes.size() == 1) {
        snprintf(xRot, sizeof(xRot), "%.3f", currentRotDeg.x);
        snprintf(yRot, sizeof(yRot), "%.3f", currentRotDeg.y);
        snprintf(zRot, sizeof(zRot), "%.3f", currentRotDeg.z);
        lastInputRotation = currentRotDeg;
        rotationInitialized = true;
    }

    ImGui::InputText("rot x", xRot, IM_ARRAYSIZE(xRot));
    ImGui::InputText("rot y", yRot, IM_ARRAYSIZE(yRot));
    ImGui::InputText("rot z", zRot, IM_ARRAYSIZE(zRot));
    try {
        // Convert degrees to radians when setting
        glm::vec3 newInputRotDeg
            = { std::stof(xRot), std::stof(yRot), std::stof(zRot) };

        // Calculate delta from last input
        glm::vec3 deltaDeg = newInputRotDeg - lastInputRotation;

        // Only apply if there's an actual change
        if (glm::length(deltaDeg) > 0.0001f) {
            glm::vec3 deltaRad = glm::radians(deltaDeg);
            // Apply delta to all selected objects
            for (auto *node : m_selectedNodes) {
                node->getData().setRotation(
                    node->getData().getRotation() + deltaRad);
            }
            lastInputRotation = newInputRotDeg;
        }
    } catch (const std::invalid_argument &) {
    }

    // Scale

    ImGui::Text("Scale");
    char xScale[64];
    char yScale[64];
    char zScale[64];
    static glm::vec3 lastInputScale = glm::vec3(1.0f);
    static bool scaleInitialized = false;

    glm::vec3 currentScale = m_selectedNodes[0]->getData().getScale();

    // Initialize or update display values
    if (!scaleInitialized || m_selectedNodes.size() == 1) {
        snprintf(xScale, sizeof(xScale), "%.3f", currentScale.x);
        snprintf(yScale, sizeof(yScale), "%.3f", currentScale.y);
        snprintf(zScale, sizeof(zScale), "%.3f", currentScale.z);
        lastInputScale = currentScale;
        scaleInitialized = true;
    }

    ImGui::InputText("scale x", xScale, IM_ARRAYSIZE(xScale));
    ImGui::InputText("scale y", yScale, IM_ARRAYSIZE(yScale));
    ImGui::InputText("scale z", zScale, IM_ARRAYSIZE(zScale));
    try {
        glm::vec3 newInputScale
            = { std::stof(xScale), std::stof(yScale), std::stof(zScale) };

        // Calculate scale ratio from last input
        glm::vec3 scaleRatio = newInputScale / lastInputScale;

        // Only apply if there's an actual change
        if (glm::length(scaleRatio - glm::vec3(1.0f)) > 0.0001f) {
            // Apply scale ratio to all selected objects
            for (auto *node : m_selectedNodes) {
                node->getData().setScale(
                    node->getData().getScale() * scaleRatio);
            }
            lastInputScale = newInputScale;
        }
    } catch (const std::invalid_argument &) {
    }

    ImGui::Separator();

    // Gizmo operation selection
    ImGui::Text("Gizmo Mode");
    if (ImGui::RadioButton(
            "Translate (T)", m_currentGizmoOperation == GizmoOp::Translate)) {
        m_currentGizmoOperation = GizmoOp::Translate;
    }
    ImGui::SameLine();
    if (ImGui::RadioButton(
            "Rotate (R)", m_currentGizmoOperation == GizmoOp::Rotate)) {
        m_currentGizmoOperation = GizmoOp::Rotate;
    }
    ImGui::SameLine();
    if (ImGui::RadioButton(
            "Scale (S)", m_currentGizmoOperation == GizmoOp::Scale)) {
        m_currentGizmoOperation = GizmoOp::Scale;
    }

    ImGui::End();
}

void App::renderCameraGizmo(int cameraId, const Camera &camera,
    ImVec2 imagePos, ImVec2 imageSize, bool isHovered)
{
    (void)cameraId;

    if (m_selectedNodes.empty()) {
        return;
    }

    const Camera *focused = m_camera.getFocusedCamera();
    if (focused != &camera) {
        return;
    }

    // Use the first selected node for the gizmo
    auto *primaryNode = m_selectedNodes[0];
    auto view = camera.getViewMatrix();
    auto proj = camera.getProjectionMatrix();
    auto worldMatrix = primaryNode->getWorldMatrix();
    auto parentWorldMatrix = primaryNode->getParentWorldMatrix();

    ImGuizmo::SetDrawlist();
    ImGuizmo::SetRect(imagePos.x, imagePos.y, imageSize.x, imageSize.y);

    ImGuizmo::OPERATION operation;
    switch (m_currentGizmoOperation) {
        case GizmoOp::Translate:
            operation = ImGuizmo::TRANSLATE;
            break;
        case GizmoOp::Rotate:
            operation = ImGuizmo::ROTATE;
            break;
        case GizmoOp::Scale:
            operation = ImGuizmo::SCALE;
            break;
        default:
            operation = ImGuizmo::TRANSLATE;
            break;
    }

    // Store initial transforms for relative manipulation across multiple
    // objects
    static std::vector<glm::vec3> initialPositions;
    static std::vector<glm::vec3> initialRotations;
    static std::vector<glm::vec3> initialScales;
    static glm::vec3 primaryInitialPos;
    static glm::vec3 primaryInitialRot;
    static glm::vec3 primaryInitialScale;
    static bool isManipulating = false;

    if (ImGuizmo::Manipulate(&view[0][0], &proj[0][0], operation,
            ImGuizmo::LOCAL, &worldMatrix[0][0])) {
        if (ImGuizmo::IsUsing()) {
            // Store initial state when starting manipulation
            if (!isManipulating) {
                isManipulating = true;
                primaryInitialPos = primaryNode->getData().getPosition();
                primaryInitialRot = primaryNode->getData().getRotation();
                primaryInitialScale = primaryNode->getData().getScale();

                initialPositions.clear();
                initialRotations.clear();
                initialScales.clear();

                for (auto *node : m_selectedNodes) {
                    initialPositions.push_back(node->getData().getPosition());
                    initialRotations.push_back(node->getData().getRotation());
                    initialScales.push_back(node->getData().getScale());
                }
            }

            // Convert world matrix back to local space
            glm::mat4 parentWorldInverse = glm::inverse(parentWorldMatrix);
            glm::mat4 localMatrix = parentWorldInverse * worldMatrix;

            glm::vec3 translation, rotation, scale;
            ImGuizmo::DecomposeMatrixToComponents(
                &localMatrix[0][0], &translation[0], &rotation[0], &scale[0]);

            // Calculate deltas from primary object's initial state
            glm::vec3 deltaPos = translation - primaryInitialPos;
            glm::vec3 deltaRot
                = glm::radians(glm::vec3(rotation.x, rotation.y, rotation.z))
                - primaryInitialRot;

            // Apply transformations to all selected objects
            for (size_t i = 0; i < m_selectedNodes.size(); ++i) {
                if (operation == ImGuizmo::TRANSLATE) {
                    m_selectedNodes[i]->getData().setPosition(
                        initialPositions[i] + deltaPos);
                } else if (operation == ImGuizmo::ROTATE) {
                    m_selectedNodes[i]->getData().setRotation(
                        initialRotations[i] + deltaRot);
                } else if (operation == ImGuizmo::SCALE) {
                    // For scale, multiply rather than add for better results
                    glm::vec3 scaleRatio = scale / primaryInitialScale;
                    m_selectedNodes[i]->getData().setScale(
                        initialScales[i] * scaleRatio);
                }
            }
        }
    } else {
        isManipulating = false;
    }

    // Object picking: select object on left-click within this camera view
    if (isHovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left)
        && !ImGuizmo::IsUsing()) {
        // Mouse in ImGui screen coords
        ImVec2 mouse = ImGui::GetMousePos();
        // Local position inside the image (top-left origin)
        const float localX = mouse.x - imagePos.x;
        const float localY = mouse.y - imagePos.y;
        if (localX >= 0.0f && localY >= 0.0f && localX <= imageSize.x
            && localY <= imageSize.y) {
            // Convert to NDC (-1..1). Y is inverted (top-left -> +1)
            const float ndcX = (localX / imageSize.x) * 2.0f - 1.0f;
            const float ndcY = 1.0f - (localY / imageSize.y) * 2.0f;

            const glm::mat4 invVP = glm::inverse(proj * view);
            const glm::vec4 nearClip(ndcX, ndcY, -1.0f, 1.0f);
            const glm::vec4 farClip(ndcX, ndcY, 1.0f, 1.0f);

            glm::vec4 nearWorld = invVP * nearClip;
            glm::vec4 farWorld = invVP * farClip;
            if (nearWorld.w != 0.0f) {
                nearWorld /= nearWorld.w;
            }
            if (farWorld.w != 0.0f) {
                farWorld /= farWorld.w;
            }

            const glm::vec3 rayOrigin = glm::vec3(nearWorld);
            glm::vec3 rayDir = glm::normalize(glm::vec3(farWorld - nearWorld));

            auto intersectsAABB
                = [](const glm::vec3 &origin, const glm::vec3 &dir,
                      const glm::vec3 &bmin, const glm::vec3 &bmax,
                      float &tHit) -> bool {
                const float EPS = 1e-6f;
                float tmin = -std::numeric_limits<float>::infinity();
                float tmax = std::numeric_limits<float>::infinity();

                for (int axis = 0; axis < 3; ++axis) {
                    const float o = origin[axis];
                    const float d = dir[axis];
                    const float minA = bmin[axis];
                    const float maxA = bmax[axis];

                    if (std::abs(d) < EPS) {
                        if (o < minA || o > maxA) {
                            return false;
                        }
                        continue;
                    }
                    const float invD = 1.0f / d;
                    float t1 = (minA - o) * invD;
                    float t2 = (maxA - o) * invD;
                    if (t1 > t2) {
                        std::swap(t1, t2);
                    }
                    tmin = std::max(tmin, t1);
                    tmax = std::min(tmax, t2);
                    if (tmin > tmax) {
                        return false;
                    }
                }
                tHit = (tmin >= 0.0f) ? tmin : tmax;
                return tHit >= 0.0f;
            };

            SceneGraph::Node *bestNode = nullptr;
            float bestTHit = std::numeric_limits<float>::infinity();

            // Traverse scene graph to find all nodes
            m_sceneGraph.traverse([&](SceneGraph::Node &node, int depth) {
                (void)depth;
                const GameObject &obj = node.getData();
                const glm::mat4 M = node.getWorldMatrix();
                const glm::vec3 a = obj.getAABBCorner1();
                const glm::vec3 b = obj.getAABBCorner2();
                const glm::vec3 amin = glm::min(a, b);
                const glm::vec3 amax = glm::max(a, b);

                // Transform 8 corners to world, then compute world AABB
                glm::vec3 corners[8] = {
                    { amin.x, amin.y, amin.z },
                    { amax.x, amin.y, amin.z },
                    { amin.x, amax.y, amin.z },
                    { amax.x, amax.y, amin.z },
                    { amin.x, amin.y, amax.z },
                    { amax.x, amin.y, amax.z },
                    { amin.x, amax.y, amax.z },
                    { amax.x, amax.y, amax.z },
                };

                glm::vec3 wmin(std::numeric_limits<float>::infinity());
                glm::vec3 wmax(-std::numeric_limits<float>::infinity());
                for (const auto &c : corners) {
                    glm::vec4 w = M * glm::vec4(c, 1.0f);
                    wmin = glm::min(wmin, glm::vec3(w));
                    wmax = glm::max(wmax, glm::vec3(w));
                }

                float t;
                if (intersectsAABB(rayOrigin, rayDir, wmin, wmax, t)) {
                    if (t < bestTHit) {
                        bestTHit = t;
                        bestNode = &node;
                    }
                }
            });

            if (bestNode) {
                m_selectedNodes.clear();
                m_selectedNodes.push_back(bestNode);
            }
        }
    }
}

static void DrawCameraManagerUI(
    CameraManager &cameraManager, ARenderer &renderer)
{
    ImGui::Begin("Camera Manager");

    // Create / Destroy
    if (ImGui::Button("Create Camera")) {
        const int id = cameraManager.createCamera();
        cameraManager.setFocused(id);
        cameraManager.setPosition({ 0.0f, 0.0f, 3.0f });
        cameraManager.setPerspective(id, 45.0f, 16.0f / 9.0f, 0.01f, 100.0f);
        renderer.createCameraViews(id, 512, 512);
    }

    ImGui::Separator();
    // List and select focus
    auto ids = cameraManager.getCameraIds();
    for (int id : ids) {
        ImGui::PushID(id);
        if (ImGui::SmallButton("Focus")) {
            cameraManager.setFocused(id);
        }
        ImGui::SameLine();
        ImGui::Text("Camera %d", id);
        ImGui::SameLine();
        if (ImGui::SmallButton("Destroy")) {
            renderer.destroyCameraViews(id);
            cameraManager.destroyCamera(id);
            ImGui::PopID();
            continue; // move to next id (this one is gone)
        }

        if (auto *cam = cameraManager.getCamera(id)) {
            // Pose controls
            glm::vec3 pos = cam->getPosition();
            glm::vec3 rot = cam->getRotation();
            if (ImGui::DragFloat3("Position##pos", &pos.x, 0.05f)) {
                cameraManager.setPosition(id, pos);
            }
            if (ImGui::DragFloat3("Rotation (deg)##rot", &rot.x, 0.1f)) {
                cameraManager.setRotation(id, rot.x, rot.y, rot.z);
            }

            // Projection controls
            bool isPerspective = cam->getProjectionMode()
                == Camera::ProjectionMode::Perspective;
            if (ImGui::Checkbox("Perspective##mode", &isPerspective)) {
                cameraManager.setProjectionMode(id,
                    isPerspective ? Camera::ProjectionMode::Perspective
                                  : Camera::ProjectionMode::Orthographic);
            }
            if (isPerspective) {
                float fov = cam->getFov();
                if (ImGui::DragFloat("FOV##fov_global", &fov, 0.1f, 10.0f,
                        160.0f, "%.1f")) {
                    cameraManager.setFov(id, fov);
                }
            } else {
                float size = cam->getOrthoSize();
                if (ImGui::DragFloat("Ortho Size##ortho_global", &size, 0.05f,
                        0.01f, 100.0f, "%.2f")) {
                    cameraManager.setOrthoSize(id, size);
                }
            }
        }
        ImGui::Separator();
        ImGui::PopID();
    }

    ImGui::End();
}

void App::render()
{
    m_renderer->beginFrame();

    vectorial_ui.renderUI(this);

    selectedTransformUI();
    m_image->renderUI();

    glm::vec4 paletteColor;
    if (m_image->consumeSelectedPaletteColor(paletteColor)) {
        vectorial_ui.setCurrentColorRGBA(paletteColor, true, true);
    }

    // Geometry UI
    m_GeometryImguiWindow.render();

    // Camera Manager UI
    DrawCameraManagerUI(m_camera, *m_renderer);

    m_sceneGraph.traverseWithTransform(
        [&](GameObject &obj, const glm::mat4 &worldTransform, int depth) {
            (void)depth;
            m_renderer->updateTransform(obj.rendererId, worldTransform);
        });

    m_renderer->renderAllViews(m_camera);

    // Update camera matrices
    m_renderer->endFrame();
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
    // Priority 1: camera panning (RMB held)
    if (firstMouse) {
        ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeAll);
        return;
    }

    // Priority 2: ImGuizmo operation (only while actively manipulating)
    if (ImGuizmo::IsUsing()) {
        switch (m_currentGizmoOperation) {
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

void App::resetAllCameraPoses()
{
    for (int id : m_camera.getCameraIds()) {
        if (auto *cam = m_camera.getCamera(id)) {
            cam->setPosition(glm::vec3(0.0f, 0.0f, 3.0f));
            cam->setRotation(0.0f, 0.0f, 0.0f);
            // Keep current projection parameters; aspect will be applied by
            // views
        }
    }
}

void App::drawBoundingBoxes()
{
    m_sceneGraph.traverse([&](SceneGraph::Node &node, int depth) {
        (void)depth;
        const GameObject &obj = node.getData();
        if (m_showAllBoundingBoxes || obj.isBoundingBoxActive()) {
            m_renderer->drawBoundingBox(
                obj.rendererId, obj.getAABBCorner1(), obj.getAABBCorner2());
        }
    });
}

// Helper function to check if a node can be added to the current selection
bool App::canAddToSelection(SceneGraph::Node *nodeToAdd)
{
    if (!nodeToAdd) {
        return false;
    }

    // Check if the node has a parent-child relationship with any selected node
    for (auto *selectedNode : m_selectedNodes) {
        if (nodeToAdd->hasParentChildRelationship(selectedNode)) {
            return false;
        }
    }

    return true;
}