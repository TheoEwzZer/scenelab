#include "App.hpp"
#include "OBJLoader.hpp"
#include "GameObject.hpp"
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

App::App()
{
    m_renderer = std::make_unique<RasterizationRenderer>();
    // const int camId = m_camera.createCamera();
    // m_camera.setFocused(camId);
    // m_camera.setPosition({ 0.0f, 0.0f, 3.0f });
    // m_camera.setProjection(45.0f, 1920.0f / 1080.0f, 0.1f, 100.0f);

    m_image = std::make_unique<Image>(m_renderer, m_gameObjects, m_camera);

    m_renderer->setCameraOverlayCallback([this](int id, const Camera& camera, ImVec2 imagePos, ImVec2 imageSize, bool isHovered) {
        this->renderCameraGizmo(id, camera, imagePos, imageSize, isHovered);
    });

    m_renderer->setBoundingBoxDrawCallback([this]() {
        this->drawBoundingBoxes();
    });
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

        m_gameObjects.push_back(new_obj);
        selectedObjectIndex = m_gameObjects.size() - 1;

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

        m_gameObjects.push_back(new_obj);
        selectedObjectIndex = m_gameObjects.size() - 1;

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

        m_gameObjects.push_back(new_obj);
        selectedObjectIndex = m_gameObjects.size() - 1;

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

            m_gameObjects.push_back(new_obj);
            selectedObjectIndex = m_gameObjects.size() - 1;

            std::cout << std::format(
                "[INFO] Spawned instance of {}\n", filepath);
            resetAllCameraPoses();
        };
}

