#include "App.hpp"
#include "illumination/Illumination.hpp"
#include "imgui.h"
#include <glm/fwd.hpp>
#include <glm/glm.hpp>


using namespace Illumination;

UIIllumination::UIIllumination() :
    m_illumination_model(0)
{
}


void UIIllumination::renderUI(App *app)
{
    static const char *models[] = { "Lambert", "Phong", "Blinn-Phong", "Gouraud" };

    ImGui::Begin("Illumination");

    ImGui::Separator();
    if (ImGui::ListBox("Illumination Model", &m_illumination_model, models,
        IM_ARRAYSIZE(models))) {
        app->m_rasterRenderer->setLightingModel(static_cast<ARenderer::LightingModel>(m_illumination_model));
    }

    ImGui::End();
}

UIIllumination::~UIIllumination() {}
