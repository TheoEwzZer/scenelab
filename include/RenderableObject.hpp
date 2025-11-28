//
// Created by clmonn on 11/14/25.
//

#ifndef SCENELAB_RENDERABLEOBJECT_H
#define SCENELAB_RENDERABLEOBJECT_H

#include <glm/glm.hpp>

#include "ShaderProgram.hpp"
#include "objects/Material.hpp"
#include "renderer/TextureLibrary.hpp"

enum class FilterMode : int { None = 0, Grayscale, Sharpen, EdgeDetect, Blur };

enum class PrimitiveType : int { Mesh = 0, Sphere, Plane };

class RenderableObject {
protected:
    unsigned int VAO = 0, VBO = 0, EBO = 0;

    bool useIndices = false;
    unsigned int indexCount = 0;

    glm::mat4 modelMatrix { 1.0f };
    bool isActive = true;

    bool m_useTexture = true;
    int m_textureHandle = -1;
    FilterMode filterMode = FilterMode::None;

    // Material for classical illumination (Phong/Blinn-Phong)
    Material m_mat;

    // Properties for path tracing
    std::vector<float> m_vertices;
    std::vector<unsigned int> m_indices;

    glm::vec3 m_emissive = glm::vec3(0.0f);
    float m_percentSpecular = 0.0f;
    float m_roughness = 0.5f;
    glm::vec3 m_specularColor = glm::vec3(1.0f);

    float m_indexOfRefraction = 1.0f;
    float m_refractionChance = 0.0f;

    PrimitiveType m_primitiveType = PrimitiveType::Mesh;
    float m_sphereRadius = 0.5f;
    glm::vec3 m_planeNormal = glm::vec3(0.0f, 1.0f, 0.0f);

public:
    RenderableObject() = default;

    virtual ~RenderableObject()
    {
        if (VAO != 0) {
            glDeleteVertexArrays(1, &VAO);
        }
        if (VBO != 0) {
            glDeleteBuffers(1, &VBO);
        }
        if (EBO != 0) {
            glDeleteBuffers(1, &EBO);
        }
    }

    void setMaterial(const Material &mat) {m_mat = mat;};

    Material &getMaterial() {return m_mat;};

    void setFilterMode(const FilterMode mode) { filterMode = mode; }

    [[nodiscard]] FilterMode getFilterMode() const { return filterMode; }

    void setUseTexture(const bool useTexture)
    {
        m_useTexture = useTexture && m_textureHandle >= 0;
    }

    [[nodiscard]] bool isUsingTexture() const { return m_useTexture; }

    void setModelMatrix(const glm::mat4 &matrix) { modelMatrix = matrix; }

    [[nodiscard]] glm::mat4 getModelMatrix() const { return modelMatrix; }

    void setStatus(const bool status) { isActive = status; }

    [[nodiscard]] bool getStatus() const { return isActive; }

    void setColor(const glm::vec3 &color)
    {
        m_mat = Material(m_mat.m_ambientColor, color, m_mat.m_specularColor, m_mat.m_emissiveColor);
        m_useTexture = false;
    }

    // [[nodiscard]] glm::vec3 getColor() const { return m_color; }

    glm::vec3 getColor() const { return m_mat.m_diffuseColor; }

    void setEmissive(const glm::vec3 &emissive)
    {
        m_emissive = emissive;
        m_mat.m_emissiveColor = emissive; // Sync for rasterization
    }

    [[nodiscard]] glm::vec3 getEmissive() const { return m_emissive; }

    void setPercentSpecular(float percent) { m_percentSpecular = percent; }

    [[nodiscard]] float getPercentSpecular() const
    {
        return m_percentSpecular;
    }

    void setRoughness(float roughness) { m_roughness = roughness; }

    [[nodiscard]] float getRoughness() const { return m_roughness; }

    void setSpecularColor(const glm::vec3 &color) { m_specularColor = color; }

    [[nodiscard]] glm::vec3 getSpecularColor() const
    {
        return m_specularColor;
    }

    void setIndexOfRefraction(float ior) { m_indexOfRefraction = ior; }

    [[nodiscard]] float getIndexOfRefraction() const
    {
        return m_indexOfRefraction;
    }

    void setRefractionChance(float chance) { m_refractionChance = chance; }

    [[nodiscard]] float getRefractionChance() const
    {
        return m_refractionChance;
    }

    void setPrimitiveType(PrimitiveType type) { m_primitiveType = type; }

    [[nodiscard]] PrimitiveType getPrimitiveType() const
    {
        return m_primitiveType;
    }

    void setSphereRadius(float radius) { m_sphereRadius = radius; }

    [[nodiscard]] float getSphereRadius() const { return m_sphereRadius; }

    void setPlaneNormal(const glm::vec3 &normal)
    {
        m_planeNormal = glm::normalize(normal);
    }

    [[nodiscard]] glm::vec3 getPlaneNormal() const { return m_planeNormal; }

    void assignTexture(const int textureHandle)
    {
        m_textureHandle = textureHandle;
        m_useTexture = (textureHandle != -1);
    }

    std::vector<float> &getVertices() { return m_vertices; }

    std::vector<unsigned int> &getIndices() { return m_indices; }

    [[nodiscard]] int getTextureHandle() const { return m_textureHandle; }

    virtual void useShader([[maybe_unused]] ShaderProgram &shader) const {}

    virtual void draw(const ShaderProgram &vectorial,
        const ShaderProgram &pointLight, const ShaderProgram &lighting,
        const TextureLibrary &textures) const
        = 0;
};

#endif // SCENELAB_RENDERABLEOBJECT_H
