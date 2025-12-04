//
// Created by clmonn on 11/28/25.
//

#ifndef SCENELAB_TRIANGULATION_H
#define SCENELAB_TRIANGULATION_H
#include "glm/vec2.hpp"
#include "glm/ext/scalar_uint_sized.hpp"

#include "../SceneGraph.hpp"
#include "../renderer/interface/IRenderer.hpp"

class Triangulation {
    struct Triangle {
        uint32_t a, b, c;

        bool operator==(const Triangle &other) const
        {
            return a == other.a && b == other.b && c == other.c;
        }
    };

    struct Edge {
        uint32_t a, b;

        bool operator==(const Edge &e) const
        {
            return (a == e.a && b == e.b) || (a == e.b && b == e.a);
        }
    };

    static float det(
        const glm::vec2 &a, const glm::vec2 &b, const glm::vec2 &c);

    static bool inCircle(const glm::vec2 &a, const glm::vec2 &b,
        const glm::vec2 &c, const glm::vec2 &p);

    static std::vector<Triangle> triangulate(
        const std::vector<glm::vec3> &pts3D);

public:
    void addPoint(SceneGraph::Node *node);
    void registerRenderable(int id);
    void updateGeometry(IRenderer &renderer) const;
    [[nodiscard]] bool isRenderable() const;
    [[nodiscard]] bool hasPoints() const;

    [[nodiscard]] bool canTriangulate() const
    {
        return m_controlPoints.size() >= 3;
    }

    [[nodiscard]] size_t getPointCount() const
    {
        return m_controlPoints.size();
    }

private:
    std::vector<SceneGraph::Node *> m_controlPoints;
    int m_renderableId = -1;
};

#endif // SCENELAB_TRIANGULATION_H
