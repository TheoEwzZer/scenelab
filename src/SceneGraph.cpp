#include "../include/SceneGraph.hpp"
#include "imgui.h"
#include <algorithm>
#include <string>

// Node methods implementation

void SceneGraph::Node::addChild(std::unique_ptr<Node> child)
{
    child->parent = this;
    children.push_back(std::move(child));
}

bool SceneGraph::Node::removeChild(Node *childToRemove)
{
    if (!childToRemove) {
        return false;
    }

    auto it = std::find_if(children.begin(), children.end(),
        [childToRemove](const std::unique_ptr<Node> &child) {
            return child.get() == childToRemove;
        });

    if (it != children.end()) {
        children.erase(it);
        return true;
    }
    return false;
}

SceneGraph::Node *SceneGraph::Node::getParent() { return parent; }

GameObject &SceneGraph::Node::getData() { return data; }

void SceneGraph::Node::setData(const GameObject &newData) { data = newData; }

SceneGraph::Node *SceneGraph::Node::getChild(int index)
{
    if (index < 0 || index >= static_cast<int>(children.size())) {
        return nullptr;
    }
    return children[index].get();
}

void SceneGraph::Node::traverse(
    std::function<void(GameObject &, int)> func, int depth)
{
    func(data, depth);
    for (auto &child : children) {
        child->traverse(func, depth + 1);
    }
}

void SceneGraph::Node::traverse(
    std::function<void(Node &, int)> func, int depth)
{
    func(*this, depth);
    for (auto &child : children) {
        child->traverse(func, depth + 1);
    }
}

void SceneGraph::Node::traverseWithTransform(
    std::function<void(GameObject &, const glm::mat4 &, int)> func,
    const glm::mat4 &parentTransform, int depth)
{
    const glm::mat4 &worldTransform = data.getWorldMatrix(parentTransform);
    func(data, worldTransform, depth);
    for (auto &child : children) {
        child->traverseWithTransform(func, worldTransform, depth + 1);
    }
}

glm::mat4 SceneGraph::Node::getParentWorldMatrix() const
{
    if (parent == nullptr) {
        return glm::mat4(1.0f);
    }
    return parent->getData().getWorldMatrix(parent->getParentWorldMatrix());
}

glm::mat4 SceneGraph::Node::getWorldMatrix() const
{
    return data.getWorldMatrix(getParentWorldMatrix());
}

bool SceneGraph::Node::isAncestorOf(const Node *other) const
{
    if (!other) {
        return false;
    }

    const Node *current = other->parent;
    while (current != nullptr) {
        if (current == this) {
            return true;
        }
        current = current->parent;
    }
    return false;
}

bool SceneGraph::Node::isDescendantOf(const Node *other) const
{
    if (!other) {
        return false;
    }
    return other->isAncestorOf(this);
}

bool SceneGraph::Node::hasParentChildRelationship(const Node *other) const
{
    if (!other) {
        return false;
    }
    return isAncestorOf(other) || isDescendantOf(other);
}

// SceneGraph methods implementation

SceneGraph::Node *SceneGraph::getRoot() { return root.get(); }

void SceneGraph::setRoot(std::unique_ptr<Node> newRoot)
{
    root = std::move(newRoot);
}

void SceneGraph::traverse(std::function<void(GameObject &, int)> func)
{
    if (root) {
        root->traverse(func);
    }
}

void SceneGraph::traverse(std::function<void(Node &, int)> func)
{
    if (root) {
        root->traverse(func);
    }
}

void SceneGraph::traverseWithTransform(
    std::function<void(GameObject &, const glm::mat4 &, int)> func,
    const glm::mat4 &parentTransform)
{
    if (root) {
        root->traverseWithTransform(func, parentTransform);
    }
}

void SceneGraph::renderHierarchyUI(std::vector<Node *> &selectedNodes,
    bool isMultiSelectKeyPressed,
    std::function<bool(Node *)> canAddToSelection)
{
    ImGui::Begin("Hierarchy");

    traverse([&](GameObject &obj, int depth) {
        std::string label = std::string(depth * 2, ' ');
        if (obj.rendererId < 0) {
            label += std::string(obj.m_name);
        } else {
            std::string objName(obj.m_name);
            if (!objName.empty() && objName != "Object") {
                label += objName;
            } else {
                label += "Object " + std::to_string(obj.rendererId);
            }
        }

        // Check if this object is selected
        bool isSelected = false;
        SceneGraph::Node *correspondingNode = nullptr;
        traverse([&](SceneGraph::Node &node, int) {
            if (node.getData().rendererId == obj.rendererId) {
                correspondingNode = &node;
                for (auto *selectedNode : selectedNodes) {
                    if (selectedNode == &node) {
                        isSelected = true;
                        break;
                    }
                }
            }
        });

        // Check if this object can be added to selection (for visual feedback)
        bool canSelect = canAddToSelection(correspondingNode);
        bool hasRelationship
            = !canSelect && !selectedNodes.empty() && !isSelected;

        // Highlight if selected
        if (isSelected) {
            ImGui::PushStyleColor(
                ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.0f, 1.0f));
        } else if (hasRelationship) {
            // Gray out objects that can't be selected due to parent-child
            // relationship
            ImGui::PushStyleColor(
                ImGuiCol_Text, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));
        }

        ImGui::Text("%s", label.c_str());

        if (isSelected || hasRelationship) {
            ImGui::PopStyleColor();
        }

        if (ImGui::IsItemClicked()) {
            // Check if multi-select key is held for multi-selection
            if (isMultiSelectKeyPressed) {
                // Toggle selection
                auto it = std::find(selectedNodes.begin(), selectedNodes.end(),
                    correspondingNode);
                if (it != selectedNodes.end()) {
                    // Remove from selection
                    selectedNodes.erase(it);
                } else {
                    // Check if we can add this node (no parent-child
                    // relationship with existing selections)
                    if (canAddToSelection(correspondingNode)) {
                        selectedNodes.push_back(correspondingNode);
                    }
                }
            } else {
                // Single selection - replace all
                selectedNodes.clear();
                selectedNodes.push_back(correspondingNode);
            }
        }
    });

    ImGui::End();
}