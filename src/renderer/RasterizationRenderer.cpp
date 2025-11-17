
#include "renderer/implementation/RasterizationRenderer.hpp"
#include "ShaderProgram.hpp"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

#include "ImGuizmo.h"
#include "imgui.h"
#include "renderer/interface/ARenderer.hpp"

#include <algorithm>
#include <cstddef>
#include <cmath>
#include <iostream>
#include <limits>
#include <vector>

#include <glm/gtc/matrix_transform.hpp>
#include "objects/Light.hpp"

namespace {
constexpr glm::vec3 DEFAULT_LIGHT_COLOR { 1.0f, 1.0f, 1.0f };
constexpr glm::vec3 DEFAULT_LIGHT_POS { 2.0f, 0.0f, 0.0f };
}

RasterizationRenderer::RasterizationRenderer()
{
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

    m_lightingShader.init(
        "../assets/shaders/shader.vert", "../assets/shaders/lighting.frag");
    m_lightingShader.use();
    m_lightingShader.setVec3("lightColor", DEFAULT_LIGHT_COLOR);
    m_lightingShader.setVec3("lightPos", DEFAULT_LIGHT_POS);
    m_lightingShader.setInt("ourTexture", 0);
    m_lightingShader.setInt("filterMode", 0);
    m_lightingShader.setVec2("texelSize", glm::vec2(1.0f));
    m_lightingShader.setInt(
        "toneMappingMode", static_cast<int>(m_toneMappingMode));
    m_lightingShader.setFloat("toneExposure", m_toneMappingExposure);

    m_pointLightShader.init(
        "../assets/shaders/shader.vert", "../assets/shaders/pointLight.frag");
    m_vectorialShader.init(
        "../assets/shaders/shader_vect.vert", "../assets/shaders/vect.frag");
    m_vectorialShader.use();
    m_vectorialShader.setInt("ourTexture", 0);
    m_vectorialShader.setBool("useTexture", false);
    m_vectorialShader.setInt("filterMode", 0);
    m_vectorialShader.setVec2("texelSize", glm::vec2(0.0f));
    m_bboxShader.init(
        "../assets/shaders/shader.vert", "../assets/shaders/bbox.frag");
    m_skyboxShader.init(
        "../assets/shaders/skybox.vert", "../assets/shaders/skybox.frag");
    m_skyboxShader.use();
    m_skyboxShader.setInt("skybox", 0);

    // Initialize view and projection matrices
    m_viewMatrix = glm::mat4(1.0f);
    m_viewMatrix = glm::translate(m_viewMatrix, glm::vec3(0.0f, 0.0f, -4.0f));
    m_projMatrix = glm::perspective(
        glm::radians(45.0f), 1920.0f / 1080.0f, 0.1f, 100.0f);

    initializeSkyboxGeometry();
    m_textureLibrary.ensureDefaultTextures();
    m_textureLibrary.ensureDefaultCubemaps();
}

RasterizationRenderer::~RasterizationRenderer()
{
    if (m_skyboxVAO != 0) {
        glDeleteVertexArrays(1, &m_skyboxVAO);
    }
    if (m_skyboxVBO != 0) {
        glDeleteBuffers(1, &m_skyboxVBO);
    }
}

