#pragma once

#include <cstdint>
#include <glm/fwd.hpp>
#include <memory>
#include <string>
#include <vector>
#include "Vectoriel.hpp"
#include "GameObject.hpp"
#include "renderer/interface/ARenderer.hpp"

class App;

namespace Vect {

struct RGBAColor {
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a;
};

namespace Primitive {

    class ASimplePrimitive : public GameObject {
    public:
        ASimplePrimitive();
        virtual ~ASimplePrimitive() {};

        void setOutlineWidth(float width);
        float getOutlineWidth() const;
        void setOutlineColor(const RGBAColor &color);
        RGBAColor getOutlineColor() const;
        void setFillColor(const RGBAColor &color);
        RGBAColor getFillColor() const;

        void setFilled(bool fill);
        bool isFilled();
        std::string getType() const;

        void setLocalScale(const glm::vec2 &scale);
        glm::vec2 getLocalScale() const;

        void setLocalPosition(const glm::vec2 &pos);
        glm::vec2 getLocalPosition() const;

        void setLocalRotation(float angle);
        float getLocalRotation() const;

        virtual std::vector<float> generateGLVertices() const = 0;

    protected:
        std::string m_type;

        bool m_filled;
        RGBAColor m_fillColor;

        RGBAColor m_outlineColor;
        float m_outlineWidth;

        glm::vec2 m_scale;
        glm::vec2 m_pos;
        float m_rotator_offset;
    };

    class StraightLine : public ASimplePrimitive {
    public:
        StraightLine(
            const glm::vec2 &pointA, const glm::vec2 &pointB, float width);
        ~StraightLine();

        void setPoints(const glm::vec2 &pointA, const glm::vec2 &pointB);
        std::vector<float> generateGLVertices() const override;

    protected:
        glm::vec2 m_pointA;
        glm::vec2 m_pointB;

        std::vector<float> triangulate(float width, float z) const;
    };

    class RegularPolygon : public ASimplePrimitive {
    public:
        RegularPolygon(uint32_t segments = 5);
        ~RegularPolygon();

        std::vector<float> generateGLVertices() const override;

    protected:
        uint32_t m_segments;

        // Structure pour les jointures
        struct MiterVertices {
            glm::vec2 outer;
            glm::vec2 inner;
        };

        static MiterVertices calculateMiterJoint(const glm::vec2 &prev,
            const glm::vec2 &curr, const glm::vec2 &next, float halfWidth);
    };

    class Ellipse : public RegularPolygon {
    public:
        Ellipse(const glm::vec2 &radius = { 0.5f, 0.25f },
            uint32_t resolution = 100);
        ~Ellipse();

        void setResolution(uint32_t resolution);
        uint32_t getResolution() const;
    };

    class Circle : public Ellipse {
    public:
        Circle(float radius = 0.5f, uint32_t resolution = 100);
        ~Circle();
    };

    class Rectangle : public RegularPolygon {
    public:
        Rectangle(const glm::vec2 &size = { 0.5f, 0.25f });
        ~Rectangle();
    };

    class Triangle : public RegularPolygon {
    public:
        Triangle();
        ~Triangle();
    };

    class Square : public Rectangle {
    public:
        Square(float size = 0.5f);
        ~Square();
    };

    using Point = Circle;
}

namespace Shape {
    class AShape : public GameObject {
    public:
        AShape();
        virtual ~AShape() {};

        std::string getType() const;
        void setColor(const RGBAColor &color);

        std::vector<float> generateGLVertices() const;

    protected:
        std::string m_type;
        float m_angle;
        std::vector<std::unique_ptr<Vect::Primitive::ASimplePrimitive>>
            m_primitives;
    };

    class House : public AShape {
    public:
        House();
        ~House();
    };

    class Doll : public AShape {
    public:
        Doll();
        ~Doll();
    };

    class LetterA : public AShape {
    public:
        LetterA(float width = 0.05f);
        ~LetterA();
    };

}

class UIDrawer {
public:
    UIDrawer();
    ~UIDrawer();

    void renderUI(App *app);

protected:
    float m_outlineWidth;
    float m_outlineColor[4];
    float m_fillColor[4];
    float m_localScale[2];
    float m_localPosition[2];
    float m_localRotation;
    bool m_fill;

    float m_line_pointA[2];
    float m_line_pointB[2];
    float m_line_width;

    float m_circle_radius;

    ARenderer *rendererRef;

    template <typename T, class... Args>
    T instanciatePrimitiveWAttributes(Args &&...args) const
    {
        T primitive(std::forward<Args>(args)...);

        primitive.setFillColor(RGBAColor(
            m_fillColor[0], m_fillColor[1], m_fillColor[2], m_fillColor[3]));
        primitive.setFilled(m_fill);
        primitive.setOutlineWidth(m_outlineWidth);
        primitive.setOutlineColor(RGBAColor(m_outlineColor[0],
            m_outlineColor[1], m_outlineColor[2], m_outlineColor[3]));
        primitive.setLocalPosition(
            glm::vec2(m_localPosition[0], m_localPosition[1]));
        primitive.setLocalRotation(m_localRotation);
        primitive.setLocalScale(glm::vec2(m_localScale[0], m_localScale[1]));

        return (primitive);
    }
};

}
