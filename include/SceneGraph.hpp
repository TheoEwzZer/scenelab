#pragma once

#include "GameObject.hpp"
#include <memory>
#include <vector>
#include <functional>

class SceneGraph {
public:
    class Node {
    private:
        std::vector<std::unique_ptr<Node>> children;
        GameObject data;
        Node *parent = nullptr;

    public:
        Node() = default;
        ~Node() = default;

        Node(const Node &) = delete;
        Node &operator=(const Node &) = delete;

        void addChild(std::unique_ptr<Node> child);
        bool removeChild(Node *childToRemove);
        Node *getParent();
        GameObject &getData();

        int getChildCount() const { return static_cast<int>(children.size()); }

        void setData(const GameObject &newData);
        Node *getChild(int index);
        void traverse(
            std::function<void(GameObject &, int)> func, int depth = 0);
        void traverse(std::function<void(Node &, int)> func, int depth = 0);
        void traverseWithTransform(
            std::function<void(GameObject &, const glm::mat4 &, int)> func,
            const glm::mat4 &parentTransform = glm::mat4(1.0f), int depth = 0);
        glm::mat4 getParentWorldMatrix() const;
        glm::mat4 getWorldMatrix() const;
        bool isAncestorOf(const Node *other) const;
        bool isDescendantOf(const Node *other) const;
        bool hasParentChildRelationship(const Node *other) const;
    };

private:
    std::unique_ptr<Node> root;

public:
    SceneGraph() = default;
    ~SceneGraph() = default;

    SceneGraph(const SceneGraph &) = delete;
    SceneGraph &operator=(const SceneGraph &) = delete;

    Node *getRoot();
    void setRoot(std::unique_ptr<Node> newRoot);
    void traverse(std::function<void(GameObject &, int)> func);
    void traverse(std::function<void(Node &, int)> func);
    void traverseWithTransform(
        std::function<void(GameObject &, const glm::mat4 &, int)> func,
        const glm::mat4 &parentTransform = glm::mat4(1.0f));

    void renderHierarchyUI(std::vector<Node *> &selectedNodes,
        bool isMultiSelectKeyPressed,
        std::function<bool(Node *)> canAddToSelection);
};