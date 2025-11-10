
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

namespace {
constexpr glm::vec3 DEFAULT_LIGHT_COLOR { 1.0f, 1.0f, 1.0f };
constexpr glm::vec3 DEFAULT_LIGHT_POS { 2.0f, 0.0f, 0.0f };
}

RasterizationRenderer::RasterizationRenderer()
{
    std::cout << "Rasterized Renderer Start" << std::endl;

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
    for (auto &obj : m_renderObjects) {
        if (obj.VAO != 0) {
            glDeleteVertexArrays(1, &obj.VAO);
        }
        if (obj.VBO != 0) {
            glDeleteBuffers(1, &obj.VBO);
        }
        if (obj.EBO != 0) {
            glDeleteBuffers(1, &obj.EBO);
        }
        if (obj.bboxVAO != 0) {
            glDeleteVertexArrays(1, &obj.bboxVAO);
        }
        if (obj.bboxVBO != 0) {
            glDeleteBuffers(1, &obj.bboxVBO);
        }
    }

    if (m_skyboxVAO != 0) {
        glDeleteVertexArrays(1, &m_skyboxVAO);
    }
    if (m_skyboxVBO != 0) {
        glDeleteBuffers(1, &m_skyboxVBO);
    }

    std::cout << "Rasterized Renderer Stop" << std::endl;
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

int RasterizationRenderer::registerObject(const std::vector<float> &vertices,
    const std::vector<unsigned int> &indices, const std::string &texturePath,
    bool isLight, bool is2D)
{
    const int textureHandle = texturePath.empty()
        ? -1
        : m_textureLibrary.loadTexture2D(texturePath, true);
    return registerObjectWithTextureHandle(
        vertices, indices, textureHandle, isLight, is2D);
}

int RasterizationRenderer::registerObject(const std::vector<float> &vertices,
    const std::vector<unsigned int> &indices, bool isLight)
{
    return registerObjectWithTextureHandle(vertices, indices, -1, isLight);
}

int RasterizationRenderer::registerObject(const std::vector<float> &vertices,
    const std::vector<unsigned int> &indices, const glm::vec3 &color,
    bool isLight)
{
    const int id
        = registerObjectWithTextureHandle(vertices, indices, -1, isLight);
    if (id >= 0) {
        m_renderObjects[id].useTexture = false;
        m_renderObjects[id].objectColor = color;
    }
    return id;
}

int RasterizationRenderer::registerObjectWithTextureHandle(
    const std::vector<float> &vertices,
    const std::vector<unsigned int> &indices, int textureHandle, bool isLight,
    bool is2D)
{
    int id = -1;
    if (!m_freeSlots.empty()) {
        id = m_freeSlots.back();
        m_freeSlots.pop_back();
    } else {
        id = static_cast<int>(m_renderObjects.size());
        m_renderObjects.emplace_back();
    }

    RenderObject &obj = m_renderObjects[id];

    glGenVertexArrays(1, &obj.VAO);
    glGenBuffers(1, &obj.VBO);

    glBindVertexArray(obj.VAO);
    glBindBuffer(GL_ARRAY_BUFFER, obj.VBO);

    std::vector<float> processedVertices;
    const bool hasIndices = !indices.empty();

    if (is2D) {
        constexpr int originalStride = 8;
        if (vertices.size() % originalStride != 0) {
            std::cerr << "[ERROR] Invalid 2D vertex buffer size." << std::endl;
        }
        const int vertexCount
            = static_cast<int>(vertices.size() / originalStride);
        float minX = std::numeric_limits<float>::max();
        float maxX = std::numeric_limits<float>::lowest();
        float minY = std::numeric_limits<float>::max();
        float maxY = std::numeric_limits<float>::lowest();

        for (int i = 0; i < vertexCount; ++i) {
            const float x = vertices[i * originalStride + 0];
            const float y = vertices[i * originalStride + 1];
            minX = std::min(minX, x);
            maxX = std::max(maxX, x);
            minY = std::min(minY, y);
            maxY = std::max(maxY, y);
        }

        const float invWidth
            = (maxX - minX) > std::numeric_limits<float>::epsilon()
            ? 1.0f / (maxX - minX)
            : 0.0f;
        const float invHeight
            = (maxY - minY) > std::numeric_limits<float>::epsilon()
            ? 1.0f / (maxY - minY)
            : 0.0f;

        processedVertices.reserve(static_cast<size_t>(vertexCount) * 9);
        for (int i = 0; i < vertexCount; ++i) {
            const float x = vertices[i * originalStride + 0];
            const float y = vertices[i * originalStride + 1];
            const float z = vertices[i * originalStride + 2];
            const float r = vertices[i * originalStride + 3];
            const float g = vertices[i * originalStride + 4];
            const float b = vertices[i * originalStride + 5];
            const float a = vertices[i * originalStride + 6];

            const float u = invWidth == 0.0f ? 0.5f : (x - minX) * invWidth;
            const float v = invHeight == 0.0f ? 0.5f : (y - minY) * invHeight;

            processedVertices.insert(
                processedVertices.end(), { x, y, z, r, g, b, a, u, v });
        }

        glBufferData(GL_ARRAY_BUFFER, processedVertices.size() * sizeof(float),
            processedVertices.data(), GL_STATIC_DRAW);

        if (!processedVertices.empty()) {
            obj.objectColor = glm::vec3(processedVertices[3],
                processedVertices[4], processedVertices[5]);
        }
    } else {
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float),
            vertices.data(), GL_STATIC_DRAW);
    }

    if (hasIndices) {
        glGenBuffers(1, &obj.EBO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, obj.EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER,
            indices.size() * sizeof(unsigned int), indices.data(),
            GL_STATIC_DRAW);
        obj.indexCount = static_cast<unsigned int>(indices.size());
        obj.useIndices = true;
    } else {
        obj.EBO = 0;
        if (is2D) {
            constexpr int newStride = 9;
            obj.indexCount = static_cast<unsigned int>(
                processedVertices.size() / newStride);
        } else {
            obj.indexCount = static_cast<unsigned int>(vertices.size() / 8);
        }
        obj.useIndices = false;
    }

    if (is2D) {
        const GLsizei stride = static_cast<GLsizei>(9 * sizeof(float));
        glVertexAttribPointer(
            0, 3, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<void *>(0));
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, stride,
            reinterpret_cast<void *>(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride,
            reinterpret_cast<void *>((3 + 4) * sizeof(float)));
        glEnableVertexAttribArray(2);
    } else {
        glVertexAttribPointer(
            0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float),
            (void *)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float),
            (void *)(5 * sizeof(float)));
        glEnableVertexAttribArray(2);
    }
    glBindVertexArray(0);

    obj.isActive = true;
    obj.isLight = isLight;
    obj.is2D = is2D;
    obj.textureHandle = textureHandle;
    obj.useTexture = textureHandle >= 0;
    obj.filterMode = FilterMode::None;
    obj.filterStrength = 1.0f;
    obj.modelMatrix = glm::mat4(1.0f);
    if (!is2D) {
        obj.objectColor = glm::vec3(1.0f);
    }

    if (const auto *tex = getTextureResource(textureHandle)) {
        obj.textureIsProcedural = tex->procedural;
    } else {
        obj.textureIsProcedural = false;
    }

    return id;
}

