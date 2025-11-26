#pragma once

#include <cstdint>
#include <glm/fwd.hpp>
#include <glm/glm.hpp>
#include <memory>
#include <string>
#include <vector>
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
    UIIllumination();
    ~UIIllumination();

    /**
     * @brief Fonction de rendu de l'interface graphique.
     * @param app Pointeur sur l'application
     */
    void renderUI(App *app);

    void setCurrentColorRGBA(const glm::vec4 &rgba, bool applyFill = true,
        bool applyOutline = true);

protected:
    void renderUIIllumination(App *app);

    int m_illumination_model = 0;

    // Persisted UI state

};

}
