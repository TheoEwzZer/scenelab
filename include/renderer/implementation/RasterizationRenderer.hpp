#pragma once

#include "../interface/IRenderer.hpp"
#include "RenderableObject.hpp"
#include "Camera.hpp"
#include "renderer/Window.hpp"
#include "ShaderProgram.hpp"
#include "glm/fwd.hpp"
#include "renderer/TextureLibrary.hpp"
#include "deferred/DeferredRenderer.hpp"
#include "pbr/IBLManager.hpp"
#include <array>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <string>
#include <vector>
#include <unordered_map>

#include <memory>

enum class ToneMappingMode : int { Off = 0, Reinhard, ACES };


class RasterizationRenderer : public IRenderer {
private:
    Window &m_window;
    LightingModel m_lightingModel = LAMBERT;

    glm::mat4 m_viewMatrix { 1.0f };
    glm::mat4 m_projMatrix { 1.0f };
    Camera::ProjectionMode m_projectionMode {
        Camera::ProjectionMode::Perspective
    };

    ShaderProgram m_lightingShader;
    ShaderProgram m_vectorialShader;
    ShaderProgram m_gouraudLightingShader;
    ShaderProgram m_pbrShader; // PBR Cook-Torrance shader
    ShaderProgram m_bboxShader;
    ShaderProgram m_skyboxShader;
    ShaderProgram m_deferredGeometryShader;
    ShaderProgram m_deferredLightingShader;

    std::vector<std::unique_ptr<RenderableObject>> m_renderObjects;
    std::vector<int> m_freeSlots;

    TextureLibrary m_textureLibrary;

    unsigned int m_skyboxVAO = 0;
    unsigned int m_skyboxVBO = 0;

    unsigned int m_bboxVAO = 0;
    unsigned int m_bboxVBO = 0;

    ToneMappingMode m_toneMappingMode = ToneMappingMode::Reinhard;
    float m_toneMappingExposure = 1.0f;

    std::unordered_map<int, CameraView> m_cameraViews;
    CameraOverlayCallback m_cameraOverlayCallback;
    BoundingBoxDrawCallback m_bboxDrawCallback;
    bool m_lockCameraWindows = false;
    int m_lockedCameraId = -1;

    // IBL support (for PBR)
    bool m_useIBL = false;
    std::unique_ptr<IBLManager> m_iblManager;
    IBLManager::IBLTextures m_currentIBLTextures;

    DeferredRenderer m_deferredRenderer;
    bool m_useDeferredRendering = false;

    void initializeSkyboxGeometry();
    void drawSkybox() const;
    void createBoundingBoxBuffers();
    void renderCameraViews(const Camera &cam, const CameraView &view);
    void renderDockableViews(CameraManager &cameraManager);

public:
    static constexpr std::array<const char *, 5> FILTER_MODE_LABELS { "None",
        "Grayscale", "Sharpen", "Edge Detect", "Blur" };
    static constexpr std::array<const char *, 3> TONEMAP_LABELS { "Off",
        "Reinhard", "ACES" };
    static constexpr glm::vec3 DEFAULT_AMBIENT_LIGHT_COLOR { 0.1f, 0.1f,
        0.1f };

    explicit RasterizationRenderer(Window &window);
    virtual ~RasterizationRenderer() override;

    RasterizationRenderer(const RasterizationRenderer &) = delete;
    RasterizationRenderer &operator=(const RasterizationRenderer &) = delete;

    // Object Related
    int registerObject(std::unique_ptr<RenderableObject> obj) override;
    int registerObject(std::unique_ptr<RenderableObject> obj,
        const std::string &texturePath) override;
    int registerObject(std::unique_ptr<RenderableObject> obj,
        const glm::vec3 &color) override;
    int registerObject(std::unique_ptr<RenderableObject> obj,
        const Material &material) override;
    void updateTransform(int objectId, const glm::mat4 &modelMatrix) override;
    void updateGeometry(
        int objectId, const std::vector<float> &vertices) override;
    void removeObject(int objectId) override;
    void drawBoundingBox(int objectId, const glm::vec3 &corner1,
        const glm::vec3 &corner2) override;
    RenderableObject &getRenderable(int objectId) const;
    std::vector<std::unique_ptr<RenderableObject>>
    extractAllObjects() override;

