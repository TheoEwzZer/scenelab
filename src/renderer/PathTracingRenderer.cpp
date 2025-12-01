#include "renderer/implementation/PathTracingRenderer.hpp"
#include "ShaderProgram.hpp"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

#include "ImGuizmo.h"
#include "imgui.h"
#include "renderer/interface/IRenderer.hpp"

#include <algorithm>
#include <iostream>
#include <vector>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/geometric.hpp>

PathTracingRenderer::PathTracingRenderer(Window &window) : m_window(window)
{

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

    m_pathTracingShader.init(
        "../assets/shaders/shader.vert", "../assets/shaders/pathtracing.frag");
    m_pathTracingShader.use();
    glm::mat4 identity = glm::mat4(1.0f);
    m_pathTracingShader.setMat4("model", identity);
    m_pathTracingShader.setMat4("view", identity);
    m_pathTracingShader.setMat4("projection", identity);
    m_pathTracingShader.setVec3("viewPos", glm::vec3(0.0f, 0.0f, 4.0f));

    // Initialize view and projection matrices
    m_viewMatrix = glm::mat4(1.0f);
    m_viewMatrix = glm::translate(m_viewMatrix, glm::vec3(0.0f, 0.0f, -4.0f));
    // default to orthographic projection
    m_projMatrix = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, 0.1f, 100.0f);
    float quadVertices[] = {
        // positions        // texCoords
        -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, // top left
        -1.0f, -1.0f, 0.0f, 0.0f, 0.0f, // bottom left
        1.0f, -1.0f, 0.0f, 1.0f, 0.0f, // bottom right

        -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, // top left
        1.0f, -1.0f, 0.0f, 1.0f, 0.0f, // bottom right
        1.0f, 1.0f, 0.0f, 1.0f, 1.0f // top right
    };
    // register in VBO, VAO etc.
    glGenVertexArrays(1, &m_quadVAO);
    glGenBuffers(1, &m_quadVBO);
    glBindVertexArray(m_quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_quadVBO);
    glBufferData(
        GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(
        0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
        (void *)(3 * sizeof(float)));
    glBindVertexArray(0);

    initAccumulationBuffers();

    m_triangles = {};

    texData.reserve(m_triangles.size() * 3 * 4);

    for (const auto &t : m_triangles) {
        texData.push_back(t.v0.x);
        texData.push_back(t.v0.y);
        texData.push_back(t.v0.z);
        texData.push_back(0.0f);

        texData.push_back(t.v1.x);
        texData.push_back(t.v1.y);
        texData.push_back(t.v1.z);
        texData.push_back(0.0f);

        texData.push_back(t.v2.x);
        texData.push_back(t.v2.y);
        texData.push_back(t.v2.z);
        texData.push_back(0.0f);
    }

    // Create geometry texture (width=3: v0, v1, v2, normal)
    glGenTextures(1, &m_triangleGeomTexture);
    glBindTexture(GL_TEXTURE_2D, m_triangleGeomTexture);
    glTexImage2D(
        GL_TEXTURE_2D, 0, GL_RGBA32F, 3, 1, 0, GL_RGBA, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // Create material texture (width=4: color, emissive, specular+ior,
    // refraction)
    glGenTextures(1, &m_triangleMaterialTexture);
    glBindTexture(GL_TEXTURE_2D, m_triangleMaterialTexture);
    glTexImage2D(
        GL_TEXTURE_2D, 0, GL_RGBA32F, 4, 1, 0, GL_RGBA, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // Create sphere geometry texture (width=1: center.xyz, radius)
    glGenTextures(1, &m_sphereGeomTexture);
    glBindTexture(GL_TEXTURE_2D, m_sphereGeomTexture);
    glTexImage2D(
        GL_TEXTURE_2D, 0, GL_RGBA32F, 1, 1, 0, GL_RGBA, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // Create sphere material texture (width=4: color, emissive, specular+ior,
    // refraction)
    glGenTextures(1, &m_sphereMaterialTexture);
    glBindTexture(GL_TEXTURE_2D, m_sphereMaterialTexture);
    glTexImage2D(
        GL_TEXTURE_2D, 0, GL_RGBA32F, 4, 1, 0, GL_RGBA, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // Create plane geometry texture (width=2: point.xyz + normal.x, normal.yz)
    glGenTextures(1, &m_planeGeomTexture);
    glBindTexture(GL_TEXTURE_2D, m_planeGeomTexture);
    glTexImage2D(
        GL_TEXTURE_2D, 0, GL_RGBA32F, 2, 1, 0, GL_RGBA, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // Create plane material texture (width=4: color, emissive, specular+ior,
    // refraction)
    glGenTextures(1, &m_planeMaterialTexture);
    glBindTexture(GL_TEXTURE_2D, m_planeMaterialTexture);
    glTexImage2D(
        GL_TEXTURE_2D, 0, GL_RGBA32F, 4, 1, 0, GL_RGBA, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    m_pathTracingShader.use();
    m_pathTracingShader.setInt("triangleGeomTex", 1);
    m_pathTracingShader.setInt("triangleMaterialTex", 2);
    m_pathTracingShader.setInt("numTriangles", 0);
    m_pathTracingShader.setInt("sphereGeomTex", 3);
    m_pathTracingShader.setInt("sphereMaterialTex", 4);
    m_pathTracingShader.setInt("numSpheres", 0);
    m_pathTracingShader.setInt("planeGeomTex", 5);
    m_pathTracingShader.setInt("planeMaterialTex", 6);
    m_pathTracingShader.setInt("numPlanes", 0);
}

PathTracingRenderer::~PathTracingRenderer()
{
    cleanupAccumulationBuffers();
    if (m_triangleGeomTexture != 0) {
        glDeleteTextures(1, &m_triangleGeomTexture);
    }
    if (m_triangleMaterialTexture != 0) {
        glDeleteTextures(1, &m_triangleMaterialTexture);
    }
    if (m_sphereGeomTexture != 0) {
        glDeleteTextures(1, &m_sphereGeomTexture);
    }
    if (m_sphereMaterialTexture != 0) {
        glDeleteTextures(1, &m_sphereMaterialTexture);
    }
    if (m_planeGeomTexture != 0) {
        glDeleteTextures(1, &m_planeGeomTexture);
    }
    if (m_planeMaterialTexture != 0) {
        glDeleteTextures(1, &m_planeMaterialTexture);
    }
}

int PathTracingRenderer::registerObject(std::unique_ptr<RenderableObject> obj)
{
    return registerObject(std::move(obj), glm::vec3(1.0f, 1.0f, 1.0f));
}

int PathTracingRenderer::registerObject(
    std::unique_ptr<RenderableObject> obj, const std::string &texturePath)
{
    (void)obj;
    (void)texturePath;
    return 0;
}

int PathTracingRenderer::registerObject(
    std::unique_ptr<RenderableObject> obj, const glm::vec3 &color)
{
    (void)color;

    int objectId;
    if (!m_freeSlots.empty()) {
        objectId = m_freeSlots.back();
        m_freeSlots.pop_back();
    } else {
        objectId = static_cast<int>(m_objects.size());
        m_objects.emplace_back();
    }

    m_objects[objectId].renderObject = std::move(obj);
    m_objects[objectId].transform = glm::mat4(1.0f);
    m_objects[objectId].triangleStartIndex = 0;
    m_objects[objectId].triangleCount = 0;

    m_trianglesDirty = true;

    return objectId;
}

int PathTracingRenderer::registerObject(
    std::unique_ptr<RenderableObject> obj, const Material &material)
{
    obj->setMaterial(material);
    return registerObject(std::move(obj), material.m_diffuseColor);
}

void PathTracingRenderer::updateTransform(
    const int objectId, const glm::mat4 &modelMatrix)
{
    if (objectId < 0 || objectId >= static_cast<int>(m_objects.size())) {
        return;
    }

    if (!m_objects[objectId].renderObject) {
        return;
    }

    m_objects[objectId].transform = modelMatrix;

    m_trianglesDirty = true;
}

void PathTracingRenderer::updateGeometry(
    int objectId, const std::vector<float> &vertices)
{
    if (objectId < 0 || objectId >= static_cast<int>(m_objects.size())) {
        return;
    }

    if (m_objects[objectId].renderObject) {
        m_objects[objectId].renderObject->updateGeometry(vertices);
        m_trianglesDirty = true;
    }
}

void PathTracingRenderer::removeObject(const int objectId)
{
    if (objectId < 0 || objectId >= static_cast<int>(m_objects.size())) {
        return;
    }

    if (!m_objects[objectId].renderObject) {
        return;
    }

    m_objects[objectId].renderObject.reset();
    m_objects[objectId].triangleStartIndex = 0;
    m_objects[objectId].triangleCount = 0;

    m_freeSlots.push_back(objectId);

    m_trianglesDirty = true;
}

std::vector<std::unique_ptr<RenderableObject>>
PathTracingRenderer::extractAllObjects()
{
    std::vector<std::unique_ptr<RenderableObject>> objects;
    objects.reserve(m_objects.size());

    for (auto &objData : m_objects) {
        if (objData.renderObject) {
            objects.push_back(std::move(objData.renderObject));
        }
    }

    m_objects.clear();
    m_freeSlots.clear();
    m_triangles.clear();
    m_trianglesDirty = true;

    return objects;
}

void PathTracingRenderer::setObjectColor(int objectId, const glm::vec3 &color)
{
    if (objectId < 0 || objectId >= static_cast<int>(m_objects.size())) {
        return;
    }
    if (!m_objects[objectId].renderObject) {
        return;
    }

    m_objects[objectId].renderObject->setColor(color);
    m_trianglesDirty = true;
}

glm::vec3 PathTracingRenderer::getObjectColor(int objectId) const
{
    if (objectId < 0 || objectId >= static_cast<int>(m_objects.size())) {
        return glm::vec3(1.0f);
    }
    if (!m_objects[objectId].renderObject) {
        return glm::vec3(1.0f);
    }

    return m_objects[objectId].renderObject->getColor();
}

void PathTracingRenderer::setObjectEmissive(
    int objectId, const glm::vec3 &emissive)
{
    if (objectId < 0 || objectId >= static_cast<int>(m_objects.size())) {
        return;
    }
    if (!m_objects[objectId].renderObject) {
        return;
    }

    m_objects[objectId].renderObject->setEmissive(emissive);
    m_trianglesDirty = true;
}

glm::vec3 PathTracingRenderer::getObjectEmissive(int objectId) const
{
    if (objectId < 0 || objectId >= static_cast<int>(m_objects.size())) {
        return glm::vec3(0.0f);
    }
    if (!m_objects[objectId].renderObject) {
        return glm::vec3(0.0f);
    }

    return m_objects[objectId].renderObject->getEmissive();
}

void PathTracingRenderer::setObjectPercentSpecular(int objectId, float percent)
{
    if (objectId < 0 || objectId >= static_cast<int>(m_objects.size())) {
        return;
    }
    if (!m_objects[objectId].renderObject) {
        return;
    }

    m_objects[objectId].renderObject->setPercentSpecular(percent);
    m_trianglesDirty = true;
}

float PathTracingRenderer::getObjectPercentSpecular(int objectId) const
{
    if (objectId < 0 || objectId >= static_cast<int>(m_objects.size())) {
        return 0.0f;
    }
    if (!m_objects[objectId].renderObject) {
        return 0.0f;
    }

    return m_objects[objectId].renderObject->getPercentSpecular();
}

void PathTracingRenderer::setObjectRoughness(int objectId, float roughness)
{
    if (objectId < 0 || objectId >= static_cast<int>(m_objects.size())) {
        return;
    }
    if (!m_objects[objectId].renderObject) {
        return;
    }

    m_objects[objectId].renderObject->setRoughness(roughness);
    m_trianglesDirty = true;
}

float PathTracingRenderer::getObjectRoughness(int objectId) const
{
    if (objectId < 0 || objectId >= static_cast<int>(m_objects.size())) {
        return 0.5f;
    }
    if (!m_objects[objectId].renderObject) {
        return 0.5f;
    }

    return m_objects[objectId].renderObject->getRoughness();
}

void PathTracingRenderer::setObjectSpecularColor(
    int objectId, const glm::vec3 &color)
{
    if (objectId < 0 || objectId >= static_cast<int>(m_objects.size())) {
        return;
    }
    if (!m_objects[objectId].renderObject) {
        return;
    }

    m_objects[objectId].renderObject->setSpecularColor(color);
    m_trianglesDirty = true;
}

glm::vec3 PathTracingRenderer::getObjectSpecularColor(int objectId) const
{
    if (objectId < 0 || objectId >= static_cast<int>(m_objects.size())) {
        return glm::vec3(1.0f);
    }
    if (!m_objects[objectId].renderObject) {
        return glm::vec3(1.0f);
    }

    return m_objects[objectId].renderObject->getSpecularColor();
}

void PathTracingRenderer::setObjectIndexOfRefraction(int objectId, float ior)
{
    if (objectId < 0 || objectId >= static_cast<int>(m_objects.size())) {
        return;
    }
    if (!m_objects[objectId].renderObject) {
        return;
    }

    m_objects[objectId].renderObject->setIndexOfRefraction(ior);
    m_trianglesDirty = true;
}

float PathTracingRenderer::getObjectIndexOfRefraction(int objectId) const
{
    if (objectId < 0 || objectId >= static_cast<int>(m_objects.size())) {
        return 1.0f;
    }
    if (!m_objects[objectId].renderObject) {
        return 1.0f;
    }

    return m_objects[objectId].renderObject->getIndexOfRefraction();
}

void PathTracingRenderer::setObjectRefractionChance(int objectId, float chance)
{
    if (objectId < 0 || objectId >= static_cast<int>(m_objects.size())) {
        return;
    }
    if (!m_objects[objectId].renderObject) {
        return;
    }

    m_objects[objectId].renderObject->setRefractionChance(chance);
    m_trianglesDirty = true;
}

float PathTracingRenderer::getObjectRefractionChance(int objectId) const
{
    if (objectId < 0 || objectId >= static_cast<int>(m_objects.size())) {
        return 0.0f;
    }
    if (!m_objects[objectId].renderObject) {
        return 0.0f;
    }

    return m_objects[objectId].renderObject->getRefractionChance();
}

void PathTracingRenderer::beginFrame()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, 1920, 1080);

    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_pathTracingShader.use();

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGuiViewport *viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->Pos);
    ImGui::SetNextWindowSize(viewport->Size);
    ImGui::SetNextWindowViewport(viewport->ID);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGuiWindowFlags window_flags
        = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
    window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse;
    window_flags |= ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
    window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus
        | ImGuiWindowFlags_NoNavFocus;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::Begin("DockSpace", nullptr, window_flags);
    ImGui::PopStyleVar();
    ImGui::PopStyleVar(2);

    ImGuiID dockspaceID = ImGui::GetID("MyDockSpace");
    ImGui::DockSpace(dockspaceID, ImVec2(0.0f, 0.0f),
        ImGuiDockNodeFlags_PassthruCentralNode);
    ImGui::End();

    ImGuizmo::BeginFrame();
    ImGuizmo::SetRect(0, 0, 1920, 1080);
}

void PathTracingRenderer::drawAll(Camera) {}

void PathTracingRenderer::endFrame()
{
    m_iFrame++;

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, 1920, 1080);
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    m_window.swapBuffers();
    m_window.pollEvents();
}

// Window-related methods
bool PathTracingRenderer::shouldWindowClose()
{
    return m_window.shouldClose();
}

void PathTracingRenderer::addKeyCallback(
    int key, int action, std::function<void()> callback)
{
    m_window.addKeyCallback(key, action, callback);
}

void PathTracingRenderer::addCursorCallback(
    std::function<void(double, double)> callback)
{
    m_window.addCursorCallback(callback);
}

void PathTracingRenderer::addDropCallback(std::function<void(
        const std::vector<std::string> &paths, double mouseX, double mouseY)>
        callback)
{
    m_window.addDropCallback(callback);
}

GLFWwindow *PathTracingRenderer::getWindow() const
{
    return m_window.getGLFWWindow();
}

// Camera view management methods
void PathTracingRenderer::setCameraOverlayCallback(
    CameraOverlayCallback callback)
{
    m_cameraOverlayCallback = std::move(callback);
}

void PathTracingRenderer::renderAllViews(CameraManager &cameraManager)
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

void PathTracingRenderer::createCameraViews(
    const int id, int width, int height)
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

        // Initialize accumulation buffers for this camera
        initCameraAccumulationBuffers(view);

        m_cameraViews[id] = std::move(view);
    }
}

