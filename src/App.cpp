#include "App.hpp"
#include "renderer/implementation/RasterizationRenderer.hpp"

#include "Camera.hpp"
#include "GLFW/glfw3.h"

#include "imgui.h"
#include "ImGuizmo.h"
#include <glm/geometric.hpp>
#include <glm/matrix.hpp>
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
    m_sceneGraph.getRoot()->addChild(std::make_unique<SceneGraph::Node>());
    m_sceneGraph.getRoot()->getChild(0)->setData(GameObject());
    m_sceneGraph.getRoot()->getChild(0)->getData().rendererId = m_renderer->registerObject(
        verticesAndNormal, {}, "../assets/wish-you-where-here.jpg", true);
    m_sceneGraph.getRoot()->addChild(std::make_unique<SceneGraph::Node>());
    m_sceneGraph.getRoot()->getChild(1)->setData(GameObject());
    m_sceneGraph.getRoot()->getChild(1)->getData().rendererId = m_renderer->registerObject(
        verticesAndNormal, {}, "../assets/wish-you-where-here.jpg", false);
    m_selectedNodes.push_back(m_sceneGraph.getRoot());

    m_sceneGraph.getRoot()->getChild(0)->getData().setPosition({ 1.2f, 0.f, 0.0f });
    m_sceneGraph.getRoot()->getChild(0)->getData().setScale(glm::vec3 { 0.2f });

    m_sceneGraph.traverseWithTransform([&](GameObject &obj, const glm::mat4 &worldTransform, int depth) {
        (void) depth;
        m_renderer->updateTransform(obj.rendererId, worldTransform);
    });

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
    // Render scene graph hierarchy with selection
    m_sceneGraph.renderHierarchyUI(
        m_selectedNodes,
        leftShiftPressed,
        [this](SceneGraph::Node *node) { return this->canAddToSelection(node); }
    );

    if (m_selectedNodes.empty()) {
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
        glm::vec3 newInputPos = { std::stof(xTransform), std::stof(yTransform), std::stof(zTransform) };

        // Calculate delta from last input
        glm::vec3 delta = newInputPos - lastInputPosition;

        // Only apply if there's an actual change
        if (glm::length(delta) > 0.0001f) {
            // Apply delta to all selected objects
            for (auto *node : m_selectedNodes) {
                node->getData().setPosition(node->getData().getPosition() + delta);
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
    glm::vec3 currentRotDeg = glm::degrees(m_selectedNodes[0]->getData().getRotation());

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
        glm::vec3 newInputRotDeg = { std::stof(xRot), std::stof(yRot), std::stof(zRot) };

        // Calculate delta from last input
        glm::vec3 deltaDeg = newInputRotDeg - lastInputRotation;

        // Only apply if there's an actual change
        if (glm::length(deltaDeg) > 0.0001f) {
            glm::vec3 deltaRad = glm::radians(deltaDeg);
            // Apply delta to all selected objects
            for (auto *node : m_selectedNodes) {
                node->getData().setRotation(node->getData().getRotation() + deltaRad);
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
        glm::vec3 newInputScale = { std::stof(xScale), std::stof(yScale), std::stof(zScale) };

        // Calculate scale ratio from last input
        glm::vec3 scaleRatio = newInputScale / lastInputScale;

        // Only apply if there's an actual change
        if (glm::length(scaleRatio - glm::vec3(1.0f)) > 0.0001f) {
            // Apply scale ratio to all selected objects
            for (auto *node : m_selectedNodes) {
                node->getData().setScale(node->getData().getScale() * scaleRatio);
            }
            lastInputScale = newInputScale;
        }
    } catch (const std::invalid_argument &) {
    }
    ImGui::End();

    // ImGuizmo manipulation
    
    auto view = m_camera.getViewMatrix();
    auto proj = m_camera.getProjectionMatrix();

    glm::mat4 worldMatrix = m_selectedNodes[0]->getWorldMatrix();

    glm::mat4 parentWorldMatrix = m_selectedNodes[0]->getParentWorldMatrix();

    static ImGuizmo::OPERATION currentGizmoOperation(ImGuizmo::TRANSLATE);
    static ImGuizmo::MODE currentGizmoMode(ImGuizmo::LOCAL);

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

    ImGui::End();

    // Store initial transforms for relative manipulation
    static std::vector<glm::vec3> initialPositions;
    static std::vector<glm::vec3> initialRotations;
    static std::vector<glm::vec3> initialScales;
    static glm::vec3 primaryInitialPos;
    static glm::vec3 primaryInitialRot;
    static glm::vec3 primaryInitialScale;
    static bool isManipulating = false;

    if (ImGuizmo::Manipulate(&view[0][0], &proj[0][0], currentGizmoOperation,
            currentGizmoMode, &worldMatrix[0][0])) {
        // Only update if ImGuizmo is actually being used
        if (ImGuizmo::IsUsing()) {
            // Store initial state when starting manipulation
            if (!isManipulating) {
                isManipulating = true;
                primaryInitialPos = m_selectedNodes[0]->getData().getPosition();
                primaryInitialRot = m_selectedNodes[0]->getData().getRotation();
                primaryInitialScale = m_selectedNodes[0]->getData().getScale();

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
            glm::vec3 deltaRot = glm::radians(glm::vec3(rotation.x, rotation.y, rotation.z)) - primaryInitialRot;
            glm::vec3 deltaScale = scale - primaryInitialScale;

            // Apply transformations to all selected objects
            for (size_t i = 0; i < m_selectedNodes.size(); ++i) {
                if (currentGizmoOperation == ImGuizmo::TRANSLATE) {
                    m_selectedNodes[i]->getData().setPosition(initialPositions[i] + deltaPos);
                } else if (currentGizmoOperation == ImGuizmo::ROTATE) {
                    m_selectedNodes[i]->getData().setRotation(initialRotations[i] + deltaRot);
                } else if (currentGizmoOperation == ImGuizmo::SCALE) {
                    // For scale, multiply rather than add for better results
                    glm::vec3 scaleRatio = scale / primaryInitialScale;
                    m_selectedNodes[i]->getData().setScale(initialScales[i] * scaleRatio);
                }
            }
        }
    } else {
        isManipulating = false;
    }
}

void App::render()
{
    m_renderer->beginFrame();

    selectedTransformUI();

    m_sceneGraph.traverseWithTransform([&](GameObject &obj, const glm::mat4 &worldTransform, int depth) {
        (void) depth;
        m_renderer->updateTransform(obj.rendererId, worldTransform);
    });

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
