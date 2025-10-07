#include "App.hpp"
#include "renderer/implementation/RasterizationRenderer.hpp"

#include "Camera.hpp"
#include "GLFW/glfw3.h"

#include "imgui.h"
#include "ImGuizmo.h"
#include <glm/geometric.hpp>
#include <glm/matrix.hpp>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

App::App()
{
    m_renderer = std::make_unique<RasterizationRenderer>();
    m_camera.setPosition({ 0.0f, 0.0f, 3.0f });
    m_camera.setProjection(45.0f, 1920.0f / 1080.0f, 0.1f, 100.0f);
}

App::~App() {}

void App::init()
{
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

    // Set initial position
    m_gameObjects[1].setPosition({ 1.2f, 0.f, 0.0f });
    m_gameObjects[1].setScale(glm::vec3 { 0.2f });

    for (const auto &obj : m_gameObjects) {
        m_renderer->updateTransform(obj.rendererId, obj.getModelMatrix());
    }

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
}

void App::update()
{
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

void App::selectedTransformUI()
{
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
    ImGui::End();

    // ImGuizmo manipulation

    auto view = m_camera.getViewMatrix();
    auto proj = m_camera.getProjectionMatrix();
    auto model = m_gameObjects[selectedObjectIndex].getModelMatrix();

    static ImGuizmo::OPERATION currentGizmoOperation(ImGuizmo::TRANSLATE);
    static ImGuizmo::MODE currentGizmoMode(ImGuizmo::WORLD);

    ImGui::Begin("Transformation Type");

    // Object selector
    ImGui::Text("Selected Object:");
    if (ImGui::RadioButton("Object 0", selectedObjectIndex == 0)) {
        selectedObjectIndex = 0;
    }
    ImGui::SameLine();
    if (ImGui::RadioButton("Object 1", selectedObjectIndex == 1)) {
        selectedObjectIndex = 1;
    }

    ImGui::Separator();

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

    ImGui::End();

    /*
    if (ImGui::IsKeyPressed(ImGuiKey_T))
    currentGizmoOperation = ImGuizmo::TRANSLATE;
    if (ImGui::IsKeyPressed(ImGuiKey_R))
    currentGizmoOperation = ImGuizmo::ROTATE;
    if (ImGui::IsKeyPressed(ImGuiKey_S))
    currentGizmoOperation = ImGuizmo::SCALE;
    */

    if (ImGuizmo::Manipulate(&view[0][0], &proj[0][0], currentGizmoOperation,
            currentGizmoMode, &model[0][0])) {
        // Only update if ImGuizmo is actually being used
        if (ImGuizmo::IsUsing()) {
            glm::vec3 translation, rotation, scale;
            ImGuizmo::DecomposeMatrixToComponents(
                &model[0][0], &translation[0], &rotation[0], &scale[0]);

            m_gameObjects[selectedObjectIndex].setPosition(translation);
            m_gameObjects[selectedObjectIndex].setRotation(
                glm::radians(glm::vec3(rotation.x, rotation.y, rotation.z)));
            m_gameObjects[selectedObjectIndex].setScale(scale);
        }
    }
}

void App::render()
{
    m_renderer->beginFrame();

    vectorial_ui.renderUI(this);

    selectedTransformUI();

    for (const auto &obj : m_gameObjects) {
        if (obj.hasTransformChanged()) {
            m_renderer->updateTransform(obj.rendererId, obj.getModelMatrix());
        }
    }

    // Update camera matrices
    m_renderer->setViewMatrix(m_camera.getViewMatrix());
    m_renderer->setProjectionMatrix(m_camera.getProjectionMatrix());

    m_renderer->drawAll();
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
