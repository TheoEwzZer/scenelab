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
#include "renderer/interface/ARenderer.hpp"

class App;

namespace Illumination {
/**
 * @brief Classe de l'interface graphique du dessin vectoriel
 */
class UIIllumination {
public:
    UIIllumination(TransformManager &tref, SceneGraph &sceneGraph);
    ~UIIllumination();

    /**
     * @brief Fonction de rendu de l'interface graphique.
     * @param app Pointeur sur l'application
     */
    void renderUI(App *app);

    void addLightToScene(
    std::unique_ptr<Light> &light, App *app, const GData &lightGeometry);


protected:

    int m_illumination_model = 0;
    float m_shininess = 10.0f;
    float m_ambient_color[3] = {0.1,0.1,0.1};
    float m_diffuse_color[3] = {0.3,0.3,0.3};
    float m_specular_color[3] = {0.5,0.5,0.5};
    float m_emissive_color[3] = {0,0,0};

    float m_kc = 0.5f;
    float m_kl = 0.09f;
    float m_kq = 0.03f;
    float m_p = 10.0f;
    float m_ambient_light_color[3] = {0.1,0.1,0.1};
    float m_light_color[3] = {0.3,0.3,0.3};

    TransformManager &m_tref;
    SceneGraph &m_sceneGraph;

    // Persisted UI state

};

}
