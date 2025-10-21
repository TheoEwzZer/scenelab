#include <cstdlib>
#include <functional>
#include <glad/gl.h>
#include <unordered_map>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <glm/glm.hpp>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include "renderer/interface/ARenderer.hpp"
#include <imgui.h>
#include "ImGuizmo.h"
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>

// Static pointer to store current renderer instance for callback
static ARenderer *s_currentRenderer = nullptr;

// Callback map
std::unordered_map<int,
    std::unordered_map<int, std::vector<std::function<void()>>>>
    keyCallbacks = {};
std::function<void(double, double)> cursorCallback = NULL;
static std::function<void(const std::vector<std::string> &, double, double)>
    dropCallback;

static void error_callback(int error, const char *description)
{
    (void)error;
    fprintf(stderr, "Error: %s\n", description);
}

static void key_callback(
    GLFWwindow *window, int key, int scancode, int action, int mods)
{
    (void)window;
    (void)scancode;
    (void)mods;

    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }

    for (const auto &callback : keyCallbacks[key][action]) {
        callback();
    }
}

static void mouse_callback(
    GLFWwindow *window, int button, int action, int mods)
{
    (void)window;
    (void)mods;

    for (const auto &callback : keyCallbacks[button][action]) {
        callback();
    }
}

void ARenderer::addKeyCallback(
    int key, int action, std::function<void()> callback)
{
    keyCallbacks[key][action].push_back(callback);
}

static void cursor_callback(GLFWwindow *window, double x, double y)
{
    (void)window;

    if (cursorCallback) {
        cursorCallback(x, y);
    }
}

void ARenderer::addCursorCallback(std::function<void(double, double)> callback)
{
    cursorCallback = callback;
    glfwSetCursorPosCallback(m_window, cursor_callback);
}

static void drop_callback(GLFWwindow *window, int count, const char **paths)
{
    double mx = 0.0, my = 0.0;
    if (window) {
        glfwGetCursorPos(window, &mx, &my);
    }
    if (!dropCallback) {
        return;
    }
    std::vector<std::string> dropped;
    dropped.reserve(static_cast<size_t>(count));
    for (int i = 0; i < count; ++i) {
        dropped.emplace_back(paths[i] ? paths[i] : "");
    }
    dropCallback(dropped, mx, my);
}

void ARenderer::addDropCallback(std::function<void(
        const std::vector<std::string> &paths, double mouseX, double mouseY)>
        callback)
{
    dropCallback = std::move(callback);
    glfwSetDropCallback(m_window, drop_callback);
}

ARenderer::ARenderer()
{
    // Set the current renderer instance for callbacks
    s_currentRenderer = this;

    // Window Creation

    glfwSetErrorCallback(error_callback);

    if (!glfwInit()) {
        exit(EXIT_FAILURE);
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    m_window = glfwCreateWindow(1920, 1080, "Simple example", NULL, NULL);
    if (!m_window) {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwSetKeyCallback(m_window, key_callback);
    glfwSetMouseButtonCallback(m_window, mouse_callback);

    glfwMakeContextCurrent(m_window);
    gladLoadGL(glfwGetProcAddress);
    glfwSwapInterval(1);
}

void ARenderer::init()
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    // io.ConfigFlags |= ImGuiConfigFlags_DpiEnableScaleFonts;
    // io.ConfigFlags |= ImGuiConfigFlags_DpiEnableScaleViewports;
    io.ConfigDpiScaleFonts = true;
    io.ConfigDpiScaleViewports = true;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(m_window, true);
    ImGui_ImplOpenGL3_Init("#version 330");
}

ARenderer::~ARenderer()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(m_window);

    glfwTerminate();
    exit(EXIT_SUCCESS);
}

bool ARenderer::shouldWindowClose() { return glfwWindowShouldClose(m_window); }

void ARenderer::setCameraOverlayCallback(CameraOverlayCallback callback)
{
    m_cameraOverlayCallback = std::move(callback);
}

void ARenderer::renderAllViews(CameraManager &cameraManager)
{
    for (auto &[id, view] : m_cameraViews) {
        if (auto *cam = cameraManager.getCamera(id)) {
            renderCameraViews(*cam, view);
        }
    }
    renderDockableViews(cameraManager);
    // Unlock when mouse released
    if (!ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
        m_lockCameraWindows = false;
        m_lockedCameraId = -1;
    }
}

void ARenderer::createCameraViews(const int id, int width, int height)
{
    if (!m_cameraViews.contains(id)) {
        CameraView view;
        view.size = { width, height };

        // Create and bind framebuffer
        glGenFramebuffers(1, &view.fbo);

        glBindFramebuffer(GL_FRAMEBUFFER, view.fbo);

        // Create and attach color texture
        glGenTextures(1, &view.colorTex);
        glBindTexture(GL_TEXTURE_2D, view.colorTex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA,
            GL_UNSIGNED_BYTE, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
            GL_TEXTURE_2D, view.colorTex, 0);

        // Create and attach depth-stencil buffer
        glGenRenderbuffers(1, &view.depthRBO);
        glBindRenderbuffer(GL_RENDERBUFFER, view.depthRBO);
        glRenderbufferStorage(
            GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT,
            GL_RENDERBUFFER, view.depthRBO);

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER)
            != GL_FRAMEBUFFER_COMPLETE) {
            std::cerr << "Framebuffer is incomplete!" << std::endl;
        }

        m_cameraViews[id] = std::move(view);
    }
}

void ARenderer::destroyCameraViews(const int id)
{
    if (const auto it = m_cameraViews.find(id); it != m_cameraViews.end()) {
        const auto &view = it->second;
        glDeleteFramebuffers(1, &view.fbo);
        glDeleteTextures(1, &view.colorTex);
        glDeleteRenderbuffers(1, &view.depthRBO);
        m_cameraViews.erase(it);
    }
}

