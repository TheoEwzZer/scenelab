#include "../../include/objects/AnalyticalSphere.hpp"
#include "../../include/GeometryGenerator.hpp"

AnalyticalSphere::AnalyticalSphere(
    float radius, int sectors, int stacks, const glm::vec3 &color)
{
    init(radius, sectors, stacks);
    setColor(color);
}

void AnalyticalSphere::init(float radius, int sectors, int stacks)
{
    m_primitiveType = PrimitiveType::Sphere;
    m_sphereRadius = radius;

    auto data = GeometryGenerator::generateSphere(radius, sectors, stacks);
    m_vertices = data.vertices;

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    glBufferData(GL_ARRAY_BUFFER, m_vertices.size() * sizeof(float),
        m_vertices.data(), GL_STATIC_DRAW);

    indexCount = static_cast<unsigned int>(m_vertices.size() / 8);
    useIndices = false;

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

void AnalyticalSphere::draw([[maybe_unused]] const ShaderProgram &vectorial,
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

    m_mat.setRasterShaderUniforms(lighting);

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