void App::init()
{
    /*
    // Create game objects
    m_gameObjects.resize(10);

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

    // Register with renderer
    m_gameObjects[0].rendererId = m_renderer->registerObject(
        verticesAndNormal, {}, "../assets/wish-you-where-here.jpg", false);
    m_gameObjects[1].rendererId = m_renderer->registerObject(
        verticesAndNormal, {}, "../assets/wish-you-where-here.jpg", true);

    // Make the initial asset visible in Image UI for histogram selection
    m_image->addImportedImagePath("../assets/wish-you-where-here.jpg");

    // Set initial position
    m_gameObjects[1].setPosition({ 1.2f, 0.f, 0.0f });
    m_gameObjects[1].setScale(glm::vec3 { 0.2f });

    for (const auto &obj : m_gameObjects) {
        m_renderer->updateTransform(obj.rendererId, obj.getModelMatrix());
    }
    */

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
            const std::size_t beforeCount = m_gameObjects.size();
            const bool added
                = m_image->addImageObjectAtScreenPos(p, mouseX, mouseY);
            if (added && m_gameObjects.size() > 0
                && m_gameObjects.size() != beforeCount) {
                selectedObjectIndex
                    = static_cast<int64_t>(m_gameObjects.size()) - 1;
                // Ensure renderer transform matches immediately so gizmo
                // centers on the image
                auto &obj
                    = m_gameObjects[static_cast<size_t>(selectedObjectIndex)];
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
    selectedObjectIndex = m_gameObjects.size();
    return (m_gameObjects.emplace_back(std::move(obj)));
}

void App::selectedTransformUI()
{
    if (m_gameObjects.empty() || selectedObjectIndex == -1) {
        return;
    }

    ImGui::Begin("Transforms");
    ImGui::Text("Position");

    // Position

    static char xTransform[64];
    static char yTransform[64];
    static char zTransform[64];

    snprintf(xTransform, sizeof(xTransform), "%.3f",
        m_gameObjects[selectedObjectIndex].getPosition().x);
    snprintf(yTransform, sizeof(yTransform), "%.3f",
        m_gameObjects[selectedObjectIndex].getPosition().y);
    snprintf(zTransform, sizeof(zTransform), "%.3f",
        m_gameObjects[selectedObjectIndex].getPosition().z);

    ImGui::InputText("x", xTransform, IM_ARRAYSIZE(xTransform));
    ImGui::InputText("y", yTransform, IM_ARRAYSIZE(yTransform));
    ImGui::InputText("z", zTransform, IM_ARRAYSIZE(zTransform));

    try {
        m_gameObjects[selectedObjectIndex].setPosition({ std::stof(xTransform),
            std::stof(yTransform), std::stof(zTransform) });
    } catch (const std::invalid_argument &) {
    }

    // Rotation

    ImGui::Text("Rotation (degrees)");
    char xRot[64];
    char yRot[64];
    char zRot[64];
    // Convert radians to degrees for display
    snprintf(xRot, sizeof(xRot), "%.3f",
        glm::degrees(m_gameObjects[selectedObjectIndex].getRotation().x));
    snprintf(yRot, sizeof(yRot), "%.3f",
        glm::degrees(m_gameObjects[selectedObjectIndex].getRotation().y));
    snprintf(zRot, sizeof(zRot), "%.3f",
        glm::degrees(m_gameObjects[selectedObjectIndex].getRotation().z));
    ImGui::InputText("rot x", xRot, IM_ARRAYSIZE(xRot));
    ImGui::InputText("rot y", yRot, IM_ARRAYSIZE(yRot));
    ImGui::InputText("rot z", zRot, IM_ARRAYSIZE(zRot));
    try {
        // Convert degrees to radians when setting
        m_gameObjects[selectedObjectIndex].setRotation(
            { glm::radians(std::stof(xRot)), glm::radians(std::stof(yRot)),
                glm::radians(std::stof(zRot)) });
    } catch (const std::invalid_argument &) {
    }

    // Scale

    ImGui::Text("Scale");
    char xScale[64];
    char yScale[64];
    char zScale[64];
    snprintf(xScale, sizeof(xScale), "%.3f",
        m_gameObjects[selectedObjectIndex].getScale().x);
    snprintf(yScale, sizeof(yScale), "%.3f",
        m_gameObjects[selectedObjectIndex].getScale().y);
    snprintf(zScale, sizeof(zScale), "%.3f",
        m_gameObjects[selectedObjectIndex].getScale().z);
    ImGui::InputText("scale x", xScale, IM_ARRAYSIZE(xScale));
    ImGui::InputText("scale y", yScale, IM_ARRAYSIZE(yScale));
    ImGui::InputText("scale z", zScale, IM_ARRAYSIZE(zScale));
    try {
        m_gameObjects[selectedObjectIndex].setScale(
            { std::stof(xScale), std::stof(yScale), std::stof(zScale) });
    } catch (const std::invalid_argument &) {
    }

    // Bounding box per obj
    ImGui::Separator();

    if (!m_showAllBoundingBoxes) {
        bool bboxActive
            = m_gameObjects[selectedObjectIndex].isBoundingBoxActive();
        if (ImGui::Checkbox("Show Bounding Box", &bboxActive)) {
            m_gameObjects[selectedObjectIndex].setBoundingBoxActive(
                bboxActive);
        }
    }

    ImGui::End();

    static ImGuizmo::OPERATION currentGizmoOperation(ImGuizmo::TRANSLATE);

    ImGui::Begin("Transformation Type");

    if (ImGui::RadioButton(
            "Translate (T)", currentGizmoOperation == ImGuizmo::TRANSLATE)) {
        currentGizmoOperation = ImGuizmo::TRANSLATE;
    }
    ImGui::SameLine();
    if (ImGui::RadioButton(
            "Rotate (R)", currentGizmoOperation == ImGuizmo::ROTATE)) {
        currentGizmoOperation = ImGuizmo::ROTATE;
    }
    ImGui::SameLine();
    if (ImGui::RadioButton(
            "Scale (S)", currentGizmoOperation == ImGuizmo::SCALE)) {
        currentGizmoOperation = ImGuizmo::SCALE;
    }

    // All bounding boxes
    ImGui::Separator();

    ImGui::Checkbox("Show All Bounding Boxes", &m_showAllBoundingBoxes);
    if (!m_showAllBoundingBoxes) {
        ImGui::SameLine();
        if (ImGui::Button("Hide All")) {
            for (auto &obj : m_gameObjects) {
                obj.setBoundingBoxActive(false);
            }
        }
    }

    // Object selector

    ImGui::Separator();

    ImGui::Text("Selected Object:");
    if (ImGui::BeginListBox("##object_list",
            ImVec2(0, 5 * ImGui::GetTextLineHeightWithSpacing()))) {
        for (std::size_t i = 0; i < m_gameObjects.size(); ++i) {
            ImGui::PushID((int)i);
            const bool isSelected = (selectedObjectIndex == i);
            if (ImGui::Selectable(m_gameObjects[i].m_name, isSelected)) {
                selectedObjectIndex = i;
            }
            if (isSelected) {
                ImGui::SetItemDefaultFocus();
            }
            ImGui::PopID();
        }
        ImGui::EndListBox();
    }

    ImGui::End();

    /*
    if (ImGui::IsKeyPressed(ImGuiKey_T))
    currentGizmoOperation = ImGuizmo::TRANSLATE;
    if (ImGui::IsKeyPressed(ImGuiKey_R))
    currentGizmoOperation = ImGuizmo::ROTATE;
    if (ImGui::IsKeyPressed(ImGuiKey_S))
    currentGizmoOperation = ImGuizmo::SCALE;
    */

    switch (currentGizmoOperation) {
        case ImGuizmo::TRANSLATE:
            m_currentGizmoOperation = GizmoOp::Translate;
            break;
        case ImGuizmo::ROTATE:
            m_currentGizmoOperation = GizmoOp::Rotate;
            break;
        case ImGuizmo::SCALE:
            m_currentGizmoOperation = GizmoOp::Scale;
            break;
        default:
            m_currentGizmoOperation = GizmoOp::Translate;
            break;
    }
}

void App::renderCameraGizmo(int cameraId, const Camera& camera, ImVec2 imagePos, ImVec2 imageSize, bool isHovered)
{
    if (selectedObjectIndex < 0
        || selectedObjectIndex >= static_cast<int>(m_gameObjects.size())) {
        return;
    }

    (void)isHovered;

    const Camera *focused = m_camera.getFocusedCamera();
    if (focused != &camera) {
        return;
    }

    auto &selectedObj = m_gameObjects[static_cast<size_t>(selectedObjectIndex)];
    auto view = camera.getViewMatrix();
    auto proj = camera.getProjectionMatrix();
    auto model = selectedObj.getModelMatrix();

    ImGuizmo::SetDrawlist();
    ImGuizmo::SetRect(imagePos.x, imagePos.y, imageSize.x, imageSize.y);

    ImGuizmo::OPERATION operation;
    switch (m_currentGizmoOperation) {
        case GizmoOp::Translate: operation = ImGuizmo::TRANSLATE; break;
        case GizmoOp::Rotate: operation = ImGuizmo::ROTATE; break;
        case GizmoOp::Scale: operation = ImGuizmo::SCALE; break;
        default: operation = ImGuizmo::TRANSLATE; break;
    }

    if (ImGuizmo::Manipulate(&view[0][0], &proj[0][0], operation,
            ImGuizmo::WORLD, &model[0][0])) {
        if (ImGuizmo::IsUsing()) {
            glm::vec3 translation, rotation, scale;
            ImGuizmo::DecomposeMatrixToComponents(
                &model[0][0], &translation[0], &rotation[0], &scale[0]);

            selectedObj.setPosition(translation);
            selectedObj.setRotation(
                glm::radians(glm::vec3(rotation.x, rotation.y, rotation.z)));
            selectedObj.setScale(scale);
        }
    }
}

static void DrawCameraManagerUI(CameraManager &cameraManager, ARenderer &renderer)
{
    ImGui::Begin("Camera Manager");

    // Create / Destroy
    if (ImGui::Button("Create Camera")) {
        const int id = cameraManager.createCamera();
        cameraManager.setFocused(id);
        cameraManager.setPosition({ 0.0f, 0.0f, 3.0f });
        cameraManager.setPerspective(id, 45.0f, 16.0f/9.0f, 0.01f, 100.0f);
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
            bool isPerspective = cam->getProjectionMode() == Camera::ProjectionMode::Perspective;
            if (ImGui::Checkbox("Perspective##mode", &isPerspective)) {
                cameraManager.setProjectionMode(id, isPerspective ? Camera::ProjectionMode::Perspective : Camera::ProjectionMode::Orthographic);
            }
            if (isPerspective) {
                float fov = cam->getFov();
                if (ImGui::DragFloat("FOV##fov_global", &fov, 0.1f, 10.0f, 160.0f, "%.1f")) {
                    cameraManager.setFov(id, fov);
                }
            } else {
                float size = cam->getOrthoSize();
                if (ImGui::DragFloat("Ortho Size##ortho_global", &size, 0.05f, 0.01f, 100.0f, "%.2f")) {
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

    for (const auto &obj : m_gameObjects) {
        if (obj.hasTransformChanged()) {
            m_renderer->updateTransform(obj.rendererId, obj.getModelMatrix());
        }
    }

    // // Update camera matrices
    // m_renderer->setViewMatrix(m_camera.getViewMatrix());
    // m_renderer->setProjectionMatrix(m_camera.getProjectionMatrix());

    // m_renderer->drawAll();

    // for (const auto &obj : m_gameObjects) {
    //     if (m_showAllBoundingBoxes || obj.isBoundingBoxActive())
    //     [[unlikely]] {
    //         m_renderer->drawBoundingBox(
    //             obj.rendererId, obj.getAABBCorner1(), obj.getAABBCorner2());
    //     }
    // }
    // m_image->handleFrameExport(m_renderer->getWindow());

    // // Update cursor state at end of frame UI decisions
    // updateCursor();

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
            // Keep current projection parameters; aspect will be applied by views
        }
    }
}

void App::drawBoundingBoxes()
{
    for (const auto &obj : m_gameObjects) {
        if (m_showAllBoundingBoxes || obj.isBoundingBoxActive()) {
            m_renderer->drawBoundingBox(
                obj.rendererId, obj.getAABBCorner1(), obj.getAABBCorner2());
        }
    }
}
