//
// Created by clmonn on 11/13/25.
//

#include "../../include/objects/Object3D.hpp"
#include "objects/Material.hpp"

Object3D::Object3D(const std::vector<Vertex> &vertices,
    const std::vector<unsigned int> &indices, const glm::vec3 &color)
{
    init(vertices, indices);
    setColor(color);
}

Object3D::Object3D(const std::vector<Vertex> &vertices,
    const std::vector<unsigned int> &indices, const int textureHandle)
{
    init(vertices, indices);
    this->m_textureHandle = textureHandle;
}

void Object3D::init(const std::vector<Vertex> &vertices,
    const std::vector<unsigned int> &indices)
{
    m_vertices.reserve(vertices.size() * 14);
    for (const auto &v : vertices) {
        m_vertices.push_back(v.position.x);
        m_vertices.push_back(v.position.y);
        m_vertices.push_back(v.position.z);
        m_vertices.push_back(v.texCoord.x);
        m_vertices.push_back(v.texCoord.y);
        m_vertices.push_back(v.normal.x);
        m_vertices.push_back(v.normal.y);
        m_vertices.push_back(v.normal.z);
        m_vertices.push_back(v.tangent.x);
        m_vertices.push_back(v.tangent.y);
        m_vertices.push_back(v.tangent.z);
        m_vertices.push_back(v.bitangent.x);
        m_vertices.push_back(v.bitangent.y);
        m_vertices.push_back(v.bitangent.z);
    }
    m_indices = indices;
    indexCount = vertices.size();

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    useIndices = !indices.empty();
    glBufferData(GL_ARRAY_BUFFER, indexCount * sizeof(Vertex), vertices.data(),
        GL_STATIC_DRAW);

    if (useIndices) {
        glGenBuffers(1, &EBO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER,
            indices.size() * sizeof(unsigned int), indices.data(),
            GL_STATIC_DRAW);
        indexCount = static_cast<unsigned int>(indices.size());
        useIndices = true;
    }

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
        (void *)offsetof(Vertex, position));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
        (void *)offsetof(Vertex, texCoord));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
        (void *)offsetof(Vertex, normal));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
        (void *)offsetof(Vertex, tangent));
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
        (void *)offsetof(Vertex, bitangent));
    glEnableVertexAttribArray(4);
    glBindVertexArray(0);

    isActive = true;
}

void Object3D::draw([[maybe_unused]] const ShaderProgram &vectorial,
    [[maybe_unused]] const ShaderProgram &pointLight,
    const ShaderProgram &lighting, const TextureLibrary &textures) const
{
    const TextureResource *texture
        = textures.getTextureResource(m_textureHandle);

    const NormalMapResource *normalMap
        = textures.getNormalMapResource(m_normalMapHandler);

    lighting.use();
    lighting.setMat4("model", modelMatrix);
    const bool useTexture = this->m_useTexture && texture
        && texture->target == TextureTarget::Texture2D;
    lighting.setBool("useTexture", useTexture);

    const bool useNormalMap = this->m_useNormalMap && normalMap;
    lighting.setBool("useNormalMap", useNormalMap);

    m_mat.setShaderUniforms(lighting);
    lighting.setInt("filterMode", static_cast<int>(filterMode));
    const glm::vec2 texelSize = useTexture
        ? glm::vec2(1.0f / static_cast<float>(texture->size.x),
            1.0f / static_cast<float>(texture->size.y))
        : glm::vec2(0.0f);
    lighting.setVec2("texelSize", texelSize);

    glActiveTexture(GL_TEXTURE0);
    if (texture && texture->target == TextureTarget::Texture2D) {
        glBindTexture(GL_TEXTURE_2D, texture->id);
    } else {
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    glActiveTexture(GL_TEXTURE1);
    if (normalMap) {
        glBindTexture(GL_TEXTURE_2D, normalMap->id);
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