void RasterizationRenderer::initializeSkyboxGeometry()
{
    // clang-format off
    constexpr float skyboxVertices[] = {
        // Face arrière (z = -1)
        -1.0f, -1.0f, -1.0f,   1.0f, -1.0f, -1.0f,   1.0f,  1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,  -1.0f,  1.0f, -1.0f,  -1.0f, -1.0f, -1.0f,
    
        // Face front (z = 1)
        -1.0f, -1.0f,  1.0f,   1.0f,  1.0f,  1.0f,   1.0f, -1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,  -1.0f, -1.0f,  1.0f,  -1.0f,  1.0f,  1.0f,
    
        // Face left (x = -1)
        -1.0f,  1.0f,  1.0f,  -1.0f,  1.0f, -1.0f,  -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,  -1.0f, -1.0f,  1.0f,  -1.0f,  1.0f,  1.0f,
    
        // Face right (x = 1)
         1.0f,  1.0f,  1.0f,   1.0f, -1.0f, -1.0f,   1.0f,  1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,   1.0f,  1.0f,  1.0f,   1.0f, -1.0f,  1.0f,
    
        // Face bottom (y = -1)
        -1.0f, -1.0f, -1.0f,   1.0f, -1.0f, -1.0f,   1.0f, -1.0f,  1.0f,
         1.0f, -1.0f,  1.0f,  -1.0f, -1.0f,  1.0f,  -1.0f, -1.0f, -1.0f,
    
        // Face top (y = 1)
        -1.0f,  1.0f, -1.0f,   1.0f,  1.0f,  1.0f,   1.0f,  1.0f, -1.0f,
         1.0f,  1.0f,  1.0f,  -1.0f,  1.0f, -1.0f,  -1.0f,  1.0f,  1.0f
    };
    

    glGenVertexArrays(1, &m_skyboxVAO);
    glGenBuffers(1, &m_skyboxVBO);
    glBindVertexArray(m_skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), skyboxVertices,
        GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(
        0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
    glBindVertexArray(0);
}

const std::vector<TextureResource> &
RasterizationRenderer::getTextureResources() const
{
    return m_textureLibrary.getTextureResources();
}

const std::vector<int> &RasterizationRenderer::getCubemapHandles() const
{
    return m_textureLibrary.getCubemapHandles();
}

const TextureResource *RasterizationRenderer::getTextureResource(
    int handle) const
{
    return m_textureLibrary.getTextureResource(handle);
}

int RasterizationRenderer::loadTexture2D(
    const std::string &filepath, bool srgb)
{
    return m_textureLibrary.loadTexture2D(filepath, srgb);
}

int RasterizationRenderer::createCheckerboardTexture(const std::string &name,
    int width, int height, const glm::vec3 &colorA, const glm::vec3 &colorB,
    int checks, bool srgb)
{
    return m_textureLibrary.createCheckerboardTexture(
        name, width, height, colorA, colorB, checks, srgb);
}

int RasterizationRenderer::createNoiseTexture(
    const std::string &name, int width, int height, float frequency, bool srgb)
{
    return m_textureLibrary.createNoiseTexture(
        name, width, height, frequency, srgb);
}

int RasterizationRenderer::createSolidColorTexture(const std::string &name,
    const glm::vec3 &color, int width, int height, bool srgb)
{
    return m_textureLibrary.createSolidColorTexture(
        name, color, width, height, srgb);
}

int RasterizationRenderer::createColoredCubemap(const std::string &name,
    const std::array<glm::vec3, 6> &faceColors, int edgeSize, bool srgb)
{
    return m_textureLibrary.createColoredCubemap(
        name, faceColors, edgeSize, srgb);
}

int RasterizationRenderer::loadCubemapFromEquirectangular(
    const std::string &name, const std::string &equirectPath, int resolution,
    bool srgb)
{
    return m_textureLibrary.loadCubemapFromEquirectangular(
        name, equirectPath, resolution, srgb);
}

int RasterizationRenderer::registerObject(std::unique_ptr<RenderableObject> obj)
{
    int id;
    if (!m_freeSlots.empty()) {
        id = m_freeSlots.back();
        m_freeSlots.pop_back();
        m_renderObjects[id] = std::move(obj);
    } else {
        id = m_renderObjects.size();
        m_renderObjects.push_back(std::move(obj));
    }
    return id;
}

int RasterizationRenderer::registerObject(std::unique_ptr<RenderableObject> obj, const std::string &texturePath)
{
    int id;
    const int textureHandle = texturePath.empty()
        ? -1
        : m_textureLibrary.loadTexture2D(texturePath, true);
    obj->assignTexture(textureHandle);
    if (!m_freeSlots.empty()) {
        id = m_freeSlots.back();
        m_freeSlots.pop_back();
        m_renderObjects[id] = std::move(obj);
    } else {
        id = m_renderObjects.size();
        m_renderObjects.push_back(std::move(obj));
    }
    return id;
}

int RasterizationRenderer::registerObject(std::unique_ptr<RenderableObject> obj, const glm::vec3 &color)
{
    int id;

    obj->setColor(color);
    if (!m_freeSlots.empty()) {
        id = m_freeSlots.back();
        m_freeSlots.pop_back();
        m_renderObjects[id] = std::move(obj);
    } else {
        id = m_renderObjects.size();
        m_renderObjects.push_back(std::move(obj));
    }
    return id;
}


void RasterizationRenderer::updateTransform(
    const int objectId, const glm::mat4 &modelMatrix)
{
    if (objectId >= 0 && objectId < static_cast<int>(m_renderObjects.size())) {
        m_renderObjects[objectId]->setModelMatrix(modelMatrix);
    }
}

void RasterizationRenderer::removeObject(const int objectId)
{
    if (objectId < 0 || objectId >= static_cast<int>(m_renderObjects.size()))
        return;

    if (!m_renderObjects[objectId])
        return;

    m_renderObjects[objectId].reset();

    m_freeSlots.push_back(objectId);
}

void RasterizationRenderer::beginFrame()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, 1920, 1080);

    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_vectorialShader.use();
    m_vectorialShader.setMat4("view", m_viewMatrix);
    m_vectorialShader.setMat4("projection", m_projMatrix);

    m_lightingShader.use();
    m_lightingShader.setMat4("view", m_viewMatrix);
    m_lightingShader.setMat4("projection", m_projMatrix);
    m_lightingShader.setVec3("lightColor", DEFAULT_LIGHT_COLOR);
    m_lightingShader.setInt(
        "toneMappingMode", static_cast<int>(m_toneMappingMode));
    m_lightingShader.setFloat("toneExposure", m_toneMappingExposure);

    for (const auto &obj : m_renderObjects) {
        if (obj)
            obj->useShader(m_lightingShader);
    }

    m_pointLightShader.use();
    m_pointLightShader.setMat4("view", m_viewMatrix);
    m_pointLightShader.setMat4("projection", m_projMatrix);

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