void PathTracingRenderer::destroyCameraViews(const int id)
{
    if (const auto it = m_cameraViews.find(id); it != m_cameraViews.end()) {
        auto &view = it->second;
        glDeleteFramebuffers(1, &view.fbo);
        glDeleteTextures(1, &view.colorTex);
        glDeleteRenderbuffers(1, &view.depthRBO);
        cleanupCameraAccumulationBuffers(view);
        m_cameraViews.erase(it);
    }
}

void PathTracingRenderer::renderCameraViews(
    const Camera &cam, CameraView &view)
{
    // Flush deferred triangle rebuild if needed
    if (m_trianglesDirty) {
        rebuildTriangleArray();
        m_trianglesDirty = false;
        // Reset accumulation for all camera views since scene geometry changed
        for (auto &[id, cameraView] : m_cameraViews) {
            resetCameraAccumulation(cameraView);
        }
    }

    // Save current state
    GLint previousFBO;
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &previousFBO);
    GLint previousViewport[4];
    glGetIntegerv(GL_VIEWPORT, previousViewport);

    // Check if we need to reset accumulation (camera moved)
    if (shouldResetCameraAccumulation(cam, view)) {
        resetCameraAccumulation(view);
        view.lastViewPos = cam.getPosition();
        view.lastViewRotation = cam.getRotation();
    }

    // Bind the current accumulation FBO to render to
    glBindFramebuffer(
        GL_FRAMEBUFFER, view.accumulationFBO[view.currentAccumulationBuffer]);
    glViewport(0, 0, view.size.x, view.size.y);

    // Render pathtraced content
    m_pathTracingShader.use();
    m_pathTracingShader.setVec3("viewPos", cam.getPosition());

    // Precompute rotation matrix on CPU (avoids sin/cos per pixel in shader)
    glm::vec3 rot = glm::radians(cam.getRotation());
    float cp = std::cos(rot.x), sp = std::sin(rot.x); // pitch
    float cy = std::cos(rot.y), sy = std::sin(rot.y); // yaw
    float cr = std::cos(rot.z), sr = std::sin(rot.z); // roll
    glm::mat3 rotX(1, 0, 0, 0, cp, -sp, 0, sp, cp);
    glm::mat3 rotY(cy, 0, sy, 0, 1, 0, -sy, 0, cy);
    glm::mat3 rotZ(cr, -sr, 0, sr, cr, 0, 0, 0, 1);
    glm::mat3 viewRotMat = rotZ * rotY * rotX;
    m_pathTracingShader.setMat3("viewRotationMatrix", viewRotMat);
    float aspectRatio = (view.size.y > 0)
        ? (static_cast<float>(view.size.x) / static_cast<float>(view.size.y))
        : 1.0f;
    m_pathTracingShader.setFloat("aspectRatio", aspectRatio);
    // Precompute focalLength on CPU (avoids radians/tan per pixel in shader)
    float fovRadians = glm::radians(cam.getFov());
    float focalLength = 1.0f / std::tan(fovRadians * 0.5f);
    m_pathTracingShader.setFloat("focalLength", focalLength);
    m_pathTracingShader.setInt("iFrame", view.iFrame);

    // Bind the previous frame texture for accumulation
    int previousBuffer = 1 - view.currentAccumulationBuffer;
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, view.accumulationTexture[previousBuffer]);
    m_pathTracingShader.setInt("previousFrame", 0);

    // Bind geometry texture to texture unit 1
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, m_triangleGeomTexture);
    m_pathTracingShader.setInt("triangleGeomTex", 1);

    // Bind material texture to texture unit 2
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, m_triangleMaterialTexture);
    m_pathTracingShader.setInt("triangleMaterialTex", 2);

    // Bind sphere geometry texture to texture unit 3
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, m_sphereGeomTexture);
    m_pathTracingShader.setInt("sphereGeomTex", 3);

    // Bind sphere material texture to texture unit 4
    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, m_sphereMaterialTexture);
    m_pathTracingShader.setInt("sphereMaterialTex", 4);

    // Bind plane geometry texture to texture unit 5
    glActiveTexture(GL_TEXTURE5);
    glBindTexture(GL_TEXTURE_2D, m_planeGeomTexture);
    m_pathTracingShader.setInt("planeGeomTex", 5);

    // Bind plane material texture to texture unit 6
    glActiveTexture(GL_TEXTURE6);
    glBindTexture(GL_TEXTURE_2D, m_planeMaterialTexture);
    m_pathTracingShader.setInt("planeMaterialTex", 6);

    m_pathTracingShader.setInt(
        "numTriangles", static_cast<int>(m_triangles.size()));
    m_pathTracingShader.setInt(
        "numSpheres", static_cast<int>(m_spheres.size()));
    m_pathTracingShader.setInt("numPlanes", static_cast<int>(m_planes.size()));

    glBindVertexArray(m_quadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);

    // Now copy to the display framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, view.fbo);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D,
        view.accumulationTexture[view.currentAccumulationBuffer]);
    glBindVertexArray(m_quadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);

    // Swap buffers for next frame
    view.currentAccumulationBuffer = 1 - view.currentAccumulationBuffer;
    view.iFrame++;

    // Restore previous state
    glBindFramebuffer(GL_FRAMEBUFFER, previousFBO);
    glViewport(previousViewport[0], previousViewport[1], previousViewport[2],
        previousViewport[3]);
}

