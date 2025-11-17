//
// Created by clmonn on 11/13/25.
//

#include "../../include/objects/Light.hpp"

Light::Light(const std::vector<float> &vertices,
    const std::vector<unsigned int> &indices, const glm::vec3 &color)
{
    init(vertices, indices);
    m_color = color;
}

Light::Light(const std::vector<float> &vertices,
    const std::vector<unsigned int> &indices, const int textureHandle)
{
    init(vertices, indices);
    this->m_textureHandle = textureHandle;
}

void Light::init(const std::vector<float> &vertices,
    const std::vector<unsigned int> &indices)
{
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    std::vector<float> processedVertices;
    useIndices = !indices.empty();
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float),
        vertices.data(), GL_STATIC_DRAW);

    if (useIndices) {
        glGenBuffers(1, &EBO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER,
            indices.size() * sizeof(unsigned int), indices.data(),
            GL_STATIC_DRAW);
        indexCount = static_cast<unsigned int>(indices.size());
        useIndices = true;
    } else {
        indexCount = static_cast<unsigned int>(vertices.size() / 8);
    }

    glVertexAttribPointer(
        0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float),
        (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float),
        (void *)(5 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glBindVertexArray(0);

    isActive = true;
}

void Light::useShader(ShaderProgram &shader) const
{
    shader.setVec3("lightPos", glm::vec3(modelMatrix[3]));
}

void Light::draw([[maybe_unused]] const ShaderProgram &vectorial,
    [[maybe_unused]] const ShaderProgram &pointLight,
    const ShaderProgram &lighting, const TextureLibrary &textures) const
{
    const TextureResource *texture
        = textures.getTextureResource(m_textureHandle);

    lighting.use();
    lighting.setMat4("model", modelMatrix);

    if (texture) {
        if (texture->target == TextureTarget::Texture2D) {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, texture->id);
        }
    } else {
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    glBindVertexArray(VAO);
    if (!useIndices) {
        glDrawArrays(GL_TRIANGLES, 0, indexCount);
    } else {
        glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
    }
    glBindVertexArray(0);
}
