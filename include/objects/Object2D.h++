//
// Created by clmonn on 11/13/25.
//

#ifndef SCENELAB_OBJECT2D_H
#define SCENELAB_OBJECT2D_H

#include "RenderableObject.h++"

class Object2D : public RenderableObject {
    public:
    Object2D(const std::vector<float> &vertices, const std::vector<unsigned int> &indices, int textureHandle = -1);

    void draw(const ShaderProgram &vectorial, const ShaderProgram &pointLight, const ShaderProgram &lighting, const TextureLibrary &textures) const override;

    private:
    void init(const std::vector<float> &vertices, const std::vector<unsigned int> &indices);
};

#endif // SCENELAB_OBJECT2D_H
