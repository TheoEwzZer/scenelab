#ifndef SCENELAB_MATERIAL_H
#define SCENELAB_MATERIAL_H

#include "ShaderProgram.hpp"
#include "glm/glm.hpp"

struct Material {
    // Classic Phong constructor (backward compatible)
    Material(const glm::vec3 &ambientColor = { 0.2f, 0.2f, 0.2f },
        const glm::vec3 &diffuseColor = { 1.0f, 1.0f, 1.0f },
        const glm::vec3 &specularColor = { 1.0f, 1.0f, 1.0f },
        const glm::vec3 &emissiveColor = { 0.0f, 0.0f, 0.0f },
        float shininess = 5.0f, float roughness = 0.5f,
        float percentSpecular = 0.0f, float indexOfRefraction = 1.0f,
        float refractionChance = 0.0f) :
        m_ambientColor(ambientColor),
        m_diffuseColor(diffuseColor), m_specularColor(specularColor),
        m_emissiveColor(emissiveColor), m_shininess(shininess), m_percentSpecular(percentSpecular),
        m_roughness(roughness), m_indexOfRefraction(indexOfRefraction),
        m_refractionChance(refractionChance),
        m_metallic(0.0f), m_ao(1.0f), m_usePBR(false) {};

    // PBR constructor
    static Material createPBR(const glm::vec3 &albedo, float metallic,
        float roughness, float ao = 1.0f,
        const glm::vec3 &emissive = { 0.0f, 0.0f, 0.0f })
    {
        Material mat;
        mat.m_diffuseColor = albedo;
        mat.m_emissiveColor = emissive;
        mat.m_metallic = metallic;
        mat.m_roughness = roughness;
        mat.m_ao = ao;
        mat.m_usePBR = true;
        return mat;
    }

    ~Material() {};

    // Classic Phong properties (kept for backward compatibility)
    glm::vec3 m_ambientColor;
    glm::vec3 m_diffuseColor;
    glm::vec3 m_specularColor;
    glm::vec3 m_emissiveColor;
    float m_shininess = 5.0f;

    // Path Tracing
    float m_percentSpecular = 0.0f;
    float m_roughness = 0.5f;
    float m_indexOfRefraction = 1.0f;
    float m_refractionChance = 0.0f;

    float m_metallic;
    float m_ao;
    bool m_usePBR;

    void setShaderUniforms(const ShaderProgram &shader) const
    {
        // Classic Phong uniforms
        shader.setVec3("objectMaterial.ambient", m_ambientColor);
        shader.setVec3("objectMaterial.diffuse", m_diffuseColor);
        shader.setVec3("objectMaterial.specular", m_specularColor);
        shader.setVec3("objectMaterial.emissive", m_emissiveColor);
        shader.setFloat("objectMaterial.shininess", m_shininess);
        // Also set PBR uniforms (for PBR shader compatibility)
        setPBRShaderUniforms(shader);
    }

    void setPBRShaderUniforms(const ShaderProgram &shader) const
    {
        shader.setVec3("material.albedo", m_diffuseColor);
        shader.setVec3("material.emissive", m_emissiveColor);
        shader.setFloat("material.metallic", m_metallic);
        shader.setFloat("material.roughness", m_roughness);
        shader.setFloat("material.ao", m_ao);
    }
};

#endif // SCENELAB_MATERIAL_H
