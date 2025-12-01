//
// Created by clmonn on 11/14/25.
//

#ifndef SCENELAB_RENDERABLEOBJECT_H
#define SCENELAB_RENDERABLEOBJECT_H

#include <cmath>
#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include "ShaderProgram.hpp"
#include "glm/gtx/quaternion.hpp"
#include "objects/Material.hpp"
#include "renderer/TextureLibrary.hpp"

struct Vertex {
    glm::vec3 position;
    glm::vec2 texCoord;
    glm::vec3 normal;
    glm::vec3 tangent;
    glm::vec3 bitangent;

    static void addVertex(std::vector<Vertex> &vertices,
        const glm::vec3 &position, const glm::vec2 &texCoord,
        const glm::vec3 &normal)
    {
        const Vertex vertex { position, texCoord, normal, glm::vec3(0.0f),
            glm::vec3(0.0f) };
        vertices.push_back(vertex);
    }

    // Credit: https://learnopengl.com/Advanced-Lighting/Normal-Mapping
    static void computeTangents(std::vector<Vertex> &vertices)
    {
        for (size_t i = 0; i + 2 < vertices.size(); i += 3) {
            Vertex &v0 = vertices[i + 0];
            Vertex &v1 = vertices[i + 1];
            Vertex &v2 = vertices[i + 2];

            const glm::vec3 edge1 = v1.position - v0.position;
            const glm::vec3 edge2 = v2.position - v0.position;
            const glm::vec2 deltaUV1 = v1.texCoord - v0.texCoord;
            const glm::vec2 deltaUV2 = v2.texCoord - v0.texCoord;

            float denom = deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y;
            float r = (std::fabs(denom) < 1e-8f) ? 0.0f : 1.0f / denom;

            glm::vec3 tangent = r * (deltaUV2.y * edge1 - deltaUV1.y * edge2);
            glm::vec3 bitangent
                = r * (-deltaUV2.x * edge1 + deltaUV1.x * edge2);
            v0.tangent += tangent;
            v1.tangent += tangent;
            v2.tangent += tangent;
            v0.bitangent += bitangent;
            v1.bitangent += bitangent;
            v2.bitangent += bitangent;
        }

        // Orthonormalize and fix handedness per-vertex
        for (auto &v : vertices) {
            if (glm::length2(v.normal) < 1e-12f) {
                v.normal = glm::vec3(0.0f, 0.0f, 1.0f);
            }
            glm::vec3 N = glm::normalize(v.normal);
            // If tangent is zero (degenerate), create a fallback
            if (glm::length2(v.tangent) < 1e-12f) {
                // pick any vector not parallel to N
                glm::vec3 up = fabs(N.z) < 0.999f ? glm::vec3(0, 0, 1)
                                                  : glm::vec3(0, 1, 0);
                v.tangent = glm::normalize(glm::cross(up, N));
            }
            glm::vec3 T
                = glm::normalize(v.tangent - N * glm::dot(N, v.tangent));
            // compute handedness from accumulated bitangent
            float handedness = (glm::dot(glm::cross(N, T), v.bitangent) < 0.0f)
                ? -1.0f
                : 1.0f;
            v.tangent = T;
            v.bitangent = glm::cross(N, T) * handedness;
        }
    }

    static std::vector<Vertex> convertToVertex14(
        const std::vector<float> &vertices)
    {
        std::vector<Vertex> result;

        for (size_t i = 0; i < vertices.size(); i += 14) {
            Vertex vertex {};
            vertex.position
                = glm::vec3(vertices[i + 0], vertices[i + 1], vertices[i + 2]);
            vertex.texCoord = glm::vec2(vertices[i + 3], vertices[i + 4]);
            vertex.normal
                = glm::vec3(vertices[i + 5], vertices[i + 6], vertices[i + 7]);
            vertex.tangent = glm::vec3(0.f);
            vertex.bitangent = glm::vec3(0.f);
            result.push_back(vertex);
        }
        return result;
    }
};

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

    bool m_useNormalMap = false;
    int m_normalMapHandler = -1;
    FilterMode filterMode = FilterMode::None;

    // Material for classical illumination (Phong/Blinn-Phong)
    Material m_mat;

    // Properties for path tracing
    std::vector<float> m_vertices;
    std::vector<unsigned int> m_indices;

    glm::vec3 m_emissive = glm::vec3(0.0f);
    float m_percentSpecular = 0.0f;
    float m_roughness = 0.5f;
    float m_metallic = 0.0f; // PBR metallic property
    float m_ao = 1.0f; // PBR ambient occlusion
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

    void setMaterial(const Material &mat) { m_mat = mat; };

    Material &getMaterial() { return m_mat; };

    void setFilterMode(const FilterMode mode) { filterMode = mode; }

    [[nodiscard]] FilterMode getFilterMode() const { return filterMode; }

    void setUseTexture(const bool useTexture)
    {
        m_useTexture = useTexture && m_textureHandle >= 0;
    }

    void setUseNormalMap(const bool useNormalMap)
    {
        m_useNormalMap = useNormalMap && m_normalMapHandler >= 0;
    }

    [[nodiscard]] bool isUsingTexture() const { return m_useTexture; }

    [[nodiscard]] bool isUsingNormalMap() const { return m_useNormalMap; }

    void setModelMatrix(const glm::mat4 &matrix) { modelMatrix = matrix; }

    [[nodiscard]] glm::mat4 getModelMatrix() const { return modelMatrix; }

    void setStatus(const bool status) { isActive = status; }

    [[nodiscard]] bool getStatus() const { return isActive; }

    void setColor(const glm::vec3 &color)
    {
        m_mat = Material(m_mat.m_ambientColor, color, m_mat.m_specularColor,
            m_mat.m_emissiveColor);
        m_useTexture = false;
    }

    glm::vec3 getColor() const { return m_mat.m_diffuseColor; }

    void setEmissive(const glm::vec3 &emissive)
    {
        m_emissive = emissive;
        m_mat.m_emissiveColor = emissive;
    }

    [[nodiscard]] glm::vec3 getEmissive() const { return m_emissive; }

    void setPercentSpecular(float percent) { m_percentSpecular = percent; }

    [[nodiscard]] float getPercentSpecular() const
    {
        return m_percentSpecular;
    }

    void setRoughness(float roughness)
    {
        m_roughness = roughness;
        m_mat.m_roughness = roughness; // Sync with material
    }

    [[nodiscard]] float getRoughness() const { return m_roughness; }

    void setMetallic(float metallic)
    {
        m_metallic = metallic;
        m_mat.m_metallic = metallic; // Sync with material
    }

    [[nodiscard]] float getMetallic() const { return m_metallic; }

    void setAO(float ao)
    {
        m_ao = ao;
        m_mat.m_ao = ao; // Sync with material
    }

    [[nodiscard]] float getAO() const { return m_ao; }

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

    void assignNormalMap(const int normalMapHandle)
    {
        m_normalMapHandler = normalMapHandle;
        m_useNormalMap = (normalMapHandle != -1);
    }

    [[nodiscard]] int getNormalMapHandle() const { return m_normalMapHandler; }

    virtual void useShader([[maybe_unused]] ShaderProgram &shader) const {}

    void updateGeometry(const std::vector<float> &vertices)
    {
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float),
            vertices.data(), GL_DYNAMIC_DRAW);

        indexCount = vertices.size() / 3;
    }

    virtual void draw(const ShaderProgram &vectorial,
        const ShaderProgram &pointLight, const ShaderProgram &lighting,
        const TextureLibrary &textures) const
        = 0;
};

#endif // SCENELAB_RENDERABLEOBJECT_H
