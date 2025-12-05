#include "App.hpp"
#include "SceneGraph.hpp"
#include "illumination/Illumination.hpp"
#include "imgui.h"
#include "objects/Light.hpp"
#include "objects/Material.hpp"
#include "renderer/implementation/PathTracingRenderer.hpp"
#include "renderer/implementation/RasterizationRenderer.hpp"
#include "renderer/interface/IRenderer.hpp"
#include <glm/fwd.hpp>
#include <glm/glm.hpp>

using namespace Illumination;

UIIllumination::UIIllumination(
    TransformManager &tref, SceneGraph &sceneGraph) :
    m_illumination_model(0),
    m_tref(tref), m_sceneGraph(sceneGraph)
{
}

void UIIllumination::addLightToScene(
    std::unique_ptr<Light> &light, App *app, const GData &lightGeometry)
{
    (void)lightGeometry;
    std::unique_ptr<SceneGraph::Node> lightNode
        = std::make_unique<SceneGraph::Node>();
    lightNode->getData().setName(light->getNameStr());
    lightNode->getData().rendererId
        = app->m_renderer->registerObject(std::move(light), "");
    lightNode->getData().setPosition(glm::vec3(3.0f, 3.0f, 3.0f));
    lightNode->getData().setScale(glm::vec3(0.2f));
    m_sceneGraph.getRoot()->addChild(std::move(lightNode));
}

static void setFloat3(float *target_float_3, const glm::vec3 &v)
{
    target_float_3[0] = v[0];
    target_float_3[1] = v[1];
    target_float_3[2] = v[2];
}

void UIIllumination::renderLightUI(App *app)
{
    static const char *models[]
        = { "Lambert", "Phong", "Blinn-Phong", "Gouraud",  "PBR" };

    auto rend = dynamic_cast<PathTracingRenderer *>(app->m_renderer.get());
    auto *rasterRenderer = dynamic_cast<RasterizationRenderer *>(app->m_renderer.get());

    ImGui::Begin("Illumination");
    ImGui::Text("Scene illumination");
    if (!rend) {
        if (rasterRenderer != nullptr) {
            ImGui::Separator();
            if (ImGui::Combo("Model", &m_illumination_model, models,
                    IM_ARRAYSIZE(models))) {
                app->m_renderer->setLightingModel(
                    static_cast<IRenderer::LightingModel>(m_illumination_model));
            }
        }
            rasterRenderer->setLightingModel(
                static_cast<IRenderer::LightingModel>(m_illumination_model));
            // Deferred Rendering toggle
            bool useDeferred = rasterRenderer->getUseDeferredRendering();
            if (ImGui::Checkbox("Deferred Rendering", &useDeferred)) {
                rasterRenderer->setUseDeferredRendering(useDeferred);
            }
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("Enable G-Buffer based deferred rendering");
            }

            // IBL toggle - only shown in PBR mode
            if (m_illumination_model == 4) {
                bool useIBL = rasterRenderer->getUseIBL();
                if (ImGui::Checkbox("Use IBL (Environment Lighting)", &useIBL)) {
                    rasterRenderer->setUseIBL(useIBL);
                }
                if (ImGui::IsItemHovered()) {
                    ImGui::SetTooltip(
                        "Use Image-Based Lighting from the active cubemap/HDR");
                }
                if (useIBL && ImGui::Button("Regenerate IBL")) {
                    rasterRenderer->generateIBLFromCurrentCubemap();
                }
            }
    }

    ImGui::Separator();
    ImGui::Text("Light");
    if (!rend) {
        ImGui::ColorEdit3("Ambient Light Color", m_ambient_light_color);
    }
    ImGui::Separator();
    ImGui::Text("Dynamic Lights");
    ImGui::ColorEdit3("Light Color", m_light_color);
    ImGui::SliderFloat("Light Intensity", &m_intensity, 0.0f, 10.0f);
    if (!rend) {
        ImGui::SliderFloat("Constant Attenuation", &m_kc, 0.0f, 1.0f);
        ImGui::SliderFloat("Linear Attenuation", &m_kl, 0.0f, 1.0f);
        ImGui::SliderFloat("Quadratic Attenuation", &m_kq, 0.0f, 1.0f);
        ImGui::SliderFloat("Attenuation Exponent", &m_p, 0.0f, 100.0f);
    }
    if (ImGui::Button("Apply Light Parameters")) {
        for (auto *node : m_tref.getSelectedNodes()) {
            if (!node) {
                continue;
            }
            int rendererId = node->getData().rendererId;
            if (rendererId >= 0) {
                Light *l = dynamic_cast<Light *>(
                    &app->m_renderer->getRenderable(rendererId));
                if (l != nullptr) {
                    switch (l->getType()) {
                        case Light::Point:
                            l->setPoint(
                                glm::vec3(m_light_color[0], m_light_color[1],
                                    m_light_color[2]),
                                m_kc, m_kl, m_kq, m_intensity);
                            break;
                        case Light::Directional:
                            l->setDirectional(
                                glm::vec3(m_light_color[0], m_light_color[1],
                                    m_light_color[2]),
                                m_intensity);
                            break;
                        case Light::Spot:
                            l->setSpot(glm::vec3(m_light_color[0],
                                           m_light_color[1], m_light_color[2]),
                                m_kc, m_kl, m_kq, m_p, m_intensity);
                            break;
                        default:
                            break;
                    }
                }
            }
        }
        if (rend) {
            rend->refresh();
        }
    }

    app->m_renderer->setAmbientLight(glm::vec3(m_ambient_light_color[0],
        m_ambient_light_color[1], m_ambient_light_color[2]));

    ImGui::Separator();
    ImGui::Text("Create Lights");
    if (!rend) {
        if (ImGui::Button("Spawn Point")) {
            auto light = std::make_unique<Light>();
            light->setPoint(
                glm::vec3(m_light_color[0], m_light_color[1], m_light_color[2]),
                m_kc, m_kl, m_kq);
            addLightToScene(light, app, light->getGData());
        }
        if (ImGui::Button("Spawn Directional")) {
            auto light = std::make_unique<Light>();
            light->setDirectional(glm::vec3(
                m_light_color[0], m_light_color[1], m_light_color[2]));
            addLightToScene(light, app, light->getGData());
        }
        if (ImGui::Button("Spawn Spot")) {
            auto light = std::make_unique<Light>();
            light->setSpot(glm::vec3(m_light_color[0], m_light_color[1],
                               m_light_color[2]),
                m_kc, m_kl, m_kq, m_p);
            addLightToScene(light, app, light->getGData());
        }
    } else {
        ImGui::Text("Please switch to raster renderer to add more lights");
        ImGui::Text("Pathtracer: only point lights");
    }

    ImGui::End();
}

