#include "renderer/implementation/RasterizationRenderer.hpp"
#include "Camera.hpp"
#include "ShaderProgram.hpp"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

#include "ImGuizmo.h"
#include "glm/fwd.hpp"
#include "imgui.h"
#include "objects/Material.hpp"
#include "renderer/interface/IRenderer.hpp"

#include <algorithm>
#include <cstddef>
#include <cmath>
#include <iostream>
#include <limits>
#include <memory>
#include <stdexcept>
#include <vector>

#include <glm/gtc/matrix_transform.hpp>
#include "objects/Light.hpp"


RasterizationRenderer::RasterizationRenderer(Window &window)
    : m_window(window)
    , m_ambientLightColor(DEFAULT_AMBIENT_LIGHT_COLOR)
{

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

    m_lightingShader.init(
        "../assets/shaders/shader.vert", "../assets/shaders/lighting.frag");
    m_gouraudLightingShader.init(
        "../assets/shaders/lighting_gouraud.vert", "../assets/shaders/lighting_gouraud.frag");
    m_vectorialShader.init(
        "../assets/shaders/shader_vect.vert", "../assets/shaders/vect.frag");
    m_bboxShader.init(
        "../assets/shaders/shader.vert", "../assets/shaders/bbox.frag");
    m_skyboxShader.init(
        "../assets/shaders/skybox.vert", "../assets/shaders/skybox.frag");

    for (auto &shader: {m_vectorialShader, m_lightingShader, m_gouraudLightingShader}) {
        shader.use();
        shader.setVec3("ambientLightColor", m_ambientLightColor);
        shader.setInt("ourTexture", 0);
        shader.setInt("filterMode", 0);
        shader.setVec2("texelSize", glm::vec2(1.0f));
        shader.setInt(
            "toneMappingMode", static_cast<int>(m_toneMappingMode));
        shader.setFloat("toneExposure", m_toneMappingExposure);
    }
    m_skyboxShader.use();
    m_skyboxShader.setInt("skybox", 0);
    m_lightingShader.use();

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

int RasterizationRenderer::registerObject(std::unique_ptr<RenderableObject> obj, const Material &material)
{
    int id;

    obj->setMaterial(material);
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

std::vector<std::unique_ptr<RenderableObject>> RasterizationRenderer::extractAllObjects()
{
    std::vector<std::unique_ptr<RenderableObject>> objects;
    objects.reserve(m_renderObjects.size());

    for (auto &obj : m_renderObjects) {
        objects.push_back(std::move(obj));
    }

    m_renderObjects.clear();
    m_freeSlots.clear();

    return objects;
}

void RasterizationRenderer::setObjectColor(int objectId, const glm::vec3 &color)
{
    if (objectId < 0 || objectId >= static_cast<int>(m_renderObjects.size()))
        return;
    if (!m_renderObjects[objectId])
        return;

    m_renderObjects[objectId]->setColor(color);
}

glm::vec3 RasterizationRenderer::getObjectColor(int objectId) const
{
    if (objectId < 0 || objectId >= static_cast<int>(m_renderObjects.size()))
        return glm::vec3(1.0f);
    if (!m_renderObjects[objectId])
        return glm::vec3(1.0f);

    return m_renderObjects[objectId]->getColor();
}

void RasterizationRenderer::setObjectEmissive(int objectId, const glm::vec3 &emissive)
{
    if (objectId < 0 || objectId >= static_cast<int>(m_renderObjects.size()))
        return;
    if (!m_renderObjects[objectId])
        return;

    m_renderObjects[objectId]->setEmissive(emissive);
}

glm::vec3 RasterizationRenderer::getObjectEmissive(int objectId) const
{
    if (objectId < 0 || objectId >= static_cast<int>(m_renderObjects.size()))
        return glm::vec3(0.0f);
    if (!m_renderObjects[objectId])
        return glm::vec3(0.0f);

    return m_renderObjects[objectId]->getEmissive();
}

void RasterizationRenderer::setObjectPercentSpecular(int objectId, float percent)
{
    if (objectId < 0 || objectId >= static_cast<int>(m_renderObjects.size()))
        return;
    if (!m_renderObjects[objectId])
        return;

    m_renderObjects[objectId]->setPercentSpecular(percent);
}

float RasterizationRenderer::getObjectPercentSpecular(int objectId) const
{
    if (objectId < 0 || objectId >= static_cast<int>(m_renderObjects.size()))
        return 0.0f;
    if (!m_renderObjects[objectId])
        return 0.0f;

    return m_renderObjects[objectId]->getPercentSpecular();
}

void RasterizationRenderer::setObjectRoughness(int objectId, float roughness)
{
    if (objectId < 0 || objectId >= static_cast<int>(m_renderObjects.size()))
        return;
    if (!m_renderObjects[objectId])
        return;

    m_renderObjects[objectId]->setRoughness(roughness);
}

float RasterizationRenderer::getObjectRoughness(int objectId) const
{
    if (objectId < 0 || objectId >= static_cast<int>(m_renderObjects.size()))
        return 0.5f;
    if (!m_renderObjects[objectId])
        return 0.5f;

    return m_renderObjects[objectId]->getRoughness();
}

void RasterizationRenderer::setObjectSpecularColor(int objectId, const glm::vec3 &color)
{
    if (objectId < 0 || objectId >= static_cast<int>(m_renderObjects.size()))
        return;
    if (!m_renderObjects[objectId])
        return;

    m_renderObjects[objectId]->setSpecularColor(color);
}

glm::vec3 RasterizationRenderer::getObjectSpecularColor(int objectId) const
{
    if (objectId < 0 || objectId >= static_cast<int>(m_renderObjects.size()))
        return glm::vec3(1.0f);
    if (!m_renderObjects[objectId])
        return glm::vec3(1.0f);

    return m_renderObjects[objectId]->getSpecularColor();
}

void RasterizationRenderer::setObjectIndexOfRefraction(int objectId, float ior)
{
    if (objectId < 0 || objectId >= static_cast<int>(m_renderObjects.size()))
        return;
    if (!m_renderObjects[objectId])
        return;

    m_renderObjects[objectId]->setIndexOfRefraction(ior);
}

float RasterizationRenderer::getObjectIndexOfRefraction(int objectId) const
{
    if (objectId < 0 || objectId >= static_cast<int>(m_renderObjects.size()))
        return 1.0f;
    if (!m_renderObjects[objectId])
        return 1.0f;

    return m_renderObjects[objectId]->getIndexOfRefraction();
}

void RasterizationRenderer::setObjectRefractionChance(int objectId, float chance)
{
    if (objectId < 0 || objectId >= static_cast<int>(m_renderObjects.size()))
        return;
    if (!m_renderObjects[objectId])
        return;

    m_renderObjects[objectId]->setRefractionChance(chance);
}

float RasterizationRenderer::getObjectRefractionChance(int objectId) const
{
    if (objectId < 0 || objectId >= static_cast<int>(m_renderObjects.size()))
        return 0.0f;
    if (!m_renderObjects[objectId])
        return 0.0f;

    return m_renderObjects[objectId]->getRefractionChance();
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

    auto &lshader = this->m_lightingModel != GOURAUD ? m_lightingShader : m_gouraudLightingShader;

    lshader.use();
    lshader.setMat4("view", m_viewMatrix);
    lshader.setMat4("projection", m_projMatrix);
    lshader.setInt("lightingModel", m_lightingModel);
    lshader.setInt(
        "toneMappingMode", static_cast<int>(m_toneMappingMode));
    lshader.setFloat("toneExposure", m_toneMappingExposure);

    std::vector<int> lightsNumbers(Light::Type::TypeEnd);
    for (const auto &obj : m_renderObjects) {
        if (obj) {
            const Light *l = dynamic_cast<Light*>(obj.get());
            if (l != nullptr) {
                l->setUniforms(lightsNumbers[l->getType()], lshader);
                lightsNumbers[l->getType()] += 1;
            }

            obj->useShader(lshader);
        }
    }

    lshader.setInt("NB_DIR_LIGHTS", lightsNumbers[Light::Type::Directional]);
    lshader.setInt("NB_POINT_LIGHTS", lightsNumbers[Light::Type::Point]);
    lshader.setInt("NB_SPOT_LIGHTS", lightsNumbers[Light::Type::Spot]);

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

void RasterizationRenderer::drawAll(Camera cam)
{
    drawSkybox();

    m_vectorialShader.use();
    m_vectorialShader.setMat4("view", m_viewMatrix);
    m_vectorialShader.setMat4("projection", m_projMatrix);

    auto &lshader = this->m_lightingModel != GOURAUD ? m_lightingShader : m_gouraudLightingShader;

    lshader.use();
    lshader.setMat4("view", m_viewMatrix);
    lshader.setMat4("projection", m_projMatrix);
    lshader.setInt("lightingModel", m_lightingModel);
    lshader.setInt("toneMappingMode", static_cast<int>(m_toneMappingMode));
    lshader.setFloat("toneExposure", m_toneMappingExposure);
    lshader.setVec3("viewPosition", cam.getPosition());
    lshader.setVec3("ambientLightColor", m_ambientLightColor);

    // Configure light uniforms
    std::vector<int> lightsNumbers(Light::Type::TypeEnd, 0);
    for (const auto &obj : m_renderObjects) {
        if (obj) {
            const Light *l = dynamic_cast<Light*>(obj.get());
            if (l != nullptr) {
                l->setUniforms(lightsNumbers[l->getType()], lshader);
                lightsNumbers[l->getType()] += 1;
            }
        }
    }
    lshader.setInt("NB_DIR_LIGHTS", lightsNumbers[Light::Type::Directional]);
    lshader.setInt("NB_POINT_LIGHTS", lightsNumbers[Light::Type::Point]);
    lshader.setInt("NB_SPOT_LIGHTS", lightsNumbers[Light::Type::Spot]);

    for (const auto &obj : m_renderObjects)
        if (obj && obj->getStatus()) {
            obj->draw(m_vectorialShader, lshader, lshader, m_textureLibrary);

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

    m_window.swapBuffers();
    m_window.pollEvents();
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

RenderableObject &RasterizationRenderer::getRenderable(int objectId) const
{
    if (objectId < 0 || objectId >= static_cast<int>(m_renderObjects.size())) {
        throw std::invalid_argument("getRendrable: invalid ObjectID");
    }
    if (auto &obj = m_renderObjects[objectId]) {
        return (*obj);
    }
    throw std::invalid_argument("getRendrable: invalid ObjectID");
}

void RasterizationRenderer::assignMaterialToObject(
    const int objectId, Material &mat) const
{
    if (objectId < 0 || objectId >= static_cast<int>(m_renderObjects.size())) {
        return;
    }
    if (auto &obj = m_renderObjects[objectId]) {
        obj->setMaterial(mat);
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

// Window-related methods
bool RasterizationRenderer::shouldWindowClose()
{
    return m_window.shouldClose();
}

void RasterizationRenderer::addKeyCallback(
    int key, int action, std::function<void()> callback)
{
    m_window.addKeyCallback(key, action, callback);
}

void RasterizationRenderer::addCursorCallback(
    std::function<void(double, double)> callback)
{
    m_window.addCursorCallback(callback);
}

void RasterizationRenderer::addDropCallback(std::function<void(
        const std::vector<std::string> &paths, double mouseX, double mouseY)>
        callback)
{
    m_window.addDropCallback(callback);
}

GLFWwindow *RasterizationRenderer::getWindow() const
{
    return m_window.getGLFWWindow();
}

// Camera view management methods
void RasterizationRenderer::setCameraOverlayCallback(
    CameraOverlayCallback callback)
{
    m_cameraOverlayCallback = std::move(callback);
}

void RasterizationRenderer::setBoundingBoxDrawCallback(
    BoundingBoxDrawCallback callback)
{
    m_bboxDrawCallback = std::move(callback);
}

void RasterizationRenderer::renderAllViews(CameraManager &cameraManager)
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

void RasterizationRenderer::createCameraViews(
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

        m_cameraViews[id] = std::move(view);
    }
}

void RasterizationRenderer::destroyCameraViews(const int id)
{
    if (const auto it = m_cameraViews.find(id); it != m_cameraViews.end()) {
        const auto &view = it->second;
        glDeleteFramebuffers(1, &view.fbo);
        glDeleteTextures(1, &view.colorTex);
        glDeleteRenderbuffers(1, &view.depthRBO);
        m_cameraViews.erase(it);
    }
}

void RasterizationRenderer::renderCameraViews(
    const Camera &cam, const CameraView &view)
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
    setProjectionMode(cam.getProjectionMode());
    setProjectionMatrix(cam.getProjectionMatrix());

    // Draw scene
    drawAll(cam);

    // Draw bounding boxes if callback is set
    if (m_bboxDrawCallback) {
        m_bboxDrawCallback();
    }

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

void RasterizationRenderer::renderDockableViews(CameraManager &cameraManager)
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
