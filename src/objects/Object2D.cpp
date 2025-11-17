//
// Created by clmonn on 11/13/25.
//

#include "../../include/objects/Object2D.hpp"

Object2D::Object2D(const std::vector<float> &vertices,
    const std::vector<unsigned int> &indices, const int textureHandle)
{
    init(vertices, indices);
    this->m_textureHandle = textureHandle;
}

void Object2D::init(const std::vector<float> &vertices,
    const std::vector<unsigned int> &indices)
{
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    std::vector<float> processedVertices;
    const bool hasIndices = !indices.empty();

    constexpr int originalStride = 8;
    if (vertices.size() % originalStride != 0) {
        std::cerr << "[ERROR] Invalid 2D vertex buffer size." << std::endl;
    }
    const int vertexCount = static_cast<int>(vertices.size() / originalStride);
    float minX = std::numeric_limits<float>::max();
    float maxX = std::numeric_limits<float>::lowest();
    float minY = std::numeric_limits<float>::max();
    float maxY = std::numeric_limits<float>::lowest();

    for (int i = 0; i < vertexCount; ++i) {
        const float x = vertices[i * originalStride + 0];
        const float y = vertices[i * originalStride + 1];
        minX = std::min(minX, x);
        maxX = std::max(maxX, x);
        minY = std::min(minY, y);
        maxY = std::max(maxY, y);
    }

    const float invWidth
        = (maxX - minX) > std::numeric_limits<float>::epsilon()
        ? 1.0f / (maxX - minX)
        : 0.0f;
    const float invHeight
        = (maxY - minY) > std::numeric_limits<float>::epsilon()
        ? 1.0f / (maxY - minY)
        : 0.0f;

    processedVertices.reserve(static_cast<size_t>(vertexCount) * 9);
    for (int i = 0; i < vertexCount; ++i) {
        const float x = vertices[i * originalStride + 0];
        const float y = vertices[i * originalStride + 1];
        const float z = vertices[i * originalStride + 2];
        const float r = vertices[i * originalStride + 3];
        const float g = vertices[i * originalStride + 4];
        const float b = vertices[i * originalStride + 5];
        const float a = vertices[i * originalStride + 6];

        const float u = invWidth == 0.0f ? 0.5f : (x - minX) * invWidth;
        const float v = invHeight == 0.0f ? 0.5f : (y - minY) * invHeight;

        processedVertices.insert(
            processedVertices.end(), { x, y, z, r, g, b, a, u, v });
    }

    glBufferData(GL_ARRAY_BUFFER, processedVertices.size() * sizeof(float),
        processedVertices.data(), GL_STATIC_DRAW);

    if (!processedVertices.empty()) {
        m_color = glm::vec3(
            processedVertices[3], processedVertices[4], processedVertices[5]);
    }

    if (hasIndices) {
        glGenBuffers(1, &EBO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER,
            indices.size() * sizeof(unsigned int), indices.data(),
            GL_STATIC_DRAW);
        indexCount = static_cast<unsigned int>(indices.size());
        useIndices = true;
    } else {
        EBO = 0;
        constexpr int newStride = 9;
        indexCount
            = static_cast<unsigned int>(processedVertices.size() / newStride);
        useIndices = false;
    }

    const GLsizei stride = static_cast<GLsizei>(9 * sizeof(float));
    glVertexAttribPointer(
        0, 3, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<void *>(0));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, stride,
        reinterpret_cast<void *>(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride,
        reinterpret_cast<void *>((3 + 4) * sizeof(float)));
    glEnableVertexAttribArray(2);
    glBindVertexArray(0);

    isActive = true;
}

void Object2D::draw(const ShaderProgram &vectorial,
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
    vectorial.setInt("filterMode", static_cast<int>(filterMode));
    glm::vec2 texelSize = useTexture
        ? glm::vec2(1.0f / static_cast<float>(texture->size.x),
            1.0f / static_cast<float>(texture->size.y))
        : glm::vec2(0.0f);
    vectorial.setVec2("texelSize", texelSize);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

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
    glDisable(GL_BLEND);
}
