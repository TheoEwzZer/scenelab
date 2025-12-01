//
// Created by clmonn on 11/27/25.
//

#ifndef SCENELAB_CURVE_H
#define SCENELAB_CURVE_H

#include "RenderableObject.hpp"

class DynamicLine : public RenderableObject {
public:
    explicit DynamicLine(glm::vec3 color = glm::vec3(1.0f),
        GLenum primitiveMode = GL_LINE_STRIP);

    void setPrimitiveMode(GLenum mode) { m_primitiveMode = mode; }

    [[nodiscard]] GLenum getPrimitiveMode() const { return m_primitiveMode; }

    void draw(const ShaderProgram &vectorial, const ShaderProgram &pointLight,
        const ShaderProgram &lighting,
        const TextureLibrary &textures) const override;

private:
    GLenum m_primitiveMode = GL_LINE_STRIP;
};

#endif // SCENELAB_CURVE_H
