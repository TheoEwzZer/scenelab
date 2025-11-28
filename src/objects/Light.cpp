//
// Created by clmonn on 11/13/25.
//

#include "../../include/objects/Light.hpp"
#include "glm/fwd.hpp"
#include "glm/gtc/quaternion.hpp"
#include <sstream>
#include <stdexcept>
#include <string>

Light::Light(const std::vector<float> &vertices,
    const std::vector<unsigned int> &indices, const glm::vec3 &color)
{
    init(vertices, indices);
    m_color = color;
    m_mat.m_diffuseColor = color;
    updateEmissive();
}

Light::Light(const std::vector<float> &vertices,
    const std::vector<unsigned int> &indices, const int textureHandle)
{
    init(vertices, indices);
    this->m_textureHandle = textureHandle;
    updateEmissive();
}

void Light::init(const std::vector<float> &vertices,
    const std::vector<unsigned int> &indices)
{
    // Store vertices for path tracing
    m_vertices = vertices;
    m_indices = indices;

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

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

void Light::useShader(ShaderProgram &shader) const {
    (void) shader;
}

void Light::updateEmissive()
{
    m_mat.m_emissiveColor = m_color;
}

void Light::setIntensity(float intensity)
{
    m_intensity = intensity;
    m_color = m_color * intensity;
    updateEmissive();
}

void Light::setDirectional(const glm::vec3 &color, float intensity)
{
    m_color = color;
    m_type = Directional;
    setIntensity(intensity);
    updateEmissive();
}

void Light::setPoint(const glm::vec3 &color, float kc, float kl, float kq, float intensity)
{
    m_color = color;
    m_kc = kc;
    m_kl = kl;
    m_kq = kq;
    m_type = Point;
    setIntensity(intensity);
    updateEmissive();
}

void Light::setSpot(const glm::vec3 &color, float ke, float kl, float kq, float p, float intensity)
{
    m_color = color;
    m_kc = ke;
    m_kl = kl;
    m_kq = kq;
    m_p = p;
    m_type = Spot;
    setIntensity(intensity);
    updateEmissive();
}

std::string Light::getNameStr() const
{
    switch (this->m_type) {
        case Directional:
            return ("Directional Light");
            break;
        case Spot:
            return ("Spot Light");
            break;
        case Point:
            return ("Point Light");
            break;
        default:
            break;
    }
    return ("Light");
}

void Light::setUniforms(int uniformID, const ShaderProgram &lightingShader) const
{
    std::string uniformName;

    glm::vec4 worldDir = modelMatrix * glm::vec4(0.0f, 0.0f, -1.0f, 0.0f);;
    glm::vec3 dir = glm::normalize(glm::vec3(worldDir));

    switch (m_type) {
        case Directional:
            uniformName = "directionalLights[" + std::to_string(uniformID) +  "].";
            lightingShader.setVec3(uniformName + "direction", dir);
            lightingShader.setVec3(uniformName + "color", m_color);
            break;
        case Point:
            uniformName = "pointLights[" + std::to_string(uniformID) +  "].";
            lightingShader.setVec3(uniformName + "position", glm::vec3(modelMatrix[3]));
            lightingShader.setVec3(uniformName + "color", m_color);
            lightingShader.setFloat(uniformName + "ke", m_kc);
            lightingShader.setFloat(uniformName + "kl", m_kl);
            lightingShader.setFloat(uniformName + "kq", m_kq);
            break;
        case Spot:
            uniformName = "spotLights[" + std::to_string(uniformID) +  "].";
            lightingShader.setVec3(uniformName + "position", glm::vec3(modelMatrix[3]));
            lightingShader.setVec3(uniformName + "color", m_color);
            lightingShader.setVec3(uniformName + "direction", dir);
            lightingShader.setFloat(uniformName + "ke", m_kc);
            lightingShader.setFloat(uniformName + "kl", m_kl);
            lightingShader.setFloat(uniformName + "kq", m_kq);
            lightingShader.setFloat(uniformName + "p", m_p);
            break;
        default:
            throw std::runtime_error("Light type not supported");
    }
}

void Light::draw([[maybe_unused]] const ShaderProgram &vectorial,
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
    if (!useIndices) {
        glDrawArrays(GL_TRIANGLES, 0, indexCount);
    } else {
        glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
    }
    glBindVertexArray(0);
}
