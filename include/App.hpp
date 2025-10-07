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
#include "Vectoriel.hpp"
#include "renderer/interface/ARenderer.hpp"

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

    unsigned int selectedObjectIndex = 0;

private:
    std::vector<GameObject> m_gameObjects;
    Camera m_camera;
    void init();
    void update();
    void render();

    void selectedTransformUI();

    Vect::UIDrawer vectorial_ui;

public:
    explicit App();
    ~App();

    App(const App &) = delete;
    App &operator=(const App &) = delete;

    void run();

    std::unique_ptr<ARenderer> m_renderer;

    friend Vect::UIDrawer;
};
