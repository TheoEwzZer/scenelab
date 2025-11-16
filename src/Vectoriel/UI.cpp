#include "App.hpp"
#include "Vectoriel.hpp"
#include "imgui.h"
#include <glm/fwd.hpp>
#include <glm/glm.hpp>

#include "objects/Object2D.h++"

using namespace Vect;

UIDrawer::UIDrawer() :
    m_outlineWidth(0.05f), m_outlineColor(1, 1, 1, 1), m_fillColor(1, 1, 1, 1),
    m_localScale(1.0f, 1.0f), m_line_pointA(0, 0), m_line_pointB(1, 1),
    m_line_width(1.0f), m_circle_radius(1.0f), m_fill(false),
    m_input_segments(5)
{
}

/**
 * @brief Rendu des widgets du mode forme
 * @param app Référence à l'app
 */
void UIDrawer::renderUIShape(App *app)
{
    // Formes disponibles
    static const char *formes[] = { "House", "Doll", "Letter A" };
    static int current_forme_idx = 0;

    ImGui::ListBox(
        "Select forme", &current_forme_idx, formes, IM_ARRAYSIZE(formes));
    switch (current_forme_idx) {
        case 0:
            if (ImGui::Button("Generer Maison")) {
                auto house = Vect::Shape::House();
                house.setColor({ m_fillColor[0], m_fillColor[1],
                    m_fillColor[2], m_fillColor[3] });
                house.rendererId = app->m_renderer->registerObject(
                    std::make_unique<Object2D>(house.getVertices(), std::vector<unsigned int>{}));
                app->registerObject(house);
            }
            break;
        case 1:
            if (ImGui::Button("Generer Figure")) {
                auto doll = Vect::Shape::Doll();
                doll.setColor({ m_fillColor[0], m_fillColor[1], m_fillColor[2],
                    m_fillColor[3] });
                doll.rendererId = app->m_renderer->registerObject(
                std::make_unique<Object2D>(doll.getVertices(), std::vector<unsigned int>{}));
                app->registerObject(doll);
            }
            break;
        case 2:
            if (ImGui::Button("Generer Lettre A")) {
                auto letter = Vect::Shape::LetterA();
                letter.setColor({ m_fillColor[0], m_fillColor[1],
                    m_fillColor[2], m_fillColor[3] });
                letter.rendererId = app->m_renderer->registerObject(
                std::make_unique<Object2D>(letter.getVertices(), std::vector<unsigned int>{}));
                app->registerObject(letter);
            }
            break;
        default:
            break;
    }
}

/**
 * @brief Rendu des widgets du mode primitive
 * @param app Référence à l'app
 */
