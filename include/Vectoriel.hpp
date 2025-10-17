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
    float r;
    float g;
    float b;
    float a;
};

// Primitives vectorielles
namespace Primitive {

    /**
     * @brief Classe abstraite d'une primitive vectorielle
     */
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

        // Transformations dans l'espace locale de la primitive
        void setLocalScale(const glm::vec2 &scale);
        glm::vec2 getLocalScale() const;
        void setLocalPosition(const glm::vec2 &pos);
        glm::vec2 getLocalPosition() const;
        void setLocalRotation(float angle);
        float getLocalRotation() const;

        /**
         * @brief Fonction de génération de la primitive
         * @return std::vector<float> Attributs: Vertex1X, Vertex1Y, Vertex1Z,
         * Color1R, Color1G, Color1B, Color1A, 1.0
         */
        virtual std::vector<float> generateGLVertices() const = 0;
        std::vector<float> getVertices();

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

    /**
     * @brief Ligne droite entre deux points
     */
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

    /**
     * @brief Génération d'un polygone régulier convexe à n segments
     */
    class RegularPolygon : public ASimplePrimitive {
    public:
        RegularPolygon(uint32_t segments = 5);
        ~RegularPolygon();

        std::vector<float> generateGLVertices() const override;

    protected:
        uint32_t m_segments;

        // Structure pour des jointures propres
        struct MiterVertices {
            glm::vec2 outer;
            glm::vec2 inner;
        };

        static MiterVertices calculateMiterJoint(const glm::vec2 &prev,
            const glm::vec2 &curr, const glm::vec2 &next, float halfWidth);
    };

    /**
     * @brief Génération d'un ellipse.
     * La génération s'effectue à partir d'un polygone régulier convexe à haute
     * résolution (nombre de segments élevé).
     */
    class Ellipse : public RegularPolygon {
    public:
        Ellipse(const glm::vec2 &radius = { 0.5f, 0.25f },
            uint32_t resolution = 100);
        ~Ellipse();

        void setResolution(uint32_t resolution);
        uint32_t getResolution() const;
    };

    /**
     * @brief Génération d'un cercle
     */
    class Circle : public Ellipse {
    public:
        Circle(float radius = 0.5f, uint32_t resolution = 100);
        ~Circle();
    };

    /**
     * @brief Génération d'un rectangle
     */
    class Rectangle : public RegularPolygon {
    public:
        Rectangle(const glm::vec2 &size = { 0.5f, 0.25f });
        ~Rectangle();
    };

    /**
     * @brief Génération d'un triangle
     */
    class Triangle : public RegularPolygon {
    public:
        Triangle();
        ~Triangle();
    };

    /**
     * @brief Génération d'un carré
     */
    class Square : public Rectangle {
    public:
        Square(float size = 0.5f);
        ~Square();
    };

    using Point = Circle;
}

// Formes vectorielles
namespace Shape {
    /**
     * @brief Classe abstraite pour les formes vectorielles. Composés d'un
     * ensemble de ASimplePrimitive.
     */
    class AShape : public GameObject {
    public:
        AShape();
        virtual ~AShape() {};

        std::string getType() const;
        void setColor(const RGBAColor &color);

        std::vector<float> generateGLVertices() const;
        std::vector<float> getVertices();

    protected:
        std::string m_type;
        float m_angle;
        std::vector<std::unique_ptr<Vect::Primitive::ASimplePrimitive>>
            m_primitives;
    };

    /**
     * @brief Génération d'un forme maison.
     */
    class House : public AShape {
    public:
        House();
        ~House();
    };

    /**
     * @brief Génération d'une personnage.
     */
    class Doll : public AShape {
    public:
        Doll();
        ~Doll();
    };

    /**
     * @brief Génération de la lettre A
     */
    class LetterA : public AShape {
    public:
        LetterA(float width = 0.05f);
        ~LetterA();
    };

}

/**
 * @brief Classe de l'interface graphique du dessin vectoriel
 */
class UIDrawer {
public:
    UIDrawer();
    ~UIDrawer();

    /**
     * @brief Fonction de rendu de l'interface graphique.
     * @param app Pointeur sur l'application
     */
    void renderUI(App *app);

protected:
    void renderUIPrimitive(App *app);
    void renderUIShape(App *app);

    // Attributs modifiés par les widgets imGui. Utilisés pour générer les
    // formes et primitives vectorielles.
    float m_outlineWidth;
    float m_outlineColor[4];
    float m_fillColor[4];
    float m_localScale[2];
    float m_line_pointA[2];
    float m_line_pointB[2];
    float m_line_width;
    float m_circle_radius;
    bool m_fill;
    int m_input_segments;

    /**
     * @brief Fonction utilitaire qui instancie une primitive avec les valeurs
     * courante des outils de l'interface.
     *
     * @tparam T Classe de primitive à instancier
     * @param args Arguments à passer au constructeur de la primitive
     * @return T Primitive instancié
     */
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

        return (primitive);
    }
};

}
