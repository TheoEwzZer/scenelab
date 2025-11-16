#pragma once

#include "../interface/ARenderer.hpp"
#include "ShaderProgram.hpp"
#include "renderer/TextureLibrary.hpp"
#include <array>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <string>
#include <vector>

#include <memory>

enum class ToneMappingMode : int { Off = 0, Reinhard, ACES };

class RasterizationRenderer : public ARenderer {
private:

    glm::mat4 m_viewMatrix { 1.0f };
    glm::mat4 m_projMatrix { 1.0f };
    Camera::ProjectionMode m_projectionMode { Camera::ProjectionMode::Perspective };

    ShaderProgram m_lightingShader;
    ShaderProgram m_vectorialShader;
    ShaderProgram m_pointLightShader;
    ShaderProgram m_bboxShader;
    ShaderProgram m_skyboxShader;

    std::vector<std::unique_ptr<RenderableObject>> m_renderObjects;
    std::vector<int> m_freeSlots;

    TextureLibrary m_textureLibrary;

    unsigned int m_skyboxVAO = 0;
    unsigned int m_skyboxVBO = 0;

    unsigned int m_bboxVAO = 0;
    unsigned int m_bboxVBO = 0;

    ToneMappingMode m_toneMappingMode = ToneMappingMode::Reinhard;
    float m_toneMappingExposure = 1.0f;

    void initializeSkyboxGeometry();
    void drawSkybox() const;
    void createBoundingBoxBuffers();

public:
    static constexpr std::array<const char *, 5> FILTER_MODE_LABELS { "None",
        "Grayscale", "Sharpen", "Edge Detect", "Blur" };
    static constexpr std::array<const char *, 3> TONEMAP_LABELS { "Off",
        "Reinhard", "ACES" };

    explicit RasterizationRenderer();
    virtual ~RasterizationRenderer() override;

    RasterizationRenderer(const RasterizationRenderer &) = delete;
    RasterizationRenderer &operator=(const RasterizationRenderer &) = delete;

    // Object Related
    int registerObject(std::unique_ptr<RenderableObject> obj) override;
    int registerObject(std::unique_ptr<RenderableObject> obj, const std::string &texturePath) override;
    int registerObject(std::unique_ptr<RenderableObject> obj, const glm::vec3 &color) override;
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

    void setProjectionMode(Camera::ProjectionMode mode) override
    {
        m_projectionMode = mode;
    }

    // Rendering related
    void beginFrame() override;
    void drawAll() override;
    void endFrame() override;

    int loadTexture2D(const std::string &filepath, bool srgb = false);
    int createCheckerboardTexture(const std::string &name, int width,
        int height, const glm::vec3 &colorA, const glm::vec3 &colorB,
        int checks = 8, bool srgb = false);
    int createNoiseTexture(const std::string &name, int width, int height,
        float frequency = 8.0f, bool srgb = false);
    int createSolidColorTexture(const std::string &name,
        const glm::vec3 &color, int width = 1, int height = 1,
        bool srgb = false);
    void assignTextureToObject(int objectId, int textureHandle) const;
    void assignTextureToObject(int objectId, const std::string& texturePath);
    int getObjectTextureHandle(int objectId) const;
    void setObjectFilter(int objectId, FilterMode mode) const;
    FilterMode getObjectFilter(int objectId) const;
    void setObjectUseTexture(int objectId, bool useTexture) const;
    bool getObjectUseTexture(int objectId) const;
    const TextureResource *getTextureResource(int handle) const;

    const std::vector<TextureResource> &getTextureResources() const;

    void setToneMappingMode(ToneMappingMode mode);

    ToneMappingMode getToneMappingMode() const { return m_toneMappingMode; }

    void setToneMappingExposure(float exposure);

    float getToneMappingExposure() const { return m_toneMappingExposure; }

    int createColoredCubemap(const std::string &name,
        const std::array<glm::vec3, 6> &faceColors, int edgeSize = 32,
        bool srgb = false);
    int loadCubemapFromEquirectangular(const std::string &name,
        const std::string &equirectPath, int resolution = 512,
        bool srgb = false);
    void setActiveCubemap(int cubemapHandle);

    int getActiveCubemap() const
    {
        return m_textureLibrary.getActiveCubemap();
    }

    const std::vector<int> &getCubemapHandles() const;
};
