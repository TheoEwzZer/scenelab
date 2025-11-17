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

struct ImVec2;

class ARenderer {
public:
    ARenderer();
    virtual ~ARenderer();

    ARenderer(const ARenderer &) = delete;
    ARenderer &operator=(const ARenderer &) = delete;

    void init();

    // Object Related
    virtual int registerObject(std::unique_ptr<RenderableObject> obj) = 0;
    virtual int registerObject(
        std::unique_ptr<RenderableObject> obj, const std::string &texturePath)
        = 0;
    virtual int registerObject(
        std::unique_ptr<RenderableObject> obj, const glm::vec3 &color)
        = 0;
    virtual void updateTransform(int objectId, const glm::mat4 &modelMatrix)
        = 0;
    virtual void removeObject(int objectId) = 0;
    virtual void drawBoundingBox(
        int objectId, const glm::vec3 &corner1, const glm::vec3 &corner2)
        = 0;

    // Camera Related
    virtual void setViewMatrix(const glm::mat4 &view) = 0;
    virtual void setProjectionMatrix(const glm::mat4 &proj) = 0;
    virtual void setProjectionMode(Camera::ProjectionMode mode) = 0;

    // Rendering Related
    virtual void drawAll() = 0;
    virtual void beginFrame() = 0;
    virtual void endFrame() = 0;

    void createCameraViews(int id, int width = 512, int height = 512);
    void destroyCameraViews(int id);
    void renderAllViews(CameraManager &cameraManager);
    using CameraOverlayCallback = std::function<void(int, const Camera &,
        ImVec2 imagePos, ImVec2 imageSize, bool isHovered)>;
    using BoundingBoxDrawCallback = std::function<void()>;
    void setCameraOverlayCallback(CameraOverlayCallback callback);
    void setBoundingBoxDrawCallback(BoundingBoxDrawCallback callback);

    // Abstract
    virtual bool shouldWindowClose();
    void addKeyCallback(int key, int action, std::function<void()> callback);
    void addCursorCallback(std::function<void(double, double)> callback);
    void addDropCallback(
        std::function<void(const std::vector<std::string> &paths,
            double mouseX, double mouseY)>
            callback);

    GLFWwindow *getWindow() const { return m_window; }

protected:
    struct CameraView {
        unsigned int fbo = 0;
        unsigned int colorTex = 0;
        unsigned int depthRBO = 0;
        glm::ivec2 size = { 512, 512 };
        std::string name = "camera";
        ImVec2 lastPos = ImVec2(0.0f, 0.0f);
        ImVec2 lastSize = ImVec2(512.0f, 512.0f);
        bool hasState = false;
    };

    std::unordered_map<int, CameraView> m_cameraViews;
    CameraOverlayCallback m_cameraOverlayCallback;
    BoundingBoxDrawCallback m_bboxDrawCallback;
    bool m_lockCameraWindows = false;
    int m_lockedCameraId = -1;

    void renderCameraViews(const Camera &cam, const CameraView &view);
    void renderDockableViews(CameraManager &cameraManager);
    GLFWwindow *m_window;

private:
};
