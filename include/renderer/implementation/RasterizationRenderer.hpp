#pragma once

#include "../interface/ARenderer.hpp"
#include "ShaderProgram.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

class RasterizationRenderer : public ARenderer {
private:
    struct RenderObject {
        unsigned int VAO, VBO, EBO;
        unsigned int texture;
        unsigned int indexCount;
        bool useIndices = false;
        glm::mat4 modelMatrix { 1.0f };
        bool isActive = true;
        bool isLight = false;
        bool is2D = false;
    };

    glm::mat4 m_viewMatrix { 1.0f };
    glm::mat4 m_projMatrix { 1.0f };

    ShaderProgram m_lightingShader;
    ShaderProgram m_vectorialShader;
    ShaderProgram m_pointLightShader;

    std::vector<RenderObject> m_renderObjects;
    std::vector<int> m_freeSlots;

    unsigned int loadTexture(std::string filepath);

public:
    explicit RasterizationRenderer();
    virtual ~RasterizationRenderer() override;

    RasterizationRenderer(const RasterizationRenderer &) = delete;
    RasterizationRenderer &operator=(const RasterizationRenderer &) = delete;

    // Object Related
    int registerObject(const std::vector<float> &vertices,
        const std::vector<unsigned int> &indices,
        const std::string &texturePath, bool isLight, bool is2D) override;
    void updateTransform(int objectId, const glm::mat4 &modelMatrix) override;
    void removeObject(int objectId) override;

    // Camera Related
    void setViewMatrix(const glm::mat4 &view) override { m_viewMatrix = view; }

    void setProjectionMatrix(const glm::mat4 &proj) override
    {
        m_projMatrix = proj;
    }

    // Rendering related
    void beginFrame() override;
    void drawAll() override;
    void endFrame() override;
};
