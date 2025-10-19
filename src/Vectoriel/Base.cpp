#include "Vectoriel.hpp"
#include <cmath>
#include <glm/ext/quaternion_geometric.hpp>
#include <glm/fwd.hpp>

// Abstract Primitive

namespace Vect::Primitive {
ASimplePrimitive::ASimplePrimitive() :
    GameObject(), m_type("Abstract Primitive"), m_filled(true),
    m_fillColor({ 0, 0, 0, 0 }), m_outlineColor({ 0, 0, 0, 0 }),
    m_outlineWidth(0), m_scale(1.0f, 1.0f), m_pos(0.0f, 0.0f),
    m_rotator_offset(0.0f)
{
}

void ASimplePrimitive::setOutlineWidth(float width) { m_outlineWidth = width; }

float ASimplePrimitive::getOutlineWidth() const { return m_outlineWidth; }

RGBAColor ASimplePrimitive::getOutlineColor() const { return m_outlineColor; }

RGBAColor ASimplePrimitive::getFillColor() const { return m_fillColor; }

void ASimplePrimitive::setOutlineColor(const RGBAColor &color)
{
    m_outlineColor = color;
}

void ASimplePrimitive::setFillColor(const RGBAColor &color)
{
    m_fillColor = color;
}

void ASimplePrimitive::setFilled(bool fill) { m_filled = fill; }

bool ASimplePrimitive::isFilled() { return m_filled; }

std::string ASimplePrimitive::getType() const { return m_type; }

glm::vec2 ASimplePrimitive::getLocalScale() const { return m_scale; }

void ASimplePrimitive::setLocalScale(const glm::vec2 &scale)
{
    m_scale = scale;
}

void ASimplePrimitive::setLocalPosition(const glm::vec2 &pos) { m_pos = pos; }

glm::vec2 ASimplePrimitive::getLocalPosition() const { return m_pos; }

float ASimplePrimitive::getLocalRotation() const
{
    return m_rotator_offset * 360.0f;
}

std::vector<float> ASimplePrimitive::getVertices()
{
    return (generateGLVertices());
}

void ASimplePrimitive::setLocalRotation(float angle)
{
    float mod = fmod(angle, 360.0f);

    if (mod < 0) {
        mod += 360.0f;
    }
    m_rotator_offset = mod / 360.0f;
}
}

// Abstract shape

namespace Vect::Shape {

AShape::AShape() : GameObject()
{
    m_type = "Abstract Shape";
    m_angle = 0.0f;
}

std::string AShape::getType() const { return m_type; }

void AShape::setColor(const RGBAColor &color)
{
    for (const auto &primitive : m_primitives) {
        primitive->setFillColor(color);
        primitive->setOutlineColor(color);
    }
}

std::vector<float> AShape::generateGLVertices() const
{
    std::vector<float> vertices;

    for (const auto &primitive : m_primitives) {
        const auto p_vertices = primitive->generateGLVertices();
        vertices.insert(vertices.end(), p_vertices.begin(), p_vertices.end());
    }
    return vertices;
}

std::vector<float> AShape::getVertices() { return (generateGLVertices()); }

}