void UIIllumination::renderMaterialUI(App *app)
{
    static const char *materials_preset[] = { "Brass", "Bronze", "Gold",
        "Silver", "Copper", "Plastic", "Chrome", "Ceramic" };
    static const char *pbr_preset[]
        = { "Gold", "Copper", "Iron", "Plastic Red", "Rubber" };

    ImGui::Begin("Material");
    ImGui::Text("Material Editor");
    if (m_illumination_model != 4) {
        if (ImGui::Combo("Preset", &m_mat_preset, materials_preset,
                IM_ARRAYSIZE(materials_preset))) {
            setFloat3(m_ambient_color, materials[m_mat_preset].m_ambientColor);
            setFloat3(m_diffuse_color, materials[m_mat_preset].m_diffuseColor);
            setFloat3(m_specular_color, materials[m_mat_preset].m_specularColor);
            setFloat3(m_emissive_color, materials[m_mat_preset].m_emissiveColor);
            m_indexOfRefraction = materials[m_mat_preset].m_indexOfRefraction;
            m_refractionChance = materials[m_mat_preset].m_refractionChance;
            m_roughness = materials[m_mat_preset].m_roughness;
            m_percentSpecular = materials[m_mat_preset].m_percentSpecular;
            m_shininess = materials[m_mat_preset].m_shininess;
        }
    } else {
        if (ImGui::Combo("PBR Preset", &m_pbr_preset, pbr_preset,
                IM_ARRAYSIZE(pbr_preset))) {
            const auto &preset = pbrMaterials[m_pbr_preset];
            setFloat3(m_diffuse_color, preset.albedo);
            m_metallic = preset.metallic;
            m_roughness = preset.roughness;
            m_ao = preset.ao;
        }
    }

    ImGui::ColorEdit3("Ambient Color", m_ambient_color);
    ImGui::ColorEdit3("Diffuse Color", m_diffuse_color);
    ImGui::ColorEdit3("Specular Color", m_specular_color);
    ImGui::ColorEdit3("Emissive Color", m_emissive_color);
    ImGui::SliderFloat("Shininess", &m_shininess, 1.0f, 150.0f);

    // PBR Material section (shown when PBR mode is active)
    if (m_illumination_model == 4) { // PBR mode
        ImGui::Separator();
        ImGui::ColorEdit3("Albedo", m_diffuse_color);
        ImGui::SliderFloat("Metallic", &m_metallic, 0.0f, 1.0f);
        ImGui::SliderFloat("Roughness", &m_roughness, 0.0f, 1.0f);
        ImGui::SliderFloat("Ambient Occlusion", &m_ao, 0.0f, 1.0f);
    }

    ImGui::Separator();
    ImGui::Text("- Path tracing only -");
    ImGui::SliderFloat("Percent Specular", &m_percentSpecular, 0.0f, 1.0f);
    ImGui::SliderFloat("Index Of Refraction", &m_indexOfRefraction, 1.0f, 4.0f);
    ImGui::SliderFloat("Refraction Chance", &m_refractionChance, 0.0f, 1.0f);

    if (ImGui::Button("Apply Material")) {
        for (auto *node : m_tref.getSelectedNodes()) {
            if (!node) {
                continue;
            }
            int rendererId = node->getData().rendererId;
            if (rendererId >= 0) {
                Material mat(glm::vec3(m_ambient_color[0], m_ambient_color[1],
                                 m_ambient_color[2]),
                    glm::vec3(m_diffuse_color[0], m_diffuse_color[1],
                        m_diffuse_color[2]),
                    glm::vec3(m_specular_color[0], m_specular_color[1],
                        m_specular_color[2]),
                    glm::vec3(m_emissive_color[0], m_emissive_color[1],
                        m_emissive_color[2]),
                    m_shininess, m_roughness, m_percentSpecular,
                    m_indexOfRefraction, m_refractionChance);

                // Set PBR properties
                mat.m_metallic = m_metallic;
                mat.m_roughness = m_roughness;
                mat.m_ao = m_ao;
                mat.m_usePBR = (m_illumination_model == 4);
                app->m_renderer->setObjectMaterial(rendererId, mat);
            }
        }
    }
    ImGui::End();
}

void UIIllumination::renderUI(App *app)
{
    renderLightUI(app);
    renderMaterialUI(app);
}

UIIllumination::~UIIllumination() {}
