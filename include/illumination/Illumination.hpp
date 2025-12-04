#pragma once

#include <cstdint>
#include <glm/fwd.hpp>
#include <glm/glm.hpp>
#include <memory>
#include <string>
#include <vector>
#include "GeometryGenerator.hpp"
#include "TransformManager.hpp"
#include "objects/Light.hpp"
#include "GameObject.hpp"
#include "renderer/implementation/RasterizationRenderer.hpp"

class App;

namespace Illumination {
/**
 * @brief Classe de l'interface graphique du dessin vectoriel
 */
class UIIllumination {
public:
    UIIllumination(TransformManager &tref, SceneGraph &sceneGraph);
    ~UIIllumination();

    void renderLightUI(App *app);
    void renderMaterialUI(App *app);

    /**
     * @brief Fonction de rendu de l'interface graphique.
     * @param app Pointeur sur l'application
     */
    void renderUI(App *app);

    void addLightToScene(
        std::unique_ptr<Light> &light, App *app, const GData &lightGeometry);

protected:
    // Classic Phong materials
    const std::array<Material, 8> materials = {
        Material(glm::vec3(0.33, 0.22, 0.03), glm::vec3(0.78, 0.57, 0.11),
            glm::vec3(0.99, 0.94, 0.81), glm::vec3(0, 0, 0), 28.0f),
        Material(glm::vec3(0.21, 0.13, 0.05), glm::vec3(0.71, 0.43, 0.18),
            glm::vec3(0.39, 0.27, 0.17), glm::vec3(0, 0, 0), 26.0f),
        Material(glm::vec3(0.25, 0.20, 0.07), glm::vec3(0.75, 0.61, 0.23),
            glm::vec3(0.63, 0.56, 0.37), glm::vec3(0, 0, 0), 50.0f),
        Material(glm::vec3(0.23, 0.23, 0.23), glm::vec3(0.51, 0.51, 0.51),
            glm::vec3(0.5, 0.5, 0.5), glm::vec3(0, 0, 0), 51.0f),
        Material(glm::vec3(0.19, 0.07, 0.02), glm::vec3(0.70, 0.27, 0.08),
            glm::vec3(0.26, 0.14, 0.09), glm::vec3(0, 0, 0), 12.8f),
        Material(glm::vec3(0.2, 0.2, 0.2), glm::vec3(0.5, 0.5, 0.5),
            glm::vec3(0.0, 0.0, 0.0), glm::vec3(0, 0, 0), 1.0f),
        Material(glm::vec3(0.2, 0.2, 0.2), glm::vec3(0.4, 0.4, 0.4),
            glm::vec3(0.77, 0.77, 0.77), glm::vec3(0, 0, 0), 76.8f),
        Material(glm::vec3(0.2, 0.2, 0.2), glm::vec3(0.8, 0.8, 0.8),
            glm::vec3(0.9, 0.9, 0.9), glm::vec3(0, 0, 0), 45.0f)
    };

    // PBR materials (albedo, metallic, roughness, ao)
    struct PBRPreset {
        glm::vec3 albedo;
        float metallic;
        float roughness;
        float ao;
    };

    const std::array<PBRPreset, 5> pbrMaterials = { {
        { { 1.0f, 0.765f, 0.336f }, 1.0f, 0.3f, 1.0f }, // Gold
        { { 0.955f, 0.637f, 0.538f }, 1.0f, 0.4f, 1.0f }, // Copper
        { { 0.56f, 0.57f, 0.58f }, 1.0f, 0.6f, 1.0f }, // Iron
        { { 0.8f, 0.1f, 0.1f }, 0.0f, 0.4f, 1.0f }, // Plastic Red
        { { 0.1f, 0.1f, 0.1f }, 0.0f, 0.9f, 1.0f } // Rubber
    } };

    int m_illumination_model = 0;
    int m_mat_preset = 0;
    float m_ambient_color[3] = { 0.1, 0.1, 0.1 };
    float m_diffuse_color[3] = { 0.3, 0.3, 0.3 };
    float m_specular_color[3] = { 0.5, 0.5, 0.5 };
    float m_emissive_color[3] = { 0, 0, 0 };
    float m_shininess = 10.0f;
    float m_percentSpecular = 0.5f;
    float m_refractionChance = 0.5f;
    float m_indexOfRefraction = 1.5f;

    // PBR properties
    int m_pbr_preset = 0;
    float m_metallic = 0.0f;
    float m_roughness = 0.5f;
    float m_ao = 1.0f;

    float m_kc = 0.5f;
    float m_kl = 0.09f;
    float m_kq = 0.03f;
    float m_p = 10.0f;
    float m_intensity = 1.0f;
    float m_ambient_light_color[3] = { 0.1, 0.1, 0.1 };
    float m_light_color[3] = { 0.3, 0.3, 0.3 };

    TransformManager &m_tref;
    SceneGraph &m_sceneGraph;

    // Persisted UI state
};

}
