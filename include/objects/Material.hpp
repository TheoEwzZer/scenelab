#ifndef SCENELAB_MATERIAL_H
#define SCENELAB_MATERIAL_H

#include "ShaderProgram.hpp"
#include "glm/glm.hpp"

struct Material {
    Material(const glm::vec3 &ambientColor =  {0.2f,0.2f,0.2f}, const glm::vec3 &diffuseColor = {1.0f,1.0f,1.0f},
        const glm::vec3 &specularColor = {1.0f,1.0f,1.0f}, const glm::vec3 &emissiveColor = {0.0f,0.0f,0.0f},
        float shininess = 5.0f) :
        m_ambientColor(ambientColor), m_diffuseColor(diffuseColor),
        m_specularColor(specularColor), m_emissiveColor(emissiveColor),
        m_shininess(shininess) {};
    ~Material() {};

    glm::vec3 m_ambientColor;
    glm::vec3 m_diffuseColor;
    glm::vec3 m_specularColor;
    glm::vec3 m_emissiveColor ;
    float m_shininess;

    void setShaderUniforms(const ShaderProgram &shader) const
    {
        shader.setVec3("objectMaterial.ambient", m_ambientColor);
        shader.setVec3("objectMaterial.diffuse", m_diffuseColor);
        shader.setVec3("objectMaterial.specular", m_specularColor);
        shader.setVec3("objectMaterial.emissive", m_emissiveColor);
        shader.setFloat("objectMaterial.shininess", m_shininess);
    }
};

#endif // SCENELAB_MATERIAL_H