void RasterizationRenderer::drawSkybox() const
{
    const int activeHandle = m_textureLibrary.getActiveCubemap();
    if (activeHandle < 0) {
        return;
    }
    const TextureResource *resource = getTextureResource(activeHandle);
    if (!resource || resource->target != TextureTarget::Cubemap) {
        return;
    }

    glDepthFunc(GL_LEQUAL);
    glDepthMask(GL_FALSE);
    m_skyboxShader.use();
    const auto  view = glm::mat4(glm::mat3(m_viewMatrix));
    m_skyboxShader.setMat4("view", view);

    if (m_projectionMode == Camera::ProjectionMode::Orthographic) {
        GLint vp[4] = { 0, 0, 1, 1 };
        glGetIntegerv(GL_VIEWPORT, vp);
        const float aspect
            = (vp[3] != 0) ? static_cast<float>(vp[2]) / static_cast<float>(vp[3])
                           : 1.0f;
        const glm::mat4 skyboxProj
            = glm::perspective(glm::radians(60.0f), aspect, 0.1f, 100.0f);
        m_skyboxShader.setMat4("projection", skyboxProj);
    } else {
        m_skyboxShader.setMat4("projection", m_projMatrix);
    }
    glBindVertexArray(m_skyboxVAO);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, resource->id);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
    glDepthMask(GL_TRUE);
    glDepthFunc(GL_LESS);
}

void RasterizationRenderer::drawAll()
{
    drawSkybox();

    m_vectorialShader.use();
    m_vectorialShader.setMat4("view", m_viewMatrix);
    m_vectorialShader.setMat4("projection", m_projMatrix);

    m_lightingShader.use();
    m_lightingShader.setMat4("view", m_viewMatrix);
    m_lightingShader.setMat4("projection", m_projMatrix);

    m_pointLightShader.use();
    m_pointLightShader.setMat4("view", m_viewMatrix);
    m_pointLightShader.setMat4("projection", m_projMatrix);

    for (const auto &obj : m_renderObjects)
        if (obj && obj->getStatus()) {
            obj->draw(m_vectorialShader, m_pointLightShader, m_lightingShader, m_textureLibrary);

            if (const GLenum error = glGetError(); error != GL_NO_ERROR) {
                std::cerr << "[WARN] OpenGL error after drawing object "
                          << ": " << error << '\n';
            }
        }
}

void RasterizationRenderer::endFrame()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, 1920, 1080);
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glfwSwapBuffers(m_window);
    glfwPollEvents();
}

