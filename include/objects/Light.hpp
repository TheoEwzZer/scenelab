//
// Created by clmonn on 11/13/25.
//

#ifndef SCENELAB_LIGHT_H
#define SCENELAB_LIGHT_H

#include "RenderableObject.hpp"

class Light : public RenderableObject {
public:
    Light(const std::vector<float> &vertices,
        const std::vector<unsigned int> &indices, int textureHandle = -1);
    Light(const std::vector<float> &vertices,
        const std::vector<unsigned int> &indices, const glm::vec3 &color);

    void useShader(ShaderProgram &shader) const override;

    void draw([[maybe_unused]] const ShaderProgram &vectorial,
        [[maybe_unused]] const ShaderProgram &pointLight,
        const ShaderProgram &lighting,
        const TextureLibrary &textures) const override;

private:
    void init(const std::vector<float> &vertices,
        const std::vector<unsigned int> &indices);
};

#endif // SCENELAB_LIGHT_H
