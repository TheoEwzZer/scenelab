#pragma once

#include "GameObject.hpp"
#include "SceneGraph.hpp"
#include "GeometryImguiWindow.hpp"
#include "renderer/interface/ARenderer.hpp"
#include <memory>
#include <functional>
#include <string>

class GeometryManager {
public:
    GeometryManager(
        SceneGraph &sceneGraph, std::unique_ptr<ARenderer> &renderer);

    // Initialize geometry window callbacks
    void initGeometryWindow(std::function<void()> onObjectCreated);

    // Render geometry UI
    void renderUI();

    // Access to the window for external configuration
    GeometryImguiWindow &getGeometryWindow() { return m_geometryWindow; }

private:
    SceneGraph &m_sceneGraph;
    std::unique_ptr<ARenderer> &m_renderer;
    GeometryImguiWindow m_geometryWindow;
};
