#pragma once

#include "GameObject.hpp"
#include "SceneGraph.hpp"
#include "renderer/interface/ARenderer.hpp"
#include "Camera.hpp"
#include <vector>
#include <memory>
#include <functional>
#include <glm/glm.hpp>

// Forward declarations
struct ImVec2;

enum class GizmoOp { Translate, Rotate, Scale };

class TransformManager {
public:
    TransformManager(
        SceneGraph &sceneGraph, std::unique_ptr<ARenderer> &renderer);

    // Selection management
    void clearSelection();
    void selectNode(SceneGraph::Node *node);
    void addToSelection(SceneGraph::Node *node);
    bool isNodeSelected(SceneGraph::Node *node) const;

    const std::vector<SceneGraph::Node *> &getSelectedNodes() const
    {
        return m_selectedNodes;
    }

    bool canAddToSelection(SceneGraph::Node *nodeToAdd);

    // Transform UI
    void renderTransformUI(bool leftShiftPressed);
    void renderCameraGizmo(int cameraId, const Camera &camera, ImVec2 imagePos,
        ImVec2 imageSize, bool isHovered);

    // Gizmo operations
    GizmoOp getCurrentGizmoOperation() const
    {
        return m_currentGizmoOperation;
    }

    void setGizmoOperation(GizmoOp op) { m_currentGizmoOperation = op; }

    // Bounding boxes
    void toggleBoundingBoxes()
    {
        m_showAllBoundingBoxes = !m_showAllBoundingBoxes;
    }

    void drawBoundingBoxes();

    bool isShowingAllBoundingBoxes() const { return m_showAllBoundingBoxes; }

    // Object operations
    void deleteSelectedObjects();

private:
    SceneGraph &m_sceneGraph;
    std::unique_ptr<ARenderer> &m_renderer;
    std::vector<SceneGraph::Node *> m_selectedNodes;
    GizmoOp m_currentGizmoOperation = GizmoOp::Translate;
    bool m_showAllBoundingBoxes = false;
};