void RasterizationRenderer::createBoundingBoxBuffers()
{
    glGenVertexArrays(1, &m_bboxVAO);
    glGenBuffers(1, &m_bboxVBO);

    glBindVertexArray(m_bboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_bboxVBO);

    glBufferData(
        GL_ARRAY_BUFFER, 24 * 3 * sizeof(float), nullptr, GL_DYNAMIC_DRAW);

    glVertexAttribPointer(
        0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
}

void RasterizationRenderer::drawBoundingBox(
    const int objectId, const glm::vec3 &corner1, const glm::vec3 &corner2)
{
    if (objectId == -1 || objectId >= static_cast<int>(m_renderObjects.size()))
        [[unlikely]] {
        return;
    }

    const RenderableObject &obj = *m_renderObjects[objectId];

    if (!obj.getStatus()) [[unlikely]] {
        return;
    }
    if (m_bboxVAO == 0) [[unlikely]] {
        createBoundingBoxBuffers();
    }

    const glm::vec3 min = glm::min(corner1, corner2);
    const glm::vec3 max = glm::max(corner1, corner2);

    // clang-format off
    const float bboxVertices[24 * 3] = {
        min.x, min.y, min.z,  max.x, min.y, min.z,
        max.x, min.y, min.z,  max.x, max.y, min.z,
        max.x, max.y, min.z,  min.x, max.y, min.z,
        min.x, max.y, min.z,  min.x, min.y, min.z,

        min.x, min.y, max.z,  max.x, min.y, max.z,
        max.x, min.y, max.z,  max.x, max.y, max.z,
        max.x, max.y, max.z,  min.x, max.y, max.z,
        min.x, max.y, max.z,  min.x, min.y, max.z,

        min.x, min.y, min.z,  min.x, min.y, max.z,
        max.x, min.y, min.z,  max.x, min.y, max.z,
        max.x, max.y, min.z,  max.x, max.y, max.z,
        min.x, max.y, min.z,  min.x, max.y, max.z
    };
    // clang-format on

    glBindBuffer(GL_ARRAY_BUFFER, m_bboxVBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(bboxVertices), bboxVertices);

    m_bboxShader.use();
    m_bboxShader.setMat4("view", m_viewMatrix);
    m_bboxShader.setMat4("projection", m_projMatrix);
    m_bboxShader.setMat4("model", obj.getModelMatrix());
    m_bboxShader.setVec3("bboxColor", glm::vec3(0.0f, 1.0f, 0.0f));

    glBindVertexArray(m_bboxVAO);
    glDrawArrays(GL_LINES, 0, 24);

    glBindVertexArray(0);
}

void RasterizationRenderer::assignTextureToObject(
    const int objectId, const int textureHandle) const
{
    if (objectId < 0 || objectId >= static_cast<int>(m_renderObjects.size())) {
        return;
    }
    if (auto &obj = m_renderObjects[objectId]) {
        const TextureResource *res = getTextureResource(textureHandle);
        obj->assignTexture(res ? textureHandle : -1);
    }
}

void RasterizationRenderer::assignTextureToObject(
    const int objectId, const std::string &texturePath)
{
    if (objectId < 0 || objectId >= static_cast<int>(m_renderObjects.size())) {
        return;
    }
    if (const auto &obj = m_renderObjects[objectId]) {
        const int textureHandle = texturePath.empty()
            ? -1
            : m_textureLibrary.loadTexture2D(texturePath, true);
        obj->assignTexture(textureHandle);
    }
}

int RasterizationRenderer::getObjectTextureHandle(const int objectId) const
{
    if (objectId < 0 || objectId >= static_cast<int>(m_renderObjects.size())) {
        return -1;
    }
    if (auto &obj = m_renderObjects[objectId]) {
        return obj->getTextureHandle();
    }
    return -1;
}

void RasterizationRenderer::setObjectFilter(
    const int objectId, const FilterMode mode) const
{
    if (objectId < 0 || objectId >= static_cast<int>(m_renderObjects.size())) {
        return;
    }
    if (auto &obj = m_renderObjects[objectId]) {
        obj->setFilterMode(mode);
    }
}

FilterMode RasterizationRenderer::getObjectFilter(const int objectId) const
{
    if (objectId < 0 || objectId >= static_cast<int>(m_renderObjects.size())) {
        return FilterMode::None;
    }
    if (auto &obj = m_renderObjects[objectId]) {
        return obj->getFilterMode();
    }
    return FilterMode::None;
}

void RasterizationRenderer::setObjectUseTexture(
    const int objectId, const bool useTexture) const
{
    if (objectId < 0 || objectId >= static_cast<int>(m_renderObjects.size())) {
        return;
    }
    if (auto &obj = m_renderObjects[objectId]) {
        obj->setUseTexture(useTexture);
    }
}

bool RasterizationRenderer::getObjectUseTexture(const int objectId) const
{
    if (objectId < 0 || objectId >= static_cast<int>(m_renderObjects.size())) {
        return false;
    }
    if (auto &obj = m_renderObjects[objectId]) {
        return obj->isUsingTexture();
    }
    return false;
}

void RasterizationRenderer::setToneMappingMode(const ToneMappingMode mode)
{
    m_toneMappingMode = mode;
}

void RasterizationRenderer::setToneMappingExposure(const float exposure)
{
    m_toneMappingExposure = std::clamp(exposure, 0.01f, 20.0f);
}

void RasterizationRenderer::setActiveCubemap(int cubemapHandle)
{
    m_textureLibrary.setActiveCubemap(cubemapHandle);
}
