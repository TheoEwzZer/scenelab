//
// Created by clmonn on 11/13/25.
//

#ifndef SCENELAB_OBJECT2D_H
#define SCENELAB_OBJECT2D_H

#include "RenderableObject.hpp"

class Object2D : public RenderableObject {
public:
    Object2D(const std::vector<float> &vertices,
        const std::vector<unsigned int> &indices, int textureHandle = -1);

    void setDrawMode(GLenum mode);

    void draw(const ShaderProgram &vectorial,
        [[maybe_unused]] const ShaderProgram &pointLight,
        [[maybe_unused]] const ShaderProgram &lighting,
        const TextureLibrary &textures) const override;

private:
    GLenum m_drawMode = GL_TRIANGLES;
    void init(const std::vector<float> &vertices,
        const std::vector<unsigned int> &indices);
};

#endif // SCENELAB_OBJECT2D_H
