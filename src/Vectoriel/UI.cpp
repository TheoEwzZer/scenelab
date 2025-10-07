#include "App.hpp"
#include "Vectoriel.hpp"
#include "imgui.h"
#include <glm/fwd.hpp>

using namespace Vect;

UIDrawer::UIDrawer() :
    m_outlineWidth(0.1f), m_outlineColor(0, 0, 0, 255),
    m_fillColor(0, 0, 0, 255), m_localScale(1.0f, 1.0f),
    m_localPosition(0.0f, 0.0f), m_localRotation(0), m_fill(false)
{
}

void UIDrawer::renderUI(App *app)
{
    ImGui::Begin("Dessin vectoriel");

    ImGui::Text("Transformations");

    ImGui::InputFloat2("Position Locale", m_localPosition);
    ImGui::InputFloat2("Proportion Locale", m_localScale);
    ImGui::InputFloat("Rotation Locale", &m_localRotation);

    ImGui::Separator();
    ImGui::Text("Primitives");

    const char *primitives[] = { "Ligne", "Triangle", "Carré", "Rectangle",
        "Polygone", "Cercle", "Ellipse", "Point" };
    static int current_primitive_idx = 0;
    ImGui::ListBox("Select primitive", &current_primitive_idx, primitives,
        IM_ARRAYSIZE(primitives));

    ImGui::Separator();

    ImGui::Text("Propriétés");
    ImGui::InputFloat("Taille Contour", &m_outlineWidth);
    ImGui::Checkbox("Remplissage", &m_fill);

    // ImGui::Separator();
    // ImGui::Text("Couleurs");
    // ImGui::ColorPicker4("Couleur Contour", m_outlineColor);
    // ImGui::ColorPicker4("Couleur Remplissage", m_fillColor);

    switch (current_primitive_idx) {
        case 0:
        ImGui::InputFloat2("Position point A", m_line_pointA);
        ImGui::InputFloat2("Position point B", m_line_pointB);
        ImGui::InputFloat("Epaisseur", &m_line_width);
        ImGui::Separator();

            if (ImGui::Button("Generer Ligne")) {
                const auto &c = instanciatePrimitiveWAttributes<
                    Vect::Primitive::StraightLine>(
                    glm::vec2(m_line_pointA[0], m_line_pointA[1]),
                    glm::vec2(m_line_pointB[0], m_line_pointB[1]),
                    m_line_width);


                // TODO, faire ça en moins crad
                        static int ct = 1;

                app->selectedObjectIndex = app->m_gameObjects[++ct].rendererId = app->m_renderer->registerObject(
                    c.generateGLVertices(), {}, "", false);
            }
            break;
        case 2:
            // ImGui::InputFloat(const char *label, float *v) default : break;
        default:
        break;
    }

    // if (ImGui::Button("Ajoute Cercle")) {
    //     const auto &c
    //         = instanciatePrimitiveWAttributes<Vect::Primitive::Circle>(3);
    //     renderer->registerObject(c.generateGLVertices(), {}, "", false);
    // }

    // if (ImGui::Button("Ajoute Rectangle")) {
    //     const auto &c
    //         = instanciatePrimitiveWAttributes<Vect::Primitive::Rectangle>();
    //     renderer->registerObject(c.generateGLVertices(), {}, "", false);
    // }

    // if (ImGui::Button("Ajoute Carre")) {
    //     const auto &c
    //         = instanciatePrimitiveWAttributes<Vect::Primitive::Square>(3);
    //     renderer->registerObject(c.generateGLVertices(), {}, "", false);
    // }

    // if (ImGui::Button("Ajoute Triangle")) {
    //     const auto &c
    //         = instanciatePrimitiveWAttributes<Vect::Primitive::Triangle>();
    //     renderer->registerObject(c.generateGLVertices(), {}, "", false);
    // }

    // if (ImGui::Button("Ajoute Polygone")) {
    //     int n = 12;
    //     const auto &c
    //         =
    //         instanciatePrimitiveWAttributes<Vect::Primitive::RegularPolygon>(12);
    //     renderer->registerObject(c.generateGLVertices(), {}, "", false);
    // }

    ImGui::End();
}

UIDrawer::~UIDrawer() {}