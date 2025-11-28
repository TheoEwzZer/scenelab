#pragma once

#include <cstdlib>
#include <functional>
#include <glad/gl.h>
#include <string>
#include <vector>
#include <unordered_map>
#define GLFW_INCLUDE_NONE
#include "CameraManager.hpp"
#include "Camera.hpp"
#include <GLFW/glfw3.h>

#include <stdlib.h>
#include <stdio.h>
#include <glm/glm.hpp>
#include <memory>
#include <imgui.h>

#include "RenderableObject.hpp"
#include "objects/Material.hpp"

struct ImVec2;

// Interface for renderers
class IRenderer {
public:
    virtual ~IRenderer() = default;

    IRenderer(const IRenderer &) = delete;
    IRenderer &operator=(const IRenderer &) = delete;

    // Object Related
    virtual int registerObject(std::unique_ptr<RenderableObject> obj) = 0;
    virtual int registerObject(
        std::unique_ptr<RenderableObject> obj, const std::string &texturePath)
        = 0;
    virtual int registerObject(
        std::unique_ptr<RenderableObject> obj, const glm::vec3 &color)
        = 0;
    virtual int registerObject(
        std::unique_ptr<RenderableObject> obj, const Material &material)
        = 0;
    virtual void updateTransform(int objectId, const glm::mat4 &modelMatrix)
        = 0;
    virtual void removeObject(int objectId) = 0;
    virtual void drawBoundingBox(
        int objectId, const glm::vec3 &corner1, const glm::vec3 &corner2)
        = 0;
    virtual std::vector<std::unique_ptr<RenderableObject>> extractAllObjects()
        = 0;

    // Material property accessors
    virtual void setObjectColor(int objectId, const glm::vec3 &color) = 0;
    virtual glm::vec3 getObjectColor(int objectId) const = 0;
    virtual void setObjectEmissive(int objectId, const glm::vec3 &emissive)
        = 0;
    virtual glm::vec3 getObjectEmissive(int objectId) const = 0;
    virtual void setObjectPercentSpecular(int objectId, float percent) = 0;
    virtual float getObjectPercentSpecular(int objectId) const = 0;
    virtual void setObjectRoughness(int objectId, float roughness) = 0;
    virtual float getObjectRoughness(int objectId) const = 0;
    virtual void setObjectSpecularColor(int objectId, const glm::vec3 &color)
        = 0;
    virtual void setObjectMaterial(int objectId, const Material &mat)
        = 0;
    virtual glm::vec3 getObjectSpecularColor(int objectId) const = 0;
    virtual void setObjectIndexOfRefraction(int objectId, float ior) = 0;
    virtual float getObjectIndexOfRefraction(int objectId) const = 0;
    virtual void setObjectRefractionChance(int objectId, float chance) = 0;
    virtual float getObjectRefractionChance(int objectId) const = 0;

    // Camera Related
    virtual void setViewMatrix(const glm::mat4 &view) = 0;
    virtual void setProjectionMatrix(const glm::mat4 &proj) = 0;
    virtual void setProjectionMode(Camera::ProjectionMode mode) = 0;

    // Rendering Related
    virtual void drawAll(Camera cam) = 0;
    virtual void beginFrame() = 0;
    virtual void endFrame() = 0;

    // Window Related
    virtual bool shouldWindowClose() = 0;
    virtual void addKeyCallback(
        int key, int action, std::function<void()> callback)
        = 0;
    virtual void addCursorCallback(
        std::function<void(double, double)> callback)
        = 0;
    virtual void addDropCallback(
        std::function<void(const std::vector<std::string> &paths,
            double mouseX, double mouseY)>
            callback)
        = 0;
    virtual GLFWwindow *getWindow() const = 0;

    // Camera View Management
    virtual void createCameraViews(int id, int width = 512, int height = 512)
        = 0;
    virtual void destroyCameraViews(int id) = 0;
    virtual void renderAllViews(CameraManager &cameraManager) = 0;

    using CameraOverlayCallback = std::function<void(int, const Camera &,
        ImVec2 imagePos, ImVec2 imageSize, bool isHovered)>;
    using BoundingBoxDrawCallback = std::function<void()>;

    virtual void setCameraOverlayCallback(CameraOverlayCallback callback) = 0;
    virtual void setBoundingBoxDrawCallback(BoundingBoxDrawCallback callback)
        = 0;

protected:
    IRenderer() = default;

    struct CameraView {
        unsigned int fbo = 0;
        unsigned int colorTex = 0;
        unsigned int depthRBO = 0;
        glm::ivec2 size = { 512, 512 };
        std::string name = "camera";
        ImVec2 lastPos = ImVec2(0.0f, 0.0f);
        ImVec2 lastSize = ImVec2(512.0f, 512.0f);
        bool hasState = false;

        // Per-camera accumulation buffers
        unsigned int accumulationFBO[2] = { 0, 0 };
        unsigned int accumulationTexture[2] = { 0, 0 };
        int currentAccumulationBuffer = 0;
        int iFrame = 0;
        glm::vec3 lastViewPos = glm::vec3(0.0f);
        glm::vec3 lastViewRotation = glm::vec3(0.0f);
    };
};
