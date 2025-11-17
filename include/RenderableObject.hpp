//
// Created by clmonn on 11/14/25.
//

#ifndef SCENELAB_RENDERABLEOBJECT_H
#define SCENELAB_RENDERABLEOBJECT_H

#include <glm/glm.hpp>

#include "ShaderProgram.hpp"
#include "renderer/TextureLibrary.hpp"

enum class FilterMode : int { None = 0, Grayscale, Sharpen, EdgeDetect, Blur };

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

    glm::vec3 m_color = glm::vec3(1.0f);

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
        m_color = color;
        m_useTexture = false;
    }

    [[nodiscard]] glm::vec3 getColor() const { return m_color; }

    void assignTexture(const int textureHandle)
    {
        m_textureHandle = textureHandle;
        m_useTexture = (textureHandle != -1);
    }

    [[nodiscard]] int getTextureHandle() const { return m_textureHandle; }

    virtual void useShader([[maybe_unused]] ShaderProgram &shader) const {}

    virtual void draw(const ShaderProgram &vectorial,
        const ShaderProgram &pointLight, const ShaderProgram &lighting,
        const TextureLibrary &textures) const
        = 0;
};

#endif // SCENELAB_RENDERABLEOBJECT_H
