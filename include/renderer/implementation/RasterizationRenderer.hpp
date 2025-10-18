#pragma once

#include "../interface/ARenderer.hpp"
#include "ShaderProgram.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

class RasterizationRenderer : public ARenderer {
private:
    struct RenderObject {
        unsigned int VAO, VBO, EBO, bboxVAO { 0 }, bboxVBO { 0 };
        unsigned int texture;
        unsigned int indexCount;
        bool useIndices = false;
        glm::mat4 modelMatrix { 1.0f };
        bool isActive = true;
        bool isLight = false;
        bool useTexture = true;
        glm::vec3 objectColor { 1.0f, 1.0f, 1.0f };
    };

    glm::mat4 m_viewMatrix { 1.0f };
    glm::mat4 m_projMatrix { 1.0f };

    ShaderProgram m_lightingShader;
    ShaderProgram m_pointLightShader;
    ShaderProgram m_bboxShader;

    std::vector<RenderObject> m_renderObjects;
    std::vector<int> m_freeSlots;

    unsigned int loadTexture(std::string filepath);
    void createBoundingBoxBuffers(RenderObject &obj);

public:
    explicit RasterizationRenderer();
    virtual ~RasterizationRenderer() override;

    RasterizationRenderer(const RasterizationRenderer &) = delete;
    RasterizationRenderer &operator=(const RasterizationRenderer &) = delete;

    // Object Related
    int registerObject(const std::vector<float> &vertices,
        const std::vector<unsigned int> &indices,
        const std::string &texturePath, bool isLight) override;
    int registerObject(const std::vector<float> &vertices,
        const std::vector<unsigned int> &indices, bool isLight);
    int registerObject(const std::vector<float> &vertices,
        const std::vector<unsigned int> &indices, const glm::vec3 &color,
        bool isLight);
    void updateTransform(int objectId, const glm::mat4 &modelMatrix) override;
    void removeObject(int objectId) override;
    void drawBoundingBox(int objectId, const glm::vec3 &corner1,
        const glm::vec3 &corner2) override;

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