    // Material property accessors
    void setObjectColor(int objectId, const glm::vec3 &color) override;
    glm::vec3 getObjectColor(int objectId) const override;
    void setObjectEmissive(int objectId, const glm::vec3 &emissive) override;
    glm::vec3 getObjectEmissive(int objectId) const override;
    void setObjectPercentSpecular(int objectId, float percent) override;
    float getObjectPercentSpecular(int objectId) const override;
    void setObjectRoughness(int objectId, float roughness) override;
    float getObjectRoughness(int objectId) const override;
    void setObjectSpecularColor(int objectId, const glm::vec3 &color) override;
    glm::vec3 getObjectSpecularColor(int objectId) const override;
    void setObjectIndexOfRefraction(int objectId, float ior) override;
    float getObjectIndexOfRefraction(int objectId) const override;
    void setObjectRefractionChance(int objectId, float chance) override;
    float getObjectRefractionChance(int objectId) const override;

    void setObjectMaterial(int objectId, const Material &mat) override;
    void setAmbientLight(const glm::vec3 &color) override {m_ambientLightColor = color;};

    // PBR material accessors
    void setObjectMetallic(int objectId, float metallic);
    float getObjectMetallic(int objectId) const;
    void setObjectAO(int objectId, float ao);
    float getObjectAO(int objectId) const;

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
    void drawAll(Camera cam) override;
    void endFrame() override;

    // Window related
    bool shouldWindowClose() override;
    void addKeyCallback(
        int key, int action, std::function<void()> callback) override;
    void addCursorCallback(
        std::function<void(double, double)> callback) override;
    void addDropCallback(
        std::function<void(const std::vector<std::string> &paths,
            double mouseX, double mouseY)>
            callback) override;
    GLFWwindow *getWindow() const override;

    // Camera view management
    void createCameraViews(int id, int width = 512, int height = 512) override;
    void destroyCameraViews(int id) override;
    void renderAllViews(CameraManager &cameraManager) override;
    void setCameraOverlayCallback(CameraOverlayCallback callback) override;
    void setBoundingBoxDrawCallback(BoundingBoxDrawCallback callback) override;

    int loadTexture2D(const std::string &filepath, bool srgb = false);
    int loadNormalMap(const std::string &filepath);
    int createCheckerboardTexture(const std::string &name, int width,
        int height, const glm::vec3 &colorA, const glm::vec3 &colorB,
        int checks = 8, bool srgb = false);
    int createNoiseTexture(const std::string &name, int width, int height,
        float frequency = 8.0f, bool srgb = false);
    int createSolidColorTexture(const std::string &name,
        const glm::vec3 &color, int width = 1, int height = 1,
        bool srgb = false);
    void assignTextureToObject(int objectId, int textureHandle) const;
    void assignTextureToObject(int objectId, const std::string &texturePath);
    void assignNormalMapToObject(int objectId, int normalMapHandle) const;
    void assignNormalMapToObject(int objectId, const std::string &texturePath);
    void assignMaterialToObject(const int objectId, Material &mat) const;
    int getObjectTextureHandle(int objectId) const;
    int getObjectNormalMapHandle(int objectId) const;
    void setObjectFilter(int objectId, FilterMode mode) const;
    FilterMode getObjectFilter(int objectId) const;
    void setObjectUseTexture(int objectId, bool useTexture) const;
    bool getObjectUseTexture(int objectId) const;
    void setObjectUseNormalMap(int objectId, bool useNormalMap) const;
    bool getObjectUseNormalMap(int objectId) const;
    const TextureResource *getTextureResource(int handle) const;
    const NormalMapResource *getNormalMapResource(int handle) const;

    const std::vector<TextureResource> &getTextureResources() const;
    const std::vector<NormalMapResource> &getNormalMapResources() const;

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

    void setLightingModel(LightingModel model) { m_lightingModel = model; }

    LightingModel getLightingModel() const { return m_lightingModel; }

    void setUseIBL(bool useIBL);

    bool getUseIBL() const { return m_useIBL; }

    void generateIBLFromCurrentCubemap();

    void setUseDeferredRendering(bool useDeferred);

    bool getUseDeferredRendering() const { return m_useDeferredRendering; }

    DeferredRenderer &getDeferredRenderer() { return m_deferredRenderer; }

    glm::vec3 m_ambientLightColor;
};
