/*
** EPITECH PROJECT, 2024
** Raytracer
** File description:
** App
*/

#pragma once

#include <memory>

#include "GameObject.hpp"
#include "Camera.hpp"
#include "SceneGraph.hpp"
#include "renderer/interface/ARenderer.hpp"

class App {

    bool wPressed = false;
    bool sPressed = false;
    bool aPressed = false;
    bool dPressed = false;
    bool spacePressed = false;
    bool leftCtrlPressed = false;
    bool leftShiftPressed = false;
    glm::vec2 m_currentMousePos;
    bool firstMouse = false;

    glm::vec2 mouseDelta { 0.0f };
    glm::vec2 prevMousePos { 0.0f };

    // selected objects (multiple selection support)
    std::vector<SceneGraph::Node *> m_selectedNodes;

private:
    SceneGraph m_sceneGraph;

    Camera m_camera;
    void init();
    void update();
    void render();

    void selectedTransformUI();

    // Helper function for multi-selection validation
    bool canAddToSelection(SceneGraph::Node *nodeToAdd);

public:
    explicit App();
    ~App();

    App(const App &) = delete;
    App &operator=(const App &) = delete;

    void run();

    std::unique_ptr<ARenderer> m_renderer;
};