void PathTracingRenderer::renderDockableViews(CameraManager &cameraManager)
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
            const std::string tableId
                = std::string("cam_ctl_") + std::to_string(id);
            if (ImGui::BeginTable(
                    tableId.c_str(), 5, ImGuiTableFlags_SizingFixedFit)) {
                ImGui::TableNextColumn();
                bool isPerspective = cam->getProjectionMode()
                    == Camera::ProjectionMode::Perspective;
                if (ImGui::Checkbox("Persp##mode", &isPerspective)) {
                    cam->setProjectionMode(isPerspective
                            ? Camera::ProjectionMode::Perspective
                            : Camera::ProjectionMode::Orthographic);
                }

                ImGui::TableNextColumn();
                if (isPerspective) {
                    float fov = cam->getFov();
                    if (ImGui::DragFloat(
                            "FOV##fov", &fov, 0.1f, 10.0f, 160.0f, "%.1f")) {
                        cam->setFov(fov);
                    }
                } else {
                    float orthoSize = cam->getOrthoSize();
                    if (ImGui::DragFloat("Size##ortho", &orthoSize, 0.05f,
                            0.01f, 100.0f, "%.2f")) {
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

            // Recreate accumulation buffers with new size
            cleanupCameraAccumulationBuffers(view);
            initCameraAccumulationBuffers(view);

            if (auto *cam = cameraManager.getCamera(id)) {
                const float aspect = static_cast<float>(view.size.x)
                    / static_cast<float>(view.size.y);
                cam->setAspect(aspect);
            }
        }
        ImVec2 imagePos = ImGui::GetCursorScreenPos();
        ImGui::Image((void *)(intptr_t)view.colorTex, avail, ImVec2(0, 1),
            ImVec2(1, 0));

        // Auto focus this camera when user clicks on its image/window
        if (ImGui::IsItemClicked(ImGuiMouseButton_Left)
            || (ImGui::IsWindowHovered()
                && ImGui::IsMouseClicked(ImGuiMouseButton_Left))) {
            cameraManager.setFocused(id);
        }

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

void PathTracingRenderer::initAccumulationBuffers()
{
    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);
    int width = viewport[2];
    int height = viewport[3];

    if (width <= 0 || height <= 0) {
        width = 1920;
        height = 1080;
    }

    // Create two framebuffers and textures for ping-pong
    glGenFramebuffers(2, m_accumulationFBO);
    glGenTextures(2, m_accumulationTexture);

    for (int i = 0; i < 2; i++) {
        // Setup texture
        glBindTexture(GL_TEXTURE_2D, m_accumulationTexture[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA,
            GL_FLOAT, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        // Setup framebuffer
        glBindFramebuffer(GL_FRAMEBUFFER, m_accumulationFBO[i]);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
            GL_TEXTURE_2D, m_accumulationTexture[i], 0);

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER)
            != GL_FRAMEBUFFER_COMPLETE) {
            std::cerr << "Accumulation framebuffer " << i << " is incomplete!"
                      << std::endl;
        }
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Clear both textures to black
    for (int i = 0; i < 2; i++) {
        glBindFramebuffer(GL_FRAMEBUFFER, m_accumulationFBO[i]);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void PathTracingRenderer::cleanupAccumulationBuffers()
{
    glDeleteFramebuffers(2, m_accumulationFBO);
    glDeleteTextures(2, m_accumulationTexture);
}

void PathTracingRenderer::resetAccumulation()
{
    m_iFrame = 0;

    // Clear both accumulation buffers
    for (int i = 0; i < 2; i++) {
        glBindFramebuffer(GL_FRAMEBUFFER, m_accumulationFBO[i]);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

bool PathTracingRenderer::shouldResetAccumulation(const Camera &cam) const
{
    const float epsilon = 0.0001f;

    glm::vec3 currentPos = cam.getPosition();
    glm::vec3 currentRot = cam.getRotation();

    // Check if camera position or rotation has changed
    if (glm::length(currentPos - m_lastViewPos) > epsilon) {
        return true;
    }

    if (glm::length(currentRot - m_lastViewRotation) > epsilon) {
        return true;
    }

    return false;
}

void PathTracingRenderer::initCameraAccumulationBuffers(CameraView &view)
{
    int width = view.size.x;
    int height = view.size.y;

    if (width <= 0 || height <= 0) {
        width = 512;
        height = 512;
    }

    // Create two framebuffers and textures for ping-pong
    glGenFramebuffers(2, view.accumulationFBO);
    glGenTextures(2, view.accumulationTexture);

    for (int i = 0; i < 2; i++) {
        // Setup texture
        glBindTexture(GL_TEXTURE_2D, view.accumulationTexture[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA,
            GL_FLOAT, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        // Setup framebuffer
        glBindFramebuffer(GL_FRAMEBUFFER, view.accumulationFBO[i]);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
            GL_TEXTURE_2D, view.accumulationTexture[i], 0);

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER)
            != GL_FRAMEBUFFER_COMPLETE) {
            std::cerr << "Camera accumulation framebuffer " << i
                      << " is incomplete!" << std::endl;
        }
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Clear both textures to black
    for (int i = 0; i < 2; i++) {
        glBindFramebuffer(GL_FRAMEBUFFER, view.accumulationFBO[i]);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void PathTracingRenderer::cleanupCameraAccumulationBuffers(CameraView &view)
{
    if (view.accumulationFBO[0] != 0 || view.accumulationFBO[1] != 0) {
        glDeleteFramebuffers(2, view.accumulationFBO);
        view.accumulationFBO[0] = 0;
        view.accumulationFBO[1] = 0;
    }
    if (view.accumulationTexture[0] != 0 || view.accumulationTexture[1] != 0) {
        glDeleteTextures(2, view.accumulationTexture);
        view.accumulationTexture[0] = 0;
        view.accumulationTexture[1] = 0;
    }
}

void PathTracingRenderer::resetCameraAccumulation(CameraView &view)
{
    view.iFrame = 0;

    // Clear both accumulation buffers
    for (int i = 0; i < 2; i++) {
        glBindFramebuffer(GL_FRAMEBUFFER, view.accumulationFBO[i]);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

bool PathTracingRenderer::shouldResetCameraAccumulation(
    const Camera &cam, CameraView &view) const
{
    const float epsilon = 0.0001f;

    glm::vec3 currentPos = cam.getPosition();
    glm::vec3 currentRot = cam.getRotation();

    // Check if camera position or rotation has changed
    if (glm::length(currentPos - view.lastViewPos) > epsilon) {
        return true;
    }

    if (glm::length(currentRot - view.lastViewRotation) > epsilon) {
        return true;
    }

    return false;
}

void PathTracingRenderer::rebuildTriangleArray()
{
    m_triangles.clear();
    m_spheres.clear();
    m_planes.clear();

    for (auto &objData : m_objects) {
        if (!objData.renderObject) {
            objData.triangleStartIndex = 0;
            objData.triangleCount = 0;
            continue;
        }

        PrimitiveType primType = objData.renderObject->getPrimitiveType();
        glm::vec3 color = objData.renderObject->getColor();
        glm::vec3 emissive = objData.renderObject->getEmissive();
        float percentSpecular = objData.renderObject->getPercentSpecular();
        float roughness = objData.renderObject->getRoughness();
        glm::vec3 specularColor = objData.renderObject->getSpecularColor();
        float indexOfRefraction = objData.renderObject->getIndexOfRefraction();
        float refractionChance = objData.renderObject->getRefractionChance();

        if (primType == PrimitiveType::Sphere) {
            // Extract sphere center from transform matrix
            glm::vec4 center4
                = objData.transform * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
            glm::vec3 center = glm::vec3(center4) / center4.w;

            // Extract scale to adjust radius (assuming uniform scale)
            glm::vec3 scaleVec(glm::length(glm::vec3(objData.transform[0])),
                glm::length(glm::vec3(objData.transform[1])),
                glm::length(glm::vec3(objData.transform[2])));
            float scale = (scaleVec.x + scaleVec.y + scaleVec.z) / 3.0f;
            float radius = objData.renderObject->getSphereRadius() * scale;

            AnalyticalSphereData s;
            s.center = center;
            s.radius = radius;
            s.color = color;
            s.emissive = emissive;
            s.percentSpecular = percentSpecular;
            s.roughness = roughness;
            s.specularColor = specularColor;
            s.indexOfRefraction = indexOfRefraction;
            s.refractionChance = refractionChance;
            m_spheres.push_back(s);

            objData.triangleStartIndex = 0;
            objData.triangleCount = 0;
        } else if (primType == PrimitiveType::Plane) {
            // Extract plane point from transform matrix
            glm::vec4 point4
                = objData.transform * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
            glm::vec3 point = glm::vec3(point4) / point4.w;

            // Transform the normal by the rotation part of the matrix
            glm::mat3 rotMat = glm::mat3(objData.transform);
            glm::vec3 localNormal = objData.renderObject->getPlaneNormal();
            glm::vec3 worldNormal = glm::normalize(rotMat * localNormal);

            AnalyticalPlaneData p;
            p.point = point;
            p.normal = worldNormal;
            p.color = color;
            p.emissive = emissive;
            p.percentSpecular = percentSpecular;
            p.roughness = roughness;
            p.specularColor = specularColor;
            p.indexOfRefraction = indexOfRefraction;
            p.refractionChance = refractionChance;
            m_planes.push_back(p);

            objData.triangleStartIndex = 0;
            objData.triangleCount = 0;
        } else {
            // Mesh - convert to triangles as before
            objData.triangleStartIndex = static_cast<int>(m_triangles.size());
            std::vector<float> vertices = objData.renderObject->getVertices();

            // Vertices are in format: x,y,z, u,v, nx,ny,nz per vertex (8
            // floats) Each triangle has 3 vertices, so 24 floats per triangle
            int stride = 8; // position (3) + texcoord (2) + normal (3)

            for (size_t i = 0; i < vertices.size(); i += stride * 3) {
                if (i + stride * 3 > vertices.size()) {
                    break;
                }

                Triangle t;

                glm::vec4 v0 = objData.transform
                    * glm::vec4(
                        vertices[i], vertices[i + 1], vertices[i + 2], 1.0f);
                glm::vec4 v1 = objData.transform
                    * glm::vec4(vertices[i + stride], vertices[i + stride + 1],
                        vertices[i + stride + 2], 1.0f);
                glm::vec4 v2 = objData.transform
                    * glm::vec4(vertices[i + stride * 2],
                        vertices[i + stride * 2 + 1],
                        vertices[i + stride * 2 + 2], 1.0f);

                t.v0 = glm::vec3(v0) / v0.w;
                t.v1 = glm::vec3(v1) / v1.w;
                t.v2 = glm::vec3(v2) / v2.w;

                // Precompute face normal
                glm::vec3 e0 = t.v1 - t.v0;
                glm::vec3 e1 = t.v0 - t.v2;
                t.normal = glm::normalize(glm::cross(e1, e0));

                t.color = color;
                t.emissive = emissive;
                t.percentSpecular = percentSpecular;
                t.roughness = roughness;
                t.specularColor = specularColor;
                t.indexOfRefraction = indexOfRefraction;
                t.refractionChance = refractionChance;

                m_triangles.push_back(t);
            }

            objData.triangleCount = static_cast<int>(m_triangles.size())
                - objData.triangleStartIndex;
        }
    }

    // Create separate geometry and material texture data
    // Geometry texture (width=3): v0, v1, v2, normal - used for all
    // intersection tests Material texture (width=4): color, emissive,
    // specular+ior, refraction - only for closest hit
    std::vector<float> geomData;
    std::vector<float> materialData;
    geomData.reserve(m_triangles.size() * 3 * 4);
    materialData.reserve(m_triangles.size() * 4 * 4);

    for (const auto &t : m_triangles) {
        // Geometry texture: 3 pixels per triangle
        // Pixel 0: [v0.xyz, v1.x]
        geomData.push_back(t.v0.x);
        geomData.push_back(t.v0.y);
        geomData.push_back(t.v0.z);
        geomData.push_back(t.v1.x);

        // Pixel 1: [v1.yz, v2.xy]
        geomData.push_back(t.v1.y);
        geomData.push_back(t.v1.z);
        geomData.push_back(t.v2.x);
        geomData.push_back(t.v2.y);

        // Pixel 2: [v2.z, normal.xyz]
        geomData.push_back(t.v2.z);
        geomData.push_back(t.normal.x);
        geomData.push_back(t.normal.y);
        geomData.push_back(t.normal.z);

        // Material texture: 4 pixels per triangle
        // Pixel 0: [color.xyz, percentSpecular]
        materialData.push_back(t.color.x);
        materialData.push_back(t.color.y);
        materialData.push_back(t.color.z);
        materialData.push_back(t.percentSpecular);

        // Pixel 1: [emissive.xyz, roughness]
        materialData.push_back(t.emissive.x);
        materialData.push_back(t.emissive.y);
        materialData.push_back(t.emissive.z);
        materialData.push_back(t.roughness);

        // Pixel 2: [specularColor.xyz, indexOfRefraction]
        materialData.push_back(t.specularColor.x);
        materialData.push_back(t.specularColor.y);
        materialData.push_back(t.specularColor.z);
        materialData.push_back(t.indexOfRefraction);

        // Pixel 3: [refractionChance, padding, padding, padding]
        materialData.push_back(t.refractionChance);
        materialData.push_back(0.0f);
        materialData.push_back(0.0f);
        materialData.push_back(0.0f);
    }

    int height = std::max(1, static_cast<int>(m_triangles.size()));
    bool needsReallocation = (height != m_lastTriangleTextureHeight);

    // Upload geometry texture
    glBindTexture(GL_TEXTURE_2D, m_triangleGeomTexture);
    if (needsReallocation) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, 3, height, 0, GL_RGBA,
            GL_FLOAT, geomData.empty() ? nullptr : geomData.data());
    } else if (!geomData.empty()) {
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 3, height, GL_RGBA, GL_FLOAT,
            geomData.data());
    }

    // Upload material texture (width=4)
    glBindTexture(GL_TEXTURE_2D, m_triangleMaterialTexture);
    if (needsReallocation) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, 4, height, 0, GL_RGBA,
            GL_FLOAT, materialData.empty() ? nullptr : materialData.data());
        m_lastTriangleTextureHeight = height;
    } else if (!materialData.empty()) {
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 4, height, GL_RGBA, GL_FLOAT,
            materialData.data());
    }

    // Build sphere texture data
    std::vector<float> sphereGeomData;
    std::vector<float> sphereMaterialData;
    sphereGeomData.reserve(m_spheres.size() * 4);
    sphereMaterialData.reserve(m_spheres.size() * 4 * 4);

    for (const auto &s : m_spheres) {
        // Geometry: 1 pixel per sphere [center.xyz, radius]
        sphereGeomData.push_back(s.center.x);
        sphereGeomData.push_back(s.center.y);
        sphereGeomData.push_back(s.center.z);
        sphereGeomData.push_back(s.radius);

        // Material: 4 pixels per sphere
        // Pixel 0: [color.xyz, percentSpecular]
        sphereMaterialData.push_back(s.color.x);
        sphereMaterialData.push_back(s.color.y);
        sphereMaterialData.push_back(s.color.z);
        sphereMaterialData.push_back(s.percentSpecular);

        // Pixel 1: [emissive.xyz, roughness]
        sphereMaterialData.push_back(s.emissive.x);
        sphereMaterialData.push_back(s.emissive.y);
        sphereMaterialData.push_back(s.emissive.z);
        sphereMaterialData.push_back(s.roughness);

        // Pixel 2: [specularColor.xyz, indexOfRefraction]
        sphereMaterialData.push_back(s.specularColor.x);
        sphereMaterialData.push_back(s.specularColor.y);
        sphereMaterialData.push_back(s.specularColor.z);
        sphereMaterialData.push_back(s.indexOfRefraction);

        // Pixel 3: [refractionChance, padding, padding, padding]
        sphereMaterialData.push_back(s.refractionChance);
        sphereMaterialData.push_back(0.0f);
        sphereMaterialData.push_back(0.0f);
        sphereMaterialData.push_back(0.0f);
    }

    int sphereHeight = std::max(1, static_cast<int>(m_spheres.size()));
    bool sphereNeedsRealloc = (sphereHeight != m_lastSphereTextureHeight);

    // Upload sphere geometry texture
    glBindTexture(GL_TEXTURE_2D, m_sphereGeomTexture);
    if (sphereNeedsRealloc) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, 1, sphereHeight, 0, GL_RGBA,
            GL_FLOAT,
            sphereGeomData.empty() ? nullptr : sphereGeomData.data());
    } else if (!sphereGeomData.empty()) {
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 1, sphereHeight, GL_RGBA,
            GL_FLOAT, sphereGeomData.data());
    }

    // Upload sphere material texture (width=4)
    glBindTexture(GL_TEXTURE_2D, m_sphereMaterialTexture);
    if (sphereNeedsRealloc) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, 4, sphereHeight, 0, GL_RGBA,
            GL_FLOAT,
            sphereMaterialData.empty() ? nullptr : sphereMaterialData.data());
        m_lastSphereTextureHeight = sphereHeight;
    } else if (!sphereMaterialData.empty()) {
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 4, sphereHeight, GL_RGBA,
            GL_FLOAT, sphereMaterialData.data());
    }

    // Build plane texture data
    std::vector<float> planeGeomData;
    std::vector<float> planeMaterialData;
    planeGeomData.reserve(m_planes.size() * 2 * 4);
    planeMaterialData.reserve(m_planes.size() * 4 * 4);

    for (const auto &p : m_planes) {
        // Geometry: 2 pixels per plane
        // Pixel 0: [point.xyz, normal.x]
        planeGeomData.push_back(p.point.x);
        planeGeomData.push_back(p.point.y);
        planeGeomData.push_back(p.point.z);
        planeGeomData.push_back(p.normal.x);

        // Pixel 1: [normal.yz, padding, padding]
        planeGeomData.push_back(p.normal.y);
        planeGeomData.push_back(p.normal.z);
        planeGeomData.push_back(0.0f);
        planeGeomData.push_back(0.0f);

        // Material: 4 pixels per plane
        // Pixel 0: [color.xyz, percentSpecular]
        planeMaterialData.push_back(p.color.x);
        planeMaterialData.push_back(p.color.y);
        planeMaterialData.push_back(p.color.z);
        planeMaterialData.push_back(p.percentSpecular);

        // Pixel 1: [emissive.xyz, roughness]
        planeMaterialData.push_back(p.emissive.x);
        planeMaterialData.push_back(p.emissive.y);
        planeMaterialData.push_back(p.emissive.z);
        planeMaterialData.push_back(p.roughness);

        // Pixel 2: [specularColor.xyz, indexOfRefraction]
        planeMaterialData.push_back(p.specularColor.x);
        planeMaterialData.push_back(p.specularColor.y);
        planeMaterialData.push_back(p.specularColor.z);
        planeMaterialData.push_back(p.indexOfRefraction);

        // Pixel 3: [refractionChance, padding, padding, padding]
        planeMaterialData.push_back(p.refractionChance);
        planeMaterialData.push_back(0.0f);
        planeMaterialData.push_back(0.0f);
        planeMaterialData.push_back(0.0f);
    }

    int planeHeight = std::max(1, static_cast<int>(m_planes.size()));
    bool planeNeedsRealloc = (planeHeight != m_lastPlaneTextureHeight);

    // Upload plane geometry texture
    glBindTexture(GL_TEXTURE_2D, m_planeGeomTexture);
    if (planeNeedsRealloc) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, 2, planeHeight, 0, GL_RGBA,
            GL_FLOAT, planeGeomData.empty() ? nullptr : planeGeomData.data());
    } else if (!planeGeomData.empty()) {
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 2, planeHeight, GL_RGBA,
            GL_FLOAT, planeGeomData.data());
    }

    // Upload plane material texture (width=4)
    glBindTexture(GL_TEXTURE_2D, m_planeMaterialTexture);
    if (planeNeedsRealloc) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, 4, planeHeight, 0, GL_RGBA,
            GL_FLOAT,
            planeMaterialData.empty() ? nullptr : planeMaterialData.data());
        m_lastPlaneTextureHeight = planeHeight;
    } else if (!planeMaterialData.empty()) {
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 4, planeHeight, GL_RGBA,
            GL_FLOAT, planeMaterialData.data());
    }

    m_pathTracingShader.use();
    m_pathTracingShader.setInt("triangleGeomTex", 1);
    m_pathTracingShader.setInt("triangleMaterialTex", 2);
    m_pathTracingShader.setInt(
        "numTriangles", static_cast<int>(m_triangles.size()));
    m_pathTracingShader.setInt("sphereGeomTex", 3);
    m_pathTracingShader.setInt("sphereMaterialTex", 4);
    m_pathTracingShader.setInt(
        "numSpheres", static_cast<int>(m_spheres.size()));
    m_pathTracingShader.setInt("planeGeomTex", 5);
    m_pathTracingShader.setInt("planeMaterialTex", 6);
    m_pathTracingShader.setInt("numPlanes", static_cast<int>(m_planes.size()));
}

void PathTracingRenderer::setToneMappingMode(ToneMappingMode mode)
{
    m_toneMappingMode = mode;
}

void PathTracingRenderer::setToneMappingExposure(float exposure)
{
    m_toneMappingExposure = std::clamp(exposure, 0.01f, 20.0f);
}

void PathTracingRenderer::setActiveCubemap(int cubemapHandle)
{
    m_textureLibrary.setActiveCubemap(cubemapHandle);
}

const std::vector<int> &PathTracingRenderer::getCubemapHandles() const
{
    return m_textureLibrary.getCubemapHandles();
}

const TextureResource *PathTracingRenderer::getTextureResource(
    int handle) const
{
    return m_textureLibrary.getTextureResource(handle);
}
