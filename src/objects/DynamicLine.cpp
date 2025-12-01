//
// Created by clmonn on 11/27/25.
//

#include "../../include/objects/DynamicLine.hpp"

DynamicLine::DynamicLine(const glm::vec3 color, const GLenum primitiveMode) :
    m_primitiveMode(primitiveMode)
{
    setColor(color);
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    useIndices = false;
    glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);

    glVertexAttribPointer(
        0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);

    isActive = true;
}

void DynamicLine::draw(const ShaderProgram &vectorial,
    [[maybe_unused]] const ShaderProgram &pointLight,
    [[maybe_unused]] const ShaderProgram &lighting,
    const TextureLibrary &textures) const
{
    const TextureResource *texture
        = textures.getTextureResource(m_textureHandle);

    vectorial.use();
    vectorial.setMat4("model", modelMatrix);
    const bool useTexture = this->m_useTexture && texture
        && texture->target == TextureTarget::Texture2D;
    vectorial.setBool("useTexture", useTexture);
    vectorial.setVec3("objectColor", getColor());
    vectorial.setInt("filterMode", static_cast<int>(filterMode));
    glm::vec2 texelSize = useTexture
        ? glm::vec2(1.0f / static_cast<float>(texture->size.x),
            1.0f / static_cast<float>(texture->size.y))
        : glm::vec2(0.0f);
    vectorial.setVec2("texelSize", texelSize);

    if (texture) {
        if (texture->target == TextureTarget::Texture2D) {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, texture->id);
        }
    } else {
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    glBindVertexArray(VAO);
    glDrawArrays(m_primitiveMode, 0, indexCount);
    glBindVertexArray(0);
}
