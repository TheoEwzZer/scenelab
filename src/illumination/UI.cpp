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
        = app->m_rasterRenderer->registerObject(std::move(light), "");
    lightNode->getData().setAABB(
        lightGeometry.aabbCorner1, lightGeometry.aabbCorner2);
    lightNode->getData().setPosition(glm::vec3(3.0f, 3.0f, 3.0f));
    lightNode->getData().setScale(glm::vec3(0.2f));
    m_sceneGraph.getRoot()->addChild(std::move(lightNode));
}

void UIIllumination::renderUI(App *app)
{

    if (app->m_rasterRenderer != nullptr) {

        static const char *models[]
            = { "Lambert", "Phong", "Blinn-Phong", "Gouraud" };

        ImGui::Begin("Illumination");

        ImGui::Separator();
        ImGui::Text("Scene illumination");
        if (ImGui::Combo("Model", &m_illumination_model, models,
                IM_ARRAYSIZE(models))) {
            app->m_rasterRenderer->setLightingModel(
                static_cast<ARenderer::LightingModel>(m_illumination_model));
        }

        ImGui::Separator();
        ImGui::Text("Material");

        ImGui::ColorEdit3("Ambient Color", m_ambient_color);
        ImGui::ColorEdit3("Diffuse Color", m_diffuse_color);
        ImGui::ColorEdit3("Specular Color", m_specular_color);
        ImGui::ColorEdit3("Emissive Color", m_emissive_color);
        ImGui::SliderFloat("Shininess", &m_shininess, 1.0f, 150.0f);

        if (ImGui::Button("Apply Material")) {
            for (auto *node : m_tref.getSelectedNodes()) {
                if (!node) {
                    continue;
                }
                int rendererId = node->getData().rendererId;
                if (rendererId >= 0) {
                    RasterizationRenderer *rend
                        = dynamic_cast<RasterizationRenderer *>(
                            app->m_renderer.get());
                    if (rend != nullptr) {
                        Material mat(
                            glm::vec3(m_ambient_color[0], m_ambient_color[1],
                                m_ambient_color[2]),
                            glm::vec3(m_diffuse_color[0], m_diffuse_color[1],
                                m_diffuse_color[2]),
                            glm::vec3(m_specular_color[0], m_specular_color[1],
                                m_specular_color[2]),
                            glm::vec3(m_emissive_color[0], m_emissive_color[1],
                                m_emissive_color[2]),
                            m_shininess);
                        rend->assignMaterialToObject(rendererId, mat);
                    }
                }
            }
        }

        ImGui::Separator();
        ImGui::Text("Light");
        ImGui::ColorEdit3("Ambient Light Color", m_ambient_light_color);

        ImGui::Separator();
        ImGui::Text("Dynamic Lights");
        ImGui::ColorEdit3("Light Color", m_light_color);
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
                    RasterizationRenderer *rend
                        = dynamic_cast<RasterizationRenderer *>(
                            app->m_renderer.get());
                    if (rend != nullptr) {
                        Light *l = dynamic_cast<Light *>(
                            &rend->getRenderable(rendererId));
                        if (l != nullptr) {
                            switch (l->getType()) {
                                case Light::Point:
                                    l->setPoint(glm::vec3(m_light_color[0],
                                                    m_light_color[1],
                                                    m_light_color[2]),
                                        m_kc, m_kl, m_kq);
                                    break;
                                case Light::Directional:
                                    l->setDirectional(glm::vec3(
                                        m_light_color[0], m_light_color[1],
                                        m_light_color[2]));
                                    break;
                                case Light::Spot:
                                    l->setSpot(glm::vec3(m_light_color[0],
                                                   m_light_color[1],
                                                   m_light_color[2]),
                                        m_kc, m_kl, m_kq, m_p);
                                    break;
                                default:
                                    break;
                            }
                        }
                    }
                }
            }
        }

        app->m_rasterRenderer->m_ambientLightColor
        = glm::vec3(m_ambient_light_color[0], m_ambient_light_color[1],
            m_ambient_light_color[2]);

            ImGui::Separator();
            ImGui::Text("Create Lights");
        if (ImGui::Button("Spawn Directional")) {
            const GData lightGeometry
                = GeometryGenerator::generateSphere(0.5f, 16, 16);
            auto light = std::make_unique<Light>(
                lightGeometry.vertices, std::vector<unsigned int> {});
            light->setDirectional(glm::vec3(
                m_light_color[0], m_light_color[1], m_light_color[2]));
            addLightToScene(light, app, lightGeometry);
        }
        if (ImGui::Button("Spawn Point")) {
            const GData lightGeometry
                = GeometryGenerator::generateSphere(0.5f, 16, 16);
            auto light = std::make_unique<Light>(
                lightGeometry.vertices, std::vector<unsigned int> {});
            light->setPoint(glm::vec3(m_light_color[0], m_light_color[1],
                                m_light_color[2]),
                m_kc, m_kl, m_kq);
            addLightToScene(light, app, lightGeometry);
        }
        if (ImGui::Button("Spawn Spot")) {
            const GData lightGeometry
                = GeometryGenerator::generateSphere(0.5f, 16, 16);
            auto light = std::make_unique<Light>(
                lightGeometry.vertices, std::vector<unsigned int> {});
            light->setSpot(glm::vec3(m_light_color[0], m_light_color[1],
                               m_light_color[2]),
                m_kc, m_kl, m_kq, m_p);
            addLightToScene(light, app, lightGeometry);
        }
    }

    ImGui::End();
}

UIIllumination::~UIIllumination() {}
