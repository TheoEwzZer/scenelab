#ifndef SCENELAB_ANALYTICALPLANE_H
#define SCENELAB_ANALYTICALPLANE_H

#include "RenderableObject.hpp"

class AnalyticalPlane : public RenderableObject {
public:
    AnalyticalPlane(float width, float height,
        const glm::vec3 &normal = glm::vec3(0.0f, 1.0f, 0.0f),
        const glm::vec3 &color = glm::vec3(1.0f));

    void draw([[maybe_unused]] const ShaderProgram &vectorial,
        [[maybe_unused]] const ShaderProgram &pointLight,
        const ShaderProgram &lighting,
        const TextureLibrary &textures) const override;

private:
    void init(float width, float height, const glm::vec3 &normal);
};

#endif // SCENELAB_ANALYTICALPLANE_H