void UIDrawer::renderUIPrimitive(App *app)
{
    const char *primitives[] = { "Ligne", "Triangle", "Carre", "Rectangle",
        "Polygone", "Cercle", "Ellipse", "Point" };

    std::vector<float> vertices;

    ImGui::ListBox("Select primitive", &m_currentPrimitiveIndex, primitives,
        IM_ARRAYSIZE(primitives));
    ImGui::Text("Propriétés");
    ImGui::SliderFloat("Taille Contour", &m_outlineWidth, 0.0f, 1.0f);
    ImGui::Checkbox("Remplissage", &m_fill);

    switch (m_currentPrimitiveIndex) {
        case 0:
            ImGui::InputFloat2("Position point A", m_line_pointA);
            ImGui::InputFloat2("Position point B", m_line_pointB);
            ImGui::Separator();

            if (ImGui::Button("Generer Ligne")) {
                auto p = instanciatePrimitiveWAttributes<
                    Vect::Primitive::StraightLine>(
                    glm::vec2(m_line_pointA[0], m_line_pointA[1]),
                    glm::vec2(m_line_pointB[0], m_line_pointB[1]),
                    m_outlineWidth);
                p.rendererId = app->m_renderer->registerObject(
                std::make_unique<Object2D>(p.getVertices(), std::vector<unsigned int>{}));
                app->registerObject(p);
            }
            break;
        case 1:
            if (ImGui::Button("Generer Triangle")) {
                auto p = instanciatePrimitiveWAttributes<
                    Vect::Primitive::Triangle>();
                p.rendererId = app->m_renderer->registerObject(
                std::make_unique<Object2D>(p.getVertices(), std::vector<unsigned int>{}));
                app->registerObject(p);
            }
            break;
        case 2:
            if (ImGui::Button("Carre")) {
                ImGui::SliderFloat("Taille", &m_circle_radius, 0.0f, 1.0f);
                auto p
                    = instanciatePrimitiveWAttributes<Vect::Primitive::Square>(
                        m_circle_radius);
                p.rendererId = app->m_renderer->registerObject(
                std::make_unique<Object2D>(p.getVertices(), std::vector<unsigned int>{}));
                app->registerObject(p);
            }
            break;
        case 3:
            ImGui::SliderFloat2("Taille", m_localScale, 0.0f, 1.0f);
            if (ImGui::Button("Generer Rectangle")) {
                auto p = instanciatePrimitiveWAttributes<
                    Vect::Primitive::Rectangle>(
                    glm::vec2(m_localScale[0], m_localScale[1]));
                p.rendererId = app->m_renderer->registerObject(
                std::make_unique<Object2D>(p.getVertices(), std::vector<unsigned int>{}));
                app->registerObject(p);
            }
            break;
        case 4:
            ImGui::SliderInt("Nb segments", &m_input_segments, 4, 100);
            if (ImGui::Button("Generer Polygone")) {
                auto p = instanciatePrimitiveWAttributes<
                    Vect::Primitive::RegularPolygon>(m_input_segments);
                p.rendererId = app->m_renderer->registerObject(
                std::make_unique<Object2D>(p.getVertices(), std::vector<unsigned int>{}));
                app->registerObject(p);
            }
            break;
        case 5:
            ImGui::SliderFloat("Rayon", &m_circle_radius, 0.0f, 1.0f);
            if (ImGui::Button("Generer Cercle")) {
                auto p
                    = instanciatePrimitiveWAttributes<Vect::Primitive::Circle>(
                        m_circle_radius);
                p.rendererId = app->m_renderer->registerObject(
                std::make_unique<Object2D>(p.getVertices(), std::vector<unsigned int>{}));
                app->registerObject(p);
            }
            break;
        case 6:
            ImGui::SliderFloat2("Taille", m_localScale, 0.0f, 1.0f);
            if (ImGui::Button("Generer Ellipse")) {
                auto p = instanciatePrimitiveWAttributes<
                    Vect::Primitive::Ellipse>(
                    glm::vec2(m_localScale[0], m_localScale[1]));
                p.rendererId = app->m_renderer->registerObject(
                std::make_unique<Object2D>(p.getVertices(), std::vector<unsigned int>{}));
                app->registerObject(p);
            }
            break;
        case 7:
            if (ImGui::Button("Generer Point")) {
                m_fill = true;
                auto p
                    = instanciatePrimitiveWAttributes<Vect::Primitive::Point>(
                        0.1f);
                p.rendererId = app->m_renderer->registerObject(
                    std::make_unique<Object2D>(p.getVertices(), std::vector<unsigned int>{}));
                app->registerObject(p);
            }
            break;
        default:
            break;
    }
}

void UIDrawer::renderUI(App *app)
{
    ImGui::Begin("Dessin vectoriel");
    ImGui::Separator();
    ImGui::Text("Primitives");

    // Selection du mode (Primitive ou Forme)
    ImGui::RadioButton("Mode Primitive", &m_uiMode, 0);
    ImGui::RadioButton("Mode Forme", &m_uiMode, 1);
    ImGui::Separator();

    // Sélecteurs de couleurs
    ImGui::Text("Couleurs");
    ImGui::ColorEdit4("Couleur Contour", m_outlineColor);
    ImGui::ColorEdit4("Couleur Remplissage", m_fillColor);
    ImGui::Separator();

    // Rendu des widgets d'attributs du mode correspondant
    if (m_uiMode == 0) {
        renderUIPrimitive(app);
    } else {
        renderUIShape(app);
    }
    ImGui::End();
}

UIDrawer::~UIDrawer() {}

void UIDrawer::setCurrentColorRGBA(
    const glm::vec4 &rgba, bool applyFill, bool applyOutline)
{
    if (applyOutline) {
        m_outlineColor[0] = rgba.x;
        m_outlineColor[1] = rgba.y;
        m_outlineColor[2] = rgba.z;
        m_outlineColor[3] = rgba.w;
    }
    if (applyFill) {
        m_fillColor[0] = rgba.x;
        m_fillColor[1] = rgba.y;
        m_fillColor[2] = rgba.z;
        m_fillColor[3] = rgba.w;
    }
}