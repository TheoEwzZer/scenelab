#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include "GameObject.hpp"

struct RGBAColor {
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a;
};

class ASimpleVectPrimitive : public GameObject {
public:
    ASimpleVectPrimitive();
    virtual ~ASimpleVectPrimitive() {};

    void setOutlineWidth(float width);
    float getOutlineWidth() const;
    void setOutlineColor(const RGBAColor &color);
    RGBAColor getOutlineColor() const;

    void setFilled(bool fill);
    bool isFilled();
    std::string getType() const;

    virtual std::vector<float> generateGLVertices() const = 0;

protected:
    std::string m_type;

    bool m_filled;
    RGBAColor m_fillColor;

    RGBAColor m_outlineColor;
    float m_outlineWidth;
};

class VectPolygon : public ASimpleVectPrimitive {
public:
    VectPolygon(uint32_t segments);
    ~VectPolygon();

    std::vector<float> generateGLVertices() const override;

protected:
    uint32_t m_segments;

    // Structure pour les jointures
    struct MiterVertices {
        glm::vec2 outer;
        glm::vec2 inner;
    };

    static MiterVertices calculateMiterJoint(
        glm::vec2 prev, glm::vec2 curr, glm::vec2 next, float halfWidth);
};