void RasterizationRenderer::updateTransform(
    int objectId, const glm::mat4 &modelMatrix)
{
    if (objectId >= 0 && objectId < static_cast<int>(m_renderObjects.size())) {
        m_renderObjects[objectId].modelMatrix = modelMatrix;
    }
}

void RasterizationRenderer::removeObject(int objectId)
{
    if (objectId >= 0 && objectId < static_cast<int>(m_renderObjects.size())) {
        RenderObject &obj = m_renderObjects[objectId];

        if (obj.VAO != 0) {
            glDeleteVertexArrays(1, &obj.VAO);
            obj.VAO = 0;
        }
        if (obj.VBO != 0) {
            glDeleteBuffers(1, &obj.VBO);
            obj.VBO = 0;
        }
        if (obj.EBO != 0) {
            glDeleteBuffers(1, &obj.EBO);
            obj.EBO = 0;
        }
        if (obj.bboxVAO != 0) {
            glDeleteVertexArrays(1, &obj.bboxVAO);
            obj.bboxVAO = 0;
        }
        if (obj.bboxVBO != 0) {
            glDeleteBuffers(1, &obj.bboxVBO);
            obj.bboxVBO = 0;
        }

        obj.isActive = false;
        obj.textureHandle = -1;
        obj.useTexture = false;

        m_freeSlots.push_back(objectId);
    }
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
        if (obj.isLight) {
            m_lightingShader.setVec3(
                "lightPos", glm::vec3(obj.modelMatrix[3]));
        }
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

void RasterizationRenderer::drawSkybox()
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
    glm::mat4 view = glm::mat4(glm::mat3(m_viewMatrix));
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

    for (size_t i = 0; i < m_renderObjects.size(); ++i) {
        const auto &obj = m_renderObjects[i];
        if (!obj.isActive) {
            continue;
        }

        const TextureResource *texture = getTextureResource(obj.textureHandle);

        if (obj.isLight) {
            m_pointLightShader.use();
            m_pointLightShader.setMat4("model", obj.modelMatrix);
        } else if (obj.is2D) {
            m_vectorialShader.use();
            m_vectorialShader.setMat4("model", obj.modelMatrix);
            const bool useTexture = obj.useTexture && texture
                && texture->target == TextureTarget::Texture2D;
            m_vectorialShader.setBool("useTexture", useTexture);
            m_vectorialShader.setInt(
                "filterMode", static_cast<int>(obj.filterMode));
            glm::vec2 texelSize = useTexture
                ? glm::vec2(1.0f / static_cast<float>(texture->size.x),
                    1.0f / static_cast<float>(texture->size.y))
                : glm::vec2(0.0f);
            m_vectorialShader.setVec2("texelSize", texelSize);
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        } else {
            m_lightingShader.use();
            m_lightingShader.setMat4("model", obj.modelMatrix);
            const bool useTexture = obj.useTexture && texture
                && texture->target == TextureTarget::Texture2D;
            m_lightingShader.setBool("useTexture", useTexture);
            m_lightingShader.setVec3("objectColor", obj.objectColor);
            m_lightingShader.setInt(
                "filterMode", static_cast<int>(obj.filterMode));
            glm::vec2 texelSize = useTexture
                ? glm::vec2(1.0f / static_cast<float>(texture->size.x),
                    1.0f / static_cast<float>(texture->size.y))
                : glm::vec2(0.0f);
            m_lightingShader.setVec2("texelSize", texelSize);
        }

        if (texture) {
            if (texture->target == TextureTarget::Texture2D) {
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, texture->id);
            }
        } else {
            glBindTexture(GL_TEXTURE_2D, 0);
        }

        glBindVertexArray(obj.VAO);
        if (!obj.useIndices) {
            glDrawArrays(GL_TRIANGLES, 0, obj.indexCount);
        } else {
            glDrawElements(GL_TRIANGLES, obj.indexCount, GL_UNSIGNED_INT, 0);
        }
        glBindVertexArray(0);

        if (obj.is2D) {
            glDisable(GL_BLEND);
        }

        if (GLenum error = glGetError(); error != GL_NO_ERROR) {
            std::cerr << "[WARN] OpenGL error after drawing object " << i
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

void RasterizationRenderer::createBoundingBoxBuffers(RenderObject &obj)
{
    glGenVertexArrays(1, &obj.bboxVAO);
    glGenBuffers(1, &obj.bboxVBO);

    glBindVertexArray(obj.bboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, obj.bboxVBO);

    glBufferData(
        GL_ARRAY_BUFFER, 24 * 3 * sizeof(float), nullptr, GL_DYNAMIC_DRAW);

    glVertexAttribPointer(
        0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
}

void RasterizationRenderer::drawBoundingBox(
    int objectId, const glm::vec3 &corner1, const glm::vec3 &corner2)
{
    if (objectId == -1 || objectId >= static_cast<int>(m_renderObjects.size()))
        [[unlikely]] {
        return;
    }

    RenderObject &obj = m_renderObjects[objectId];

    if (!obj.isActive) [[unlikely]] {
        return;
    }
    if (obj.bboxVAO == 0) [[unlikely]] {
        createBoundingBoxBuffers(obj);
    }

    glm::vec3 min = glm::min(corner1, corner2);
    glm::vec3 max = glm::max(corner1, corner2);

    // clang-format off
    float bboxVertices[24 * 3] = {
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

    glBindBuffer(GL_ARRAY_BUFFER, obj.bboxVBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(bboxVertices), bboxVertices);

    m_bboxShader.use();
    m_bboxShader.setMat4("view", m_viewMatrix);
    m_bboxShader.setMat4("projection", m_projMatrix);
    m_bboxShader.setMat4("model", obj.modelMatrix);
    m_bboxShader.setVec3("bboxColor", glm::vec3(0.0f, 1.0f, 0.0f));

    glBindVertexArray(obj.bboxVAO);
    glDrawArrays(GL_LINES, 0, 24);

    glBindVertexArray(0);
}

void RasterizationRenderer::assignTextureToObject(
    int objectId, int textureHandle)
{
    if (objectId < 0 || objectId >= static_cast<int>(m_renderObjects.size())) {
        return;
    }
    RenderObject &obj = m_renderObjects[objectId];
    const TextureResource *res = getTextureResource(textureHandle);
    obj.textureHandle = res ? textureHandle : -1;
    obj.useTexture = res != nullptr;
    obj.textureIsProcedural = res ? res->procedural : false;
}

int RasterizationRenderer::getObjectTextureHandle(int objectId) const
{
    if (objectId < 0 || objectId >= static_cast<int>(m_renderObjects.size())) {
        return -1;
    }
    return m_renderObjects[objectId].textureHandle;
}

void RasterizationRenderer::setObjectFilter(int objectId, FilterMode mode)
{
    if (objectId < 0 || objectId >= static_cast<int>(m_renderObjects.size())) {
        return;
    }
    m_renderObjects[objectId].filterMode = mode;
}

FilterMode RasterizationRenderer::getObjectFilter(int objectId) const
{
    if (objectId < 0 || objectId >= static_cast<int>(m_renderObjects.size())) {
        return FilterMode::None;
    }
    return m_renderObjects[objectId].filterMode;
}

void RasterizationRenderer::setObjectUseTexture(int objectId, bool useTexture)
{
    if (objectId < 0 || objectId >= static_cast<int>(m_renderObjects.size())) {
        return;
    }
    RenderObject &obj = m_renderObjects[objectId];
    obj.useTexture = useTexture && obj.textureHandle >= 0;
}

bool RasterizationRenderer::getObjectUseTexture(int objectId) const
{
    if (objectId < 0 || objectId >= static_cast<int>(m_renderObjects.size())) {
        return false;
    }
    return m_renderObjects[objectId].useTexture;
}

void RasterizationRenderer::setToneMappingMode(ToneMappingMode mode)
{
    m_toneMappingMode = mode;
}

void RasterizationRenderer::setToneMappingExposure(float exposure)
{
    m_toneMappingExposure = std::clamp(exposure, 0.01f, 20.0f);
}

void RasterizationRenderer::setActiveCubemap(int cubemapHandle)
{
    m_textureLibrary.setActiveCubemap(cubemapHandle);
}
