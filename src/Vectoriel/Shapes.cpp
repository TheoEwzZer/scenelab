#include "Vectoriel.hpp"
#include <cstring>
#include <glm/ext/quaternion_geometric.hpp>
#include <glm/fwd.hpp>
#include <memory>
#include <vector>

namespace Vect::Shape {

House::House() : AShape()
{
    m_type = "Shape House";
    auto square = std::make_unique<Vect::Primitive::Square>(0.5f);
    auto triangle = std::make_unique<Vect::Primitive::RegularPolygon>(3);
    auto rect
        = std::make_unique<Vect::Primitive::Rectangle>(glm::vec2(0.1, 0.22));
    triangle->setLocalPosition({ 0, 0.5 });
    triangle->setLocalScale({ 0.55, 0.35f });
    triangle->setLocalRotation(-30);
    rect->setLocalPosition({ 0.28, 0.5 });

    m_primitives.emplace_back(std::move(square));
    m_primitives.emplace_back(std::move(triangle));
    m_primitives.emplace_back(std::move(rect));
    std::strncpy(m_name, m_type.c_str(), sizeof(m_name));
}

House::~House() {};

Doll::Doll() : AShape()
{
    m_type = "Shape Figure";
    auto rect
        = std::make_unique<Vect::Primitive::Rectangle>(glm::vec2(0.3, 0.1));
    auto triangle = std::make_unique<Vect::Primitive::Triangle>();
    auto circle = std::make_unique<Vect::Primitive::Circle>(0.30f);
    auto triangle2 = std::make_unique<Vect::Primitive::Triangle>();

    triangle->setLocalPosition({ 0, 0.75 });
    triangle->setLocalScale({ 0.4, 0.25 });

    circle->setLocalPosition({ 0, 0.35 });

    triangle2->setLocalPosition({ 0, -0.60 });
    triangle2->setLocalScale({ 0.55, 0.55 });

    m_primitives.emplace_back(std::move(triangle));
    m_primitives.emplace_back(std::move(circle));
    m_primitives.emplace_back(std::move(rect));
    m_primitives.emplace_back(std::move(triangle2));
    std::strncpy(m_name, m_type.c_str(), sizeof(m_name));
}

Doll::~Doll() {};

LetterA::LetterA(float width)
{
    m_type = "Shape A";
    const glm::vec2 P1(0.0f, 0.7f);
    const glm::vec2 P2(-0.4f, -0.7f);
    const glm::vec2 P3(0.4f, -0.7f);
    const glm::vec2 P4(-0.2f, 0.0f);
    const glm::vec2 P5(0.2f, 0.0f);

    auto ligne1
        = std::make_unique<Vect::Primitive::StraightLine>(P1, P2, width);
    m_primitives.emplace_back(std::move(ligne1));
    auto ligne2
        = std::make_unique<Vect::Primitive::StraightLine>(P1, P3, width);
    m_primitives.emplace_back(std::move(ligne2));
    auto ligne3
        = std::make_unique<Vect::Primitive::StraightLine>(P4, P5, width);
    m_primitives.emplace_back(std::move(ligne3));
    std::strncpy(m_name, m_type.c_str(), sizeof(m_name));
}

LetterA::~LetterA() {}

}