void ARenderer::renderCameraViews(const Camera &cam, const CameraView &view)
{
    // Save current state
    GLint previousFBO;
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &previousFBO);
    GLint previousViewport[4];
    glGetIntegerv(GL_VIEWPORT, previousViewport);

    // Bind our framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, view.fbo);

    // Set viewport to match the view size
    glViewport(0, 0, view.size.x, view.size.y);

    // Clear the framebuffer
    glClearColor(0.4f, 0.2f, 0.2f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Update view and projection matrices
    setViewMatrix(cam.getViewMatrix());
    setProjectionMatrix(cam.getProjectionMatrix());

    // Draw scene
    drawAll();

    // Check for errors
    GLenum err = glGetError();
    if (err != GL_NO_ERROR) {
        std::cout << "OpenGL error after drawing to FBO: " << err << std::endl;
    }

    // Restore previous state
    glBindFramebuffer(GL_FRAMEBUFFER, previousFBO);
    glViewport(previousViewport[0], previousViewport[1], previousViewport[2],
        previousViewport[3]);
}

void ARenderer::renderDockableViews(CameraManager &cameraManager)
{
    for (auto &[id, view] : m_cameraViews) {
        const std::string name = "Camera " + std::to_string(id);
        ImGui::SetNextWindowSize(ImVec2(512, 512), ImGuiCond_FirstUseEver);

        ImGuiWindowFlags windowFlags = 0;
        if (m_lockCameraWindows && m_lockedCameraId == id) {
            windowFlags |= ImGuiWindowFlags_NoMove;
            windowFlags |= ImGuiWindowFlags_NoResize;
            windowFlags |= ImGuiWindowFlags_NoScrollbar;
            windowFlags |= ImGuiWindowFlags_NoScrollWithMouse;
            if (view.hasState) {
                ImGui::SetNextWindowPos(view.lastPos);
                ImGui::SetNextWindowSize(view.lastSize);
            }
        }

        ImGui::Begin(name.c_str(), nullptr, windowFlags);

        // Controls toolbar for this camera
        if (auto *cam = cameraManager.getCamera(id)) {
            ImGui::PushID(id);
            const std::string tableId = std::string("cam_ctl_") + std::to_string(id);
            if (ImGui::BeginTable(tableId.c_str(), 6, ImGuiTableFlags_SizingFixedFit)) {
                ImGui::TableNextColumn();
                if (ImGui::SmallButton("Focus")) {
                    cameraManager.setFocused(id);
                }

                ImGui::TableNextColumn();
                bool isPerspective = cam->getProjectionMode() == Camera::ProjectionMode::Perspective;
                if (ImGui::Checkbox("Persp##mode", &isPerspective)) {
                    cam->setProjectionMode(isPerspective ? Camera::ProjectionMode::Perspective : Camera::ProjectionMode::Orthographic);
                }

                ImGui::TableNextColumn();
                if (isPerspective) {
                    float fov = cam->getFov();
                    if (ImGui::DragFloat("FOV##fov", &fov, 0.1f, 10.0f, 160.0f, "%.1f")) {
                        cam->setFov(fov);
                    }
                } else {
                    float orthoSize = cam->getOrthoSize();
                    if (ImGui::DragFloat("Size##ortho", &orthoSize, 0.05f, 0.01f, 100.0f, "%.2f")) {
                        cam->setOrthoSize(orthoSize);
                    }
                }

                ImGui::TableNextColumn();

                ImGui::TableNextColumn();
                if (ImGui::SmallButton("Reset Pose##reset")) {
                    cam->setPosition(glm::vec3(0.0f, 0.0f, 3.0f));
                    cam->setRotation(0.0f, 0.0f, 0.0f);
                }
                ImGui::EndTable();
            }
            ImGui::PopID();
        }

        ImVec2 avail = ImGui::GetContentRegionAvail();
        ImVec2 windowPos = ImGui::GetWindowPos();
        int newW = static_cast<int>(avail.x);
        int newH = static_cast<int>(avail.y);
        if (newW < 2 || newH < 2) {
            newW = view.size.x;
            newH = view.size.y;
        }
        if (view.size.x != newW || view.size.y != newH) {
            view.size = { newW, newH };

            glBindFramebuffer(GL_FRAMEBUFFER, view.fbo);

            glBindTexture(GL_TEXTURE_2D, view.colorTex);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, avail.x, avail.y, 0,
                GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

            glBindRenderbuffer(GL_RENDERBUFFER, view.depthRBO);
            glRenderbufferStorage(
                GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, avail.x, avail.y);

            glBindFramebuffer(GL_FRAMEBUFFER, 0);

            if (auto *cam = cameraManager.getCamera(id)) {
                const float aspect = static_cast<float>(view.size.x)
                    / static_cast<float>(view.size.y);
                cam->setAspect(aspect);
            }
        }
        ImVec2 imagePos = ImGui::GetCursorScreenPos();
        ImGui::Image((void *)(intptr_t)view.colorTex, avail, ImVec2(0, 1),
            ImVec2(1, 0));

        // Record state for locking
        view.lastPos = windowPos;
        view.lastSize = ImGui::GetWindowSize();
        view.hasState = true;

        bool isHovered = ImGui::IsItemHovered();

        if (m_cameraOverlayCallback) {
            if (const auto *cam = cameraManager.getCamera(id)) {
                m_cameraOverlayCallback(id, *cam, imagePos, avail, isHovered);
            }
        }

        if (ImGuizmo::IsUsing() && isHovered) {
            m_lockCameraWindows = true;
            m_lockedCameraId = id;
        }

        ImGui::End();
    }
}
