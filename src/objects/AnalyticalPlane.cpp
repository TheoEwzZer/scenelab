#include "../../include/objects/AnalyticalPlane.hpp"
#include "../../include/GeometryGenerator.hpp"

AnalyticalPlane::AnalyticalPlane(
    float width, float height, const glm::vec3 &normal, const glm::vec3 &color)
{
    init(width, height, normal);
    setColor(color);
}

void AnalyticalPlane::init(float width, float height, const glm::vec3 &normal)
{
    m_primitiveType = PrimitiveType::Plane;
    m_planeNormal = glm::normalize(normal);

    auto data = GeometryGenerator::generatePlane(width, height, normal);

    // Convert Vertex data to float array for path tracing
    m_vertices.reserve(data.vertices.size() * 14);
    for (const auto &v : data.vertices) {
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

    indexCount = static_cast<unsigned int>(data.vertices.size());
    useIndices = false;

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    glBufferData(GL_ARRAY_BUFFER, indexCount * sizeof(Vertex),
        data.vertices.data(), GL_STATIC_DRAW);

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

void AnalyticalPlane::draw([[maybe_unused]] const ShaderProgram &vectorial,
    [[maybe_unused]] const ShaderProgram &pointLight,
    const ShaderProgram &lighting, const TextureLibrary &textures) const
{
    const TextureResource *texture
        = textures.getTextureResource(m_textureHandle);

    lighting.use();
    lighting.setMat4("model", modelMatrix);
    const bool useTexture = this->m_useTexture && texture
        && texture->target == TextureTarget::Texture2D;
    lighting.setBool("useTexture", useTexture);

    m_mat.setShaderUniforms(lighting);

    lighting.setInt("filterMode", static_cast<int>(filterMode));
    glm::vec2 texelSize = useTexture
        ? glm::vec2(1.0f / static_cast<float>(texture->size.x),
            1.0f / static_cast<float>(texture->size.y))
        : glm::vec2(0.0f);
    lighting.setVec2("texelSize", texelSize);

    if (texture) {
        if (texture->target == TextureTarget::Texture2D) {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, texture->id);
        }
    } else {
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, indexCount);
    glBindVertexArray(0);
}
