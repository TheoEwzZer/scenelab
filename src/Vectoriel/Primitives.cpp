#include "Vectoriel.hpp"
#include <cmath>
#include <cstdint>
#include <cstring>
#include <glm/ext/quaternion_geometric.hpp>
#include <glm/fwd.hpp>
#include <vector>

static const float CONST_POS_Z = 0.0f; // Position z des primitives

namespace Vect::Primitive {

StraightLine::StraightLine(const glm::vec2 &pointA, const glm::vec2 &pointB,
    float width) : ASimplePrimitive(), m_pointA(pointA), m_pointB(pointB)
{
    m_outlineWidth = width;
    m_type = "Straight Line";
    std::strncpy(m_name, m_type.c_str(), sizeof(m_name));
}

StraightLine::~StraightLine() {}

void StraightLine::setPoints(const glm::vec2 &pointA, const glm::vec2 &pointB)
{
    m_pointA = pointA;
    m_pointB = pointB;
}

std::vector<float> StraightLine::generateGLVertices() const
{
    return (triangulate(m_outlineWidth, CONST_POS_Z));
}

/**
 * @brief Permet de dessiner une ligne droite simple avec deux triangles. Ne
 * gère pas les jointures.
 * @param p1 Point A
 * @param p2 Point B
 * @param width Épaisseur
 * @param z Position z
 * @return std::vector<float> Vertices générés
 */
std::vector<float> StraightLine::triangulate(float width, float z) const
{
    glm::vec2 p1 = m_pointA + getLocalPosition();
    glm::vec2 p2 = m_pointB + getLocalPosition();

    glm::vec2 perp = { -(p2.y - p1.y), (p2.x - p1.x) };
    glm::vec2 norm = glm::normalize(perp);
    glm::vec2 decal = norm * (width / 2.0f);

    return {
        p1.x + decal.x, p1.y + decal.y, z, m_fillColor.r, m_fillColor.g,
        m_fillColor.b, m_fillColor.a, 1.0f, // V1
        p1.x - decal.x, p1.y - decal.y, z, m_fillColor.r, m_fillColor.g,
        m_fillColor.b, m_fillColor.a, 1.0f, // V2
        p2.x + decal.x, p2.y + decal.y, z, m_fillColor.r, m_fillColor.g,
        m_fillColor.b, m_fillColor.a, 1.0f, // V3
        p2.x + decal.x, p2.y + decal.y, z, m_fillColor.r, m_fillColor.g,
        m_fillColor.b, m_fillColor.a, 1.0f, // V3
        p1.x - decal.x, p1.y - decal.y, z, m_fillColor.r, m_fillColor.g,
        m_fillColor.b, m_fillColor.a, 1.0f, // V2
        p2.x - decal.x, p2.y - decal.y, z, m_fillColor.r, m_fillColor.g,
        m_fillColor.b, m_fillColor.a, 1.0f, // V4
    };
}

RegularPolygon::RegularPolygon(uint32_t segments) : ASimplePrimitive()
{
    m_type = "Polygon";
    m_segments = segments;
    std::strncpy(m_name, m_type.c_str(), sizeof(m_name));
}

RegularPolygon::~RegularPolygon() {}

RegularPolygon::MiterVertices RegularPolygon::calculateMiterJoint(
    const glm::vec2 &prev, const glm::vec2 &curr, const glm::vec2 &next,
    float halfWidth)
{
    glm::vec2 D_in = curr - prev;
    glm::vec2 D_out = next - curr;
    glm::vec2 N_in = glm::normalize(glm::vec2(-D_in.y, D_in.x));
    glm::vec2 N_out = glm::normalize(glm::vec2(-D_out.y, D_out.x));

    // Vecteur Miter
    glm::vec2 M = glm::normalize(N_in + N_out);

    // Facteur d'extension
    float L = 1.0f / glm::dot(M, N_in);

    // Calcule du décalage
    glm::vec2 offset = M * L * halfWidth;

    return { curr + offset, curr - offset };
}

/**
 * @brief Génère les vertices pour un polygone avec m_segments (minimum 3),
 * gère les jointures.
 * @return std::vector<float> Vertices générés
 */
std::vector<float> RegularPolygon::generateGLVertices() const
{
    std::vector<glm::vec2> linePoints(m_segments); // Points des segments
    std::vector<MiterVertices> jointVertices(
        m_segments); // Vertex avec jointure
    std::vector<float> glVertices(m_segments * 6 * 8); // Vertex finaux

    auto add_vertex = [&](glm::vec2 pos, RGBAColor color) {
        glVertices.insert(glVertices.end(),
            { pos.x, pos.y, CONST_POS_Z, color.r, color.g, color.b, color.a,
                1.0f });
    };

    // Generation des points des lignes
    for (uint32_t r = 0; r < m_segments; r++) {
        linePoints[r] = glm::vec2(std::cos(r * ((2.0f * M_PI) / m_segments)
                                      + (2.0f * M_PI * m_rotator_offset))
                                * m_scale.x,
                            std::sin((r * ((2.0f * M_PI) / m_segments))
                                + (2.0f * M_PI * m_rotator_offset))
                                * m_scale.y)
            + m_pos;
    }

    // Calcul des jointures
    for (uint32_t i = 0; i < m_segments; i++) {
        glm::vec2 prev = linePoints.at((i + m_segments - 1) % m_segments);
        glm::vec2 curr = linePoints.at(i);
        glm::vec2 next = linePoints.at((i + 1) % m_segments);
        jointVertices[i]
            = calculateMiterJoint(prev, curr, next, m_outlineWidth / 2.0f);
    }

    // Assemblage des jointures
    for (uint32_t i = 0; i < m_segments; i++) {
        MiterVertices startJoint = jointVertices.at(i);
        MiterVertices endJoint = jointVertices.at((i + 1) % m_segments);

        // Triangles: V1, V2, V3, V3, V2, V4
        add_vertex(startJoint.outer, m_outlineColor);
        add_vertex(startJoint.inner, m_outlineColor);
        add_vertex(endJoint.outer, m_outlineColor);
        add_vertex(endJoint.outer, m_outlineColor);
        add_vertex(startJoint.inner, m_outlineColor);
        add_vertex(endJoint.inner, m_outlineColor);
    }

    // Generation intérieure
    if (m_filled) {
        for (uint32_t i = 0; i < m_segments; i++) {
            add_vertex(m_pos, m_fillColor);
            add_vertex(linePoints[i], m_fillColor);
            add_vertex(linePoints[(i + 1) % m_segments], m_fillColor);
        }
    }

    return glVertices;
}

Ellipse::Ellipse(const glm::vec2 &radius, uint32_t resolution) :
    RegularPolygon(resolution)
{
    m_scale = radius;
    m_type = "Ellipse";
    std::strncpy(m_name, m_type.c_str(), sizeof(m_name));
}

Ellipse::~Ellipse() {}

void Ellipse::setResolution(uint32_t resolution) { m_segments = resolution; }

uint32_t Ellipse::getResolution() const { return (m_segments); }

Circle::Circle(float radius, uint32_t resolution) :
    Ellipse(glm::vec2(radius, radius), resolution)
{
    m_type = "Circle";
    std::strncpy(m_name, m_type.c_str(), sizeof(m_name));
}

Circle::~Circle() {}

Triangle::Triangle() : RegularPolygon(3)
{
    m_type = "Triangle";
    setLocalRotation(-30);
    std::strncpy(m_name, m_type.c_str(), sizeof(m_name));
}

Triangle::~Triangle() {}

Rectangle::Rectangle(const glm::vec2 &size) : RegularPolygon(4)
{
    m_type = "Rectangle";
    setLocalScale(size);
    setLocalRotation(45);
    std::strncpy(m_name, m_type.c_str(), sizeof(m_name));
}

Rectangle::~Rectangle() {}

Square::Square(float size) : Rectangle({ size, size })
{
    m_type = "Square";
    std::strncpy(m_name, m_type.c_str(), sizeof(m_name));
}

Square::~Square() {}

}
