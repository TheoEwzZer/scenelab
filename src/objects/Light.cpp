//
// Created by clmonn on 11/13/25.
//

#include "../../include/objects/Light.hpp"
#include "glm/ext/vector_float3.hpp"
#include "glm/fwd.hpp"
#include "glm/gtc/quaternion.hpp"
#include <sstream>
#include <stdexcept>
#include "GeometryGenerator.hpp"
#include <string>

Light::Light()
{
    init();
    m_color = glm::vec3(1,1,1);
    m_mat.m_diffuseColor = m_color;
    m_mat.m_specularColor = glm::vec3(1.0f);
    m_mat.m_percentSpecular = 0.0f;
    m_mat.m_roughness = 1.0f;
    m_mat.m_refractionChance = 0.0f;
    m_mat.m_indexOfRefraction = 1.0f;
    updateEmissive();
}

void Light::init()
{
    m_primitiveType = PrimitiveType::Sphere;
    int sphereRadius = 1.0f;

    m_gdata = GeometryGenerator::generateSphere(sphereRadius, 1, 1);
    auto vertices = m_gdata.vertices;

    glGenVertexArrays(1, &VAO);
    // Convert Vertex data to float array for path tracing
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

GData &Light::getGData()
{
    return m_gdata;
}

void Light::useShader(ShaderProgram &shader) const {
    (void) shader;
}

void Light::updateEmissive()
{
    m_mat.m_emissiveColor = m_color * m_intensity * 1500.0f;
}

void Light::setIntensity(float intensity)
{
    m_intensity = intensity;
    updateEmissive();
}

void Light::setDirectional(const glm::vec3 &color, float intensity)
{
    m_color = color;
    m_type = Directional;
    m_intensity = intensity;
    updateEmissive();
}

void Light::setPoint(const glm::vec3 &color, float kc, float kl, float kq, float intensity)
{
    m_color = color;
    m_kc = kc;
    m_kl = kl;
    m_kq = kq;
    m_type = Point;
    m_intensity = intensity;
    updateEmissive();
}

void Light::setSpot(const glm::vec3 &color, float kc, float kl, float kq, float p, float intensity)
{
    m_color = color;
    m_kc = kc;
    m_kl = kl;
    m_kq = kq;
    m_p = p;
    m_type = Spot;
    m_intensity = intensity;
    updateEmissive();
}

Light::Type Light::getType() const
{
    return m_type;
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

void Light::setUniforms(
    int uniformID, const ShaderProgram &lightingShader) const
{
    std::string uniformName;

    glm::vec4 worldDir = modelMatrix * glm::vec4(0.0f, 0.0f, -1.0f, 0.0f);

    glm::vec3 dir = glm::normalize(glm::vec3(worldDir));

    switch (m_type) {
        case Directional:
            uniformName
                = "directionalLights[" + std::to_string(uniformID) + "].";
            lightingShader.setVec3(uniformName + "direction", dir);
            lightingShader.setVec3(uniformName + "color", m_color * m_intensity);
            break;
        case Point:
            uniformName = "pointLights[" + std::to_string(uniformID) +  "].";
            lightingShader.setVec3(uniformName + "position", glm::vec3(modelMatrix[3]));
            lightingShader.setVec3(uniformName + "color", m_color * m_intensity);
            lightingShader.setFloat(uniformName + "ke", m_kc);
            lightingShader.setFloat(uniformName + "kl", m_kl);
            lightingShader.setFloat(uniformName + "kq", m_kq);
            break;
        case Spot:
            uniformName = "spotLights[" + std::to_string(uniformID) +  "].";
            lightingShader.setVec3(uniformName + "position", glm::vec3(modelMatrix[3]));
            lightingShader.setVec3(uniformName + "color", m_color * m_intensity);
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
    (void) textures;
    lighting.use();
    lighting.setMat4("model", modelMatrix);
    lighting.setBool("useTexture", false);
    m_mat.setShaderUniforms(lighting);

    lighting.setInt("filterMode", static_cast<int>(filterMode));
    glm::vec2 texelSize = glm::vec2(0.0f);
    lighting.setVec2("texelSize", texelSize);

    glBindTexture(GL_TEXTURE_2D, 0);
    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, indexCount);
    glBindVertexArray(0);
}
