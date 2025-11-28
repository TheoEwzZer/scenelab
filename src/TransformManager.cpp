#include "TransformManager.hpp"
#include "imgui.h"
#include "ImGuizmo.h"
#include <glm/gtc/matrix_transform.hpp>
#include <format>
#include <algorithm>
#include <limits>
#include <cmath>

TransformManager::TransformManager(
    SceneGraph &sceneGraph, std::unique_ptr<IRenderer> &renderer) :
    m_sceneGraph(sceneGraph),
    m_renderer(renderer)
{
}

void TransformManager::clearSelection() { m_selectedNodes.clear(); }

void TransformManager::selectNode(SceneGraph::Node *node)
{
    m_selectedNodes.clear();
    if (node) {
        m_selectedNodes.push_back(node);
    }
}

void TransformManager::addToSelection(SceneGraph::Node *node)
{
    if (node && canAddToSelection(node)) {
        m_selectedNodes.push_back(node);
    }
}

bool TransformManager::isNodeSelected(SceneGraph::Node *node) const
{
    return std::find(m_selectedNodes.begin(), m_selectedNodes.end(), node)
        != m_selectedNodes.end();
}

bool TransformManager::canAddToSelection(SceneGraph::Node *nodeToAdd)
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

void TransformManager::renderTransformUI(bool leftShiftPressed)
{
    // Render scene graph hierarchy with selection
    m_sceneGraph.renderHierarchyUI(
        m_selectedNodes, leftShiftPressed, [this](SceneGraph::Node *node) {
            return this->canAddToSelection(node);
        });

    // Only early-exit if there are no objects at all
    if (m_sceneGraph.getRoot()->getChildCount() == 0
        && m_selectedNodes.empty()) {
        return;
    }

    ImGui::Begin("Transforms");

    if (m_selectedNodes.empty()) {
        ImGui::Text("No objects selected");
        ImGui::End();
        return;
    }

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

    // Material Properties
    if (ImGui::CollapsingHeader("Material Properties")) {
        int rendererId = m_selectedNodes[0]->getData().rendererId;

        if (rendererId >= 0) {
            // Color
            glm::vec3 color = m_renderer->getObjectColor(rendererId);
            float colorArray[3] = { color.r, color.g, color.b };
            if (ImGui::ColorEdit3("Color", colorArray)) {
                glm::vec3 newColor(
                    colorArray[0], colorArray[1], colorArray[2]);
                for (auto *node : m_selectedNodes) {
                    int objId = node->getData().rendererId;
                    if (objId >= 0) {
                        m_renderer->setObjectColor(objId, newColor);
                    }
                }
            }

            // Emissive
            glm::vec3 emissive = m_renderer->getObjectEmissive(rendererId);
            float emissiveArray[3] = { emissive.r, emissive.g, emissive.b };
            if (ImGui::ColorEdit3("Emissive", emissiveArray)) {
                glm::vec3 newEmissive(
                    emissiveArray[0], emissiveArray[1], emissiveArray[2]);
                for (auto *node : m_selectedNodes) {
                    int objId = node->getData().rendererId;
                    if (objId >= 0) {
                        m_renderer->setObjectEmissive(objId, newEmissive);
                    }
                }
            }

            // Percent Specular
            float percentSpecular
                = m_renderer->getObjectPercentSpecular(rendererId);
            if (ImGui::SliderFloat(
                    "Percent Specular", &percentSpecular, 0.0f, 1.0f)) {
                for (auto *node : m_selectedNodes) {
                    int objId = node->getData().rendererId;
                    if (objId >= 0) {
                        m_renderer->setObjectPercentSpecular(
                            objId, percentSpecular);
                    }
                }
            }

            // Roughness
            float roughness = m_renderer->getObjectRoughness(rendererId);
            if (ImGui::SliderFloat("Roughness", &roughness, 0.0f, 1.0f)) {
                for (auto *node : m_selectedNodes) {
                    int objId = node->getData().rendererId;
                    if (objId >= 0) {
                        m_renderer->setObjectRoughness(objId, roughness);
                    }
                }
            }

            // Specular Color
            glm::vec3 specularColor
                = m_renderer->getObjectSpecularColor(rendererId);
            float specularArray[3]
                = { specularColor.r, specularColor.g, specularColor.b };
            if (ImGui::ColorEdit3("Specular Color", specularArray)) {
                glm::vec3 newSpecularColor(
                    specularArray[0], specularArray[1], specularArray[2]);
                for (auto *node : m_selectedNodes) {
                    int objId = node->getData().rendererId;
                    if (objId >= 0) {
                        m_renderer->setObjectSpecularColor(
                            objId, newSpecularColor);
                    }
                }
            }

            // Refraction Chance
            float refractionChance
                = m_renderer->getObjectRefractionChance(rendererId);
            if (ImGui::SliderFloat(
                    "Refraction Chance", &refractionChance, 0.0f, 1.0f)) {
                for (auto *node : m_selectedNodes) {
                    int objId = node->getData().rendererId;
                    if (objId >= 0) {
                        m_renderer->setObjectRefractionChance(
                            objId, refractionChance);
                    }
                }
            }

            // Index of Refraction
            float ior = m_renderer->getObjectIndexOfRefraction(rendererId);
            if (ImGui::SliderFloat("Index of Refraction", &ior, 1.0f, 2.5f)) {
                for (auto *node : m_selectedNodes) {
                    int objId = node->getData().rendererId;
                    if (objId >= 0) {
                        m_renderer->setObjectIndexOfRefraction(objId, ior);
                    }
                }
            }
        } else {
            ImGui::TextDisabled("No renderable object");
        }
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

    ImGui::Separator();

    // Bounding box controls (AABB - Axis-Aligned Bounding Boxes)
    ImGui::Text("Bounding Boxes (AABB)");
    if (ImGui::Checkbox("Show All BBoxes (B)", &m_showAllBoundingBoxes)) {
        // Toggle applied automatically by checkbox
    }
    ImGui::SameLine();
    ImGui::TextDisabled("(?)");
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip(
            "Display axis-aligned bounding boxes for all objects.\n"
            "Press 'B' key to toggle quickly.\n"
            "Individual boxes can be toggled per object below.");
    }

    if (m_selectedNodes.size() == 1) {
        GameObject &obj = m_selectedNodes[0]->getData();
        bool isBBoxActive = obj.isBoundingBoxActive();
        if (ImGui::Checkbox("Show Selected Object's BBox", &isBBoxActive)) {
            obj.setBoundingBoxActive(isBBoxActive);
        }
    } else if (m_selectedNodes.size() > 1) {
        ImGui::Text("Toggle individual BBoxes:");

        if (ImGui::Button("Enable All Selected")) {
            for (auto *node : m_selectedNodes) {
                node->getData().setBoundingBoxActive(true);
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Disable All Selected")) {
            for (auto *node : m_selectedNodes) {
                node->getData().setBoundingBoxActive(false);
            }
        }
    }

    ImGui::End();
}

void TransformManager::renderCameraGizmo(int cameraId, const Camera &camera,
    ImVec2 imagePos, ImVec2 imageSize, bool isHovered)
{
    (void)cameraId;

    if (m_selectedNodes.empty()) {
        return;
    }

    // Use the first selected node for the gizmo
    auto *primaryNode = m_selectedNodes[0];
    auto view = camera.getViewMatrix();
    auto proj = camera.getProjectionMatrix();
    auto worldMatrix = primaryNode->getWorldMatrix();
    auto parentWorldMatrix = primaryNode->getParentWorldMatrix();

    ImGuizmo::SetOrthographic(
        camera.getProjectionMode() == Camera::ProjectionMode::Orthographic);
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

                // Skip objects without a valid renderer (like the invisible
                // root)
                if (obj.rendererId < 0) {
                    return;
                }

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

void TransformManager::drawBoundingBoxes()
{
    m_sceneGraph.traverse([&](SceneGraph::Node &node, int depth) {
        (void)depth;
        const GameObject &obj = node.getData();
        // Only draw bounding boxes for objects that have a valid renderer
        if (obj.rendererId >= 0
            && (m_showAllBoundingBoxes || obj.isBoundingBoxActive())) {
            m_renderer->drawBoundingBox(
                obj.rendererId, obj.getAABBCorner1(), obj.getAABBCorner2());
        }
    });
}

void TransformManager::deleteSelectedObjects()
{
    if (m_selectedNodes.empty()) {
        return;
    }

    std::vector<std::pair<int, SceneGraph::Node *>> toDelete;
    for (auto *node : m_selectedNodes) {
        // Don't delete the root node
        if (node && node->getParent() && node != m_sceneGraph.getRoot()) {
            int rendererId = node->getData().rendererId;
            SceneGraph::Node *parent = node->getParent();
            toDelete.push_back({ rendererId, parent });
        }
    }

    m_selectedNodes.clear();

    for (const auto &[rendererId, parent] : toDelete) {
        m_renderer->removeObject(rendererId);

        for (int i = 0; i < parent->getChildCount(); ++i) {
            auto *child = parent->getChild(i);
            if (child && child->getData().rendererId == rendererId) {
                parent->removeChild(child);
                break;
            }
        }
    }
}
