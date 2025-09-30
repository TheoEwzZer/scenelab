#include "Vectoriel.hpp"
#include "GameObject.hpp"
#include <cstdint>
#include <glm/ext/quaternion_geometric.hpp>
#include <glm/fwd.hpp>
#include <vector>

// Abstract Primitive

ASimpleVectPrimitive::ASimpleVectPrimitive() :
    GameObject(), m_type("Abstract Primitive"), m_filled(true),
    m_fillColor({ 0, 0, 0, 0 }), m_outlineColor({ 0, 0, 0, 0 }),
    m_outlineWidth(0)
{
}

void ASimpleVectPrimitive::setOutlineWidth(float width)
{
    m_outlineWidth = width;
}

float ASimpleVectPrimitive::getOutlineWidth() const { return m_outlineWidth; }

RGBAColor ASimpleVectPrimitive::getOutlineColor() const
{
    return m_outlineColor;
}

void ASimpleVectPrimitive::setOutlineColor(const RGBAColor &color)
{
    m_outlineColor = color;
}

void ASimpleVectPrimitive::setFilled(bool fill) { m_filled = fill; }

bool ASimpleVectPrimitive::isFilled() { return m_filled; }

std::string ASimpleVectPrimitive::getType() const { return m_type; }

VectPolygon::VectPolygon(uint32_t segments) : ASimpleVectPrimitive()
{
    m_type = "Polygon";
    m_segments = segments;
    m_outlineWidth = 0.01f;
}

VectPolygon::~VectPolygon() {}

/**
 * @brief Permet de dessiner une ligne simple avec des triangles. Ne gère pas
 * les jointures.
 * @param p1 Point A
 * @param p2 Point B
 * @param width Épaisseur
 * @param z Position z
 * @return std::vector<float> Vertices générés
 */
std::vector<float> straightLineToTriangle(
    glm::vec2 p1, glm::vec2 p2, float width, float z)
{
    glm::vec2 perp = { -(p2.y - p1.y), (p2.x - p1.x) };
    glm::vec2 norm = glm::normalize(perp);
    glm::vec2 decal = norm * (width / 2.0f);

    return {
        p1.x + decal.x, p1.y + decal.y, z, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, // V1
        p1.x - decal.x, p1.y - decal.y, z, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, // V2
        p2.x + decal.x, p2.y + decal.y, z, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, // V3
        p2.x + decal.x, p2.y + decal.y, z, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, // V3
        p1.x - decal.x, p1.y - decal.y, z, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, // V2
        p2.x - decal.x, p2.y - decal.y, z, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, // V4
    };
}

VectPolygon::MiterVertices VectPolygon::calculateMiterJoint(
    glm::vec2 prev, glm::vec2 curr, glm::vec2 next, float halfWidth)
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

    return {
        curr + offset,
        curr - offset
    };
}

/**
 * @brief Génère les vertices pour un polygone avec m_segments (minimum 3),
 * gère les jointures.
 * @return std::vector<float> Vertices générés
 */
std::vector<float> VectPolygon::generateGLVertices() const
{
    const float z = 0.0f; // Position z
    std::vector<glm::vec2> linePoints(m_segments); // Points des segments
    std::vector<MiterVertices> jointVertices(
        m_segments); // Vertex avec jointure
    std::vector<float> glVertices(m_segments * 6 * 8); // Vertex finaux

    // Generation des points des lignes
    for (uint32_t r = 0; r < m_segments; r++) {
        linePoints[r] = glm::vec2(std::cos(r * ((2.0f * M_PI) / m_segments)),
            std::sin((r * ((2.0f * M_PI) / m_segments))));
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
        auto add_vertex = [&](glm::vec2 pos) {
            glVertices.insert(glVertices.end(),
                { pos.x, pos.y, z, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f });
        };
        add_vertex(startJoint.outer);
        add_vertex(startJoint.inner);
        add_vertex(endJoint.outer);
        add_vertex(endJoint.outer);
        add_vertex(startJoint.inner);
        add_vertex(endJoint.inner);
    }

    return glVertices;
}