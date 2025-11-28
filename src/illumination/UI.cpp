#include "App.hpp"
#include "SceneGraph.hpp"
#include "illumination/Illumination.hpp"
#include "imgui.h"
#include "objects/Light.hpp"
#include "objects/Material.hpp"
#include "renderer/implementation/RasterizationRenderer.hpp"
#include <glm/fwd.hpp>
#include <glm/glm.hpp>

using namespace Illumination;

UIIllumination::UIIllumination(
    TransformManager &tref, SceneGraph &sceneGraph) :
    m_illumination_model(0), m_tref(tref), m_sceneGraph(sceneGraph)
{
}

void UIIllumination::addLightToScene(
    std::unique_ptr<Light> &light, App *app, const GData &lightGeometry)
{
    std::unique_ptr<SceneGraph::Node> lightNode
        = std::make_unique<SceneGraph::Node>();
    lightNode->getData().setName(light->getNameStr());
    lightNode->getData().rendererId
        = app->m_renderer->registerObject(std::move(light), "");
    lightNode->getData().setAABB(
        lightGeometry.aabbCorner1, lightGeometry.aabbCorner2);
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

void UIIllumination::renderRasterUI(
    App *app, RasterizationRenderer *rasterRenderer)
{
    static const char *models[]
        = { "Lambert", "Phong", "Blinn-Phong", "Gouraud" };

    ImGui::Begin("Raster illumination");
    ImGui::Text("Scene illumination");

    if (ImGui::Combo(
            "Model", &m_illumination_model, models, IM_ARRAYSIZE(models))) {
        rasterRenderer->setLightingModel(
            static_cast<LightingModel>(m_illumination_model));
    }

    ImGui::Separator();
    ImGui::Text("Light");
    ImGui::ColorEdit3("Ambient Light Color", m_ambient_light_color);

    ImGui::Separator();
    ImGui::Text("Dynamic Lights");
    ImGui::ColorEdit3("Light Color", m_light_color);
    ImGui::SliderFloat("Light Intensity", &m_intensity, 0.0f, 10.0f);
    ImGui::SliderFloat("Constant Attenuation", &m_kc, 0.0f, 1.0f);
    ImGui::SliderFloat("Linear Attenuation", &m_kl, 0.0f, 1.0f);
    ImGui::SliderFloat("Quadratic Attenuation", &m_kq, 0.0f, 1.0f);
    ImGui::SliderFloat("Attenuation Exponent", &m_p, 0.0f, 100.0f);

    if (ImGui::Button("Apply Light Parameters")) {
        for (auto *node : m_tref.getSelectedNodes()) {
            if (!node) {
                continue;
            }
            int rendererId = node->getData().rendererId;
            if (rendererId >= 0) {
                Light *l = dynamic_cast<Light *>(
                    &rasterRenderer->getRenderable(rendererId));
                if (l != nullptr) {
                    switch (l->getType()) {
                        case Light::Point:
                            l->setPoint(
                                glm::vec3(m_light_color[0], m_light_color[1],
                                    m_light_color[2]),
                                m_kc, m_kl, m_kq, m_intensity);
                            break;
                        case Light::Directional:
                            l->setDirectional(glm::vec3(m_light_color[0],
                                m_light_color[1], m_light_color[2]), m_intensity);
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
    }

    rasterRenderer->m_ambientLightColor = glm::vec3(m_ambient_light_color[0],
        m_ambient_light_color[1], m_ambient_light_color[2]);

    ImGui::Separator();
    ImGui::Text("Create Lights");
    if (ImGui::Button("Spawn Directional")) {
        const GData lightGeometry
            = GeometryGenerator::generateSphere(0.5f, 16, 16);
        auto light = std::make_unique<Light>(
            lightGeometry.vertices, std::vector<unsigned int> {});
        light->setDirectional(
            glm::vec3(m_light_color[0], m_light_color[1], m_light_color[2]));
        addLightToScene(light, app, lightGeometry);
    }
    if (ImGui::Button("Spawn Point")) {
        const GData lightGeometry
            = GeometryGenerator::generateSphere(0.5f, 16, 16);
        auto light = std::make_unique<Light>(
            lightGeometry.vertices, std::vector<unsigned int> {});
        light->setPoint(
            glm::vec3(m_light_color[0], m_light_color[1], m_light_color[2]),
            m_kc, m_kl, m_kq);
        addLightToScene(light, app, lightGeometry);
    }
    if (ImGui::Button("Spawn Spot")) {
        const GData lightGeometry
            = GeometryGenerator::generateSphere(0.5f, 16, 16);
        auto light = std::make_unique<Light>(
            lightGeometry.vertices, std::vector<unsigned int> {});
        light->setSpot(
            glm::vec3(m_light_color[0], m_light_color[1], m_light_color[2]),
            m_kc, m_kl, m_kq, m_p);
        addLightToScene(light, app, lightGeometry);
    }

    ImGui::End();
}

void UIIllumination::renderMaterialUI(App *app)
{
    static const char *materials_preset[] = { "Brass", "Bronze", "Gold",
        "Silver", "Copper", "Plastic", "Chrome", "Ceramic" };

    ImGui::Begin("Material");
    ImGui::Text("Material Editor");
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

    ImGui::ColorEdit3("Ambient Color", m_ambient_color);
    ImGui::ColorEdit3("Diffuse Color", m_diffuse_color);
    ImGui::ColorEdit3("Specular Color", m_specular_color);
    ImGui::ColorEdit3("Emissive Color", m_emissive_color);
    ImGui::SliderFloat("Shininess", &m_shininess, 1.0f, 150.0f);

    ImGui::Separator();
    ImGui::Text("- Path tracing only -");
    ImGui::SliderFloat("Percent Specular", &m_percentSpecular, 0.0f, 1.0f);
    ImGui::SliderFloat("Roughness", &m_roughness, 0.0f, 1.0f);
    ImGui::SliderFloat(
        "Index Of Refraction", &m_indexOfRefraction, 1.0f, 4.0f);
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
                app->m_renderer->setObjectMaterial(rendererId, mat);
            }
        }
    }
    ImGui::End();
}

void UIIllumination::renderUI(App *app)
{
    auto *rasterRenderer
        = dynamic_cast<RasterizationRenderer *>(app->m_renderer.get());

    if (rasterRenderer != nullptr) {
        renderRasterUI(app, rasterRenderer);
    }
    renderMaterialUI(app);
}

UIIllumination::~UIIllumination() {}
