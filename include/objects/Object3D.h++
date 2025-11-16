//
// Created by clmonn on 11/13/25.
//

#ifndef SCENELAB_OBJECT3D_H
#define SCENELAB_OBJECT3D_H

#include "RenderableObject.h++"

class Object3D : public RenderableObject {
    public:
    Object3D(const std::vector<float> &vertices, const std::vector<unsigned int> &indices, int textureHandle = -1);
    Object3D(const std::vector<float> &vertices, const std::vector<unsigned int> &indices, const glm::vec3 &color);

    void draw(const ShaderProgram &vectorial, const ShaderProgram &pointLight, const ShaderProgram &lighting, const TextureLibrary& textures) const override;

    private:
    void init(const std::vector<float> &vertices, const std::vector<unsigned int> &indices);
};

#endif // SCENELAB_OBJECT3D_H
