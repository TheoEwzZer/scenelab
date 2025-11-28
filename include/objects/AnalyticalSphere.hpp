#ifndef SCENELAB_ANALYTICALSPHERE_H
#define SCENELAB_ANALYTICALSPHERE_H

#include "RenderableObject.hpp"

class AnalyticalSphere : public RenderableObject {
public:
    AnalyticalSphere(float radius, int sectors, int stacks,
        const glm::vec3 &color = glm::vec3(1.0f));

    void draw([[maybe_unused]] const ShaderProgram &vectorial,
        [[maybe_unused]] const ShaderProgram &pointLight,
        const ShaderProgram &lighting,
        const TextureLibrary &textures) const override;

private:
    void init(float radius, int sectors, int stacks);
};

#endif // SCENELAB_ANALYTICALSPHERE_H
