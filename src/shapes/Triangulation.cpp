//
// Created by clmonn on 11/28/25.
//

#include "../../include/shapes/Triangulation.hpp"

float Triangulation::det(
    const glm::vec2 &a, const glm::vec2 &b, const glm::vec2 &c)
{
    return (b.x - a.x) * (c.y - a.y) - (c.x - a.x) * (b.y - a.y);
}

bool Triangulation::inCircle(const glm::vec2 &a, const glm::vec2 &b,
    const glm::vec2 &c, const glm::vec2 &p)
{
    const float ax = a.x - p.x;
    const float ay = a.y - p.y;
    const float bx = b.x - p.x;
    const float by = b.y - p.y;
    const float cx = c.x - p.x;
    const float cy = c.y - p.y;

    const float inc = (ax * ax + ay * ay) * (bx * cy - by * cx)
        - (bx * bx + by * by) * (ax * cy - ay * cx)
        + (cx * cx + cy * cy) * (ax * by - ay * bx);

    // Respect triangle orientation: if (a,b,c) is CCW, inc>0 => p inside.
    // If (a,b,c) is CW, the sign should be inverted.
    const float orientation = det(a, b, c);
    if (orientation > 0.0f) {
        return inc > 0.0f;
    } else if (orientation < 0.0f) {
        return inc < 0.0f;
    } else {
        return false; // degenerate triangle
    }
}

std::vector<Triangulation::Triangle> Triangulation::triangulate(
    const std::vector<glm::vec3> &pts3D)
{
    const size_t n = pts3D.size();
    if (n < 3) {
        return {};
    }

    std::vector<glm::vec2> pts2D(n);
    pts2D.reserve(n);
    for (size_t i = 0; i < n; i++) {
        pts2D[i] = { pts3D[i].x, pts3D[i].y };
    }

    float minX = pts2D[0].x, maxX = pts2D[0].x;
    float minY = pts2D[0].y, maxY = pts2D[0].y;
    for (const auto &p : pts2D) {
        if (p.x < minX) {
            minX = p.x;
        }
        if (p.x > maxX) {
            maxX = p.x;
        }
        if (p.y < minY) {
            minY = p.y;
        }
        if (p.y > maxY) {
            maxY = p.y;
        }
    }

    const float dx = maxX - minX, dy = maxY - minY;
    const float delta = std::max(dx, dy) * 10.f;

    const glm::vec2 p1(minX - 1, minY - 1);
    const glm::vec2 p2(minX - 1, maxY + delta);
    const glm::vec2 p3(maxX + delta, minY - 1);

    std::vector<glm::vec2> ptsExt = pts2D;
    const uint32_t i1 = ptsExt.size();
    ptsExt.push_back(p1);
    const uint32_t i2 = ptsExt.size();
    ptsExt.push_back(p2);
    const uint32_t i3 = ptsExt.size();
    ptsExt.push_back(p3);

    std::vector<Triangle> tris;
    tris.push_back({ i1, i2, i3 });

    for (uint32_t p = 0; p < n; p++) {
        std::vector<Triangle> bad;
        for (auto &t : tris) {
            if (inCircle(ptsExt[t.a], ptsExt[t.b], ptsExt[t.c], ptsExt[p])) {
                bad.push_back(t);
            }
        }

        std::vector<Edge> poly;
        for (auto &bt : bad) {
            for (Edge e[3]
                 = { { bt.a, bt.b }, { bt.b, bt.c }, { bt.c, bt.a } };
                 auto k : e) {
                bool isShared = false;
                for (auto &bt2 : bad) {
                    if (&bt == &bt2) {
                        continue;
                    }
                    if (Edge(bt2.a, bt2.b) == k || Edge(bt2.b, bt2.c) == k
                        || Edge(bt2.c, bt2.a) == k) {
                        isShared = true;
                        break;
                    }
                }
                if (!isShared) {
                    poly.push_back(k);
                }
            }
        }

        std::erase_if(tris, [&](const Triangle &t) {
            return std::ranges::find(bad, t) != bad.end();
        });

        for (auto &[a, b] : poly) {
            tris.push_back({ a, b, p });
        }
    }

    std::erase_if(tris,
        [&](const Triangle &t) { return t.a >= n || t.b >= n || t.c >= n; });

    return tris;
}

void Triangulation::addPoint(SceneGraph::Node *node)
{
    m_controlPoints.push_back(node);
}

void Triangulation::registerRenderable(const int id) { m_renderableId = id; }

void Triangulation::updateGeometry(IRenderer &renderer) const
{
    if (m_renderableId == -1) {
        return;
    }
    std::vector<glm::vec3> controls;
    std::vector<float> vertices;

    for (auto *point : m_controlPoints) {
        if (point) {
            auto &obj = point->getData();
            controls.push_back(obj.getPosition());
        }
    }
    for (const auto triangles = triangulate(controls);
         const auto &[a, b, c] : triangles) {
        glm::vec3 p0 = controls[a];
        glm::vec3 p1 = controls[b];
        glm::vec3 p2 = controls[c];

        vertices.insert(vertices.end(), { p0.x, p0.y, p0.z });
        vertices.insert(vertices.end(), { p1.x, p1.y, p1.z });
        vertices.insert(vertices.end(), { p1.x, p1.y, p1.z });
        vertices.insert(vertices.end(), { p2.x, p2.y, p2.z });
        vertices.insert(vertices.end(), { p2.x, p2.y, p2.z });
        vertices.insert(vertices.end(), { p0.x, p0.y, p0.z });
    }
    renderer.updateGeometry(m_renderableId, vertices);
}

bool Triangulation::isRenderable() const { return m_renderableId != -1; }

bool Triangulation::hasPoints() const { return !m_controlPoints.empty(); }
