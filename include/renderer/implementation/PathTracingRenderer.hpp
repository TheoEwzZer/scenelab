
#pragma once

#include "../interface/IRenderer.hpp"
#include "Camera.hpp"
#include "renderer/Window.hpp"
#include "ShaderProgram.hpp"
#include "renderer/TextureLibrary.hpp"
#include "renderer/implementation/RasterizationRenderer.hpp"
#include <array>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <string>
#include <vector>
#include <unordered_map>

#include <memory>

struct Triangle {
    glm::vec3 v0, v1, v2;
    glm::vec3 normal;
    glm::vec3 color;
    glm::vec3 emissive;
    float percentSpecular;
    float roughness;
    glm::vec3 specularColor;
    float indexOfRefraction;
    float refractionChance;
};

struct AnalyticalSphereData {
    glm::vec3 center;
    float radius;
    glm::vec3 color;
    glm::vec3 emissive;
    float percentSpecular;
    float roughness;
    glm::vec3 specularColor;
    float indexOfRefraction;
    float refractionChance;
};

struct AnalyticalPlaneData {
    glm::vec3 point;
    glm::vec3 normal;
    glm::vec3 color;
    glm::vec3 emissive;
    float percentSpecular;
    float roughness;
    glm::vec3 specularColor;
    float indexOfRefraction;
    float refractionChance;
};

class PathTracingRenderer : public IRenderer {
private:
    Window &m_window;

    int m_iFrame = 0;
    glm::mat4 m_viewMatrix { 1.0f };
    glm::mat4 m_projMatrix { 1.0f };
    Camera::ProjectionMode m_projectionMode {
        Camera::ProjectionMode::Perspective
    };

    std::vector<float> texData;

    std::vector<Triangle> m_triangles;
    GLuint m_triangleGeomTexture = 0;
    GLuint m_triangleMaterialTexture = 0;
    int m_lastTriangleTextureHeight = 0;

    std::vector<AnalyticalSphereData> m_spheres;
    GLuint m_sphereGeomTexture = 0;
    GLuint m_sphereMaterialTexture = 0;
    int m_lastSphereTextureHeight = 0;

    std::vector<AnalyticalPlaneData> m_planes;
    GLuint m_planeGeomTexture = 0;
    GLuint m_planeMaterialTexture = 0;
    int m_lastPlaneTextureHeight = 0;

    struct ObjectData {
        std::unique_ptr<RenderableObject> renderObject;
        glm::mat4 transform;
        int triangleStartIndex;
        int triangleCount;
    };

    std::vector<ObjectData> m_objects;
    bool m_trianglesDirty = false;

    void rebuildTriangleArray();

    // Main view accumulation buffers
    unsigned int m_accumulationFBO[2] = { 0, 0 };
    unsigned int m_accumulationTexture[2] = { 0, 0 };
    int m_currentAccumulationBuffer = 0;
    glm::vec3 m_lastViewPos = glm::vec3(0.0f);
    glm::vec3 m_lastViewRotation = glm::vec3(0.0f);

    void initAccumulationBuffers();
    void cleanupAccumulationBuffers();
    void resetAccumulation();
    bool shouldResetAccumulation(const Camera &cam) const;

    // Per-camera accumulation methods
    void initCameraAccumulationBuffers(CameraView &view);
    void cleanupCameraAccumulationBuffers(CameraView &view);
    void resetCameraAccumulation(CameraView &view);
    bool shouldResetCameraAccumulation(
        const Camera &cam, CameraView &view) const;

    ShaderProgram m_pathTracingShader;

    std::vector<std::unique_ptr<RenderableObject>> m_renderObjects;
    std::vector<int> m_freeSlots;

    TextureLibrary m_textureLibrary;

    unsigned int m_quadVBO, m_quadVAO;

    ToneMappingMode m_toneMappingMode = ToneMappingMode::Reinhard;
    float m_toneMappingExposure = 1.0f;

    std::unordered_map<int, CameraView> m_cameraViews;
    CameraOverlayCallback m_cameraOverlayCallback;
    BoundingBoxDrawCallback m_bboxDrawCallback;
    bool m_lockCameraWindows = false;
    int m_lockedCameraId = -1;

    void renderCameraViews(const Camera &cam, CameraView &view);
    void renderDockableViews(CameraManager &cameraManager);

public:
    static constexpr std::array<const char *, 5> FILTER_MODE_LABELS { "None",
        "Grayscale", "Sharpen", "Edge Detect", "Blur" };
    static constexpr std::array<const char *, 3> TONEMAP_LABELS { "Off",
        "Reinhard", "ACES" };

    explicit PathTracingRenderer(Window &window);
    virtual ~PathTracingRenderer() override;

    PathTracingRenderer(const PathTracingRenderer &) = delete;
    PathTracingRenderer &operator=(const PathTracingRenderer &) = delete;

    // Object Related
    int registerObject(std::unique_ptr<RenderableObject> obj) override;
    int registerObject(std::unique_ptr<RenderableObject> obj,
        const std::string &texturePath) override;
    int registerObject(std::unique_ptr<RenderableObject> obj,
        const glm::vec3 &color) override;
    int registerObject(std::unique_ptr<RenderableObject> obj,
        const Material &material) override;
    void updateTransform(int objectId, const glm::mat4 &modelMatrix) override;
    void removeObject(int objectId) override;

    void drawBoundingBox(int, const glm::vec3 &, const glm::vec3 &) override {}

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

    void setBoundingBoxDrawCallback(BoundingBoxDrawCallback) override {}

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
    void assignTextureToObject(int objectId, const std::string &texturePath);
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
