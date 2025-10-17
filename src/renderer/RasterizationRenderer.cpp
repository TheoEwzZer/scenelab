
#include "renderer/implementation/RasterizationRenderer.hpp"
#include "ShaderProgram.hpp"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

#include "imgui.h"
#include "ImGuizmo.h"

#include "renderer/interface/ARenderer.hpp"
#include <cstddef>
#include <iostream>
#include <vector>
#include <glm/gtc/matrix_transform.hpp>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

RasterizationRenderer::RasterizationRenderer()
{
    std::cout << "Rasterized Renderer Start" << std::endl;
    glEnable(GL_DEPTH_TEST);

    m_lightingShader.init(
        "../assets/shaders/shader.vert", "../assets/shaders/lighting.frag");
    m_lightingShader.use();
    m_lightingShader.setVec3("lightColor", { 1.0f, 1.0f, 1.0f });
    m_lightingShader.setVec3("lightPos", { 2.f, 0.f, 0.0f });

    m_pointLightShader.init(
        "../assets/shaders/shader.vert", "../assets/shaders/pointLight.frag");

    // Initialize view and projection matrices
    m_viewMatrix = glm::mat4(1.0f);
    m_viewMatrix = glm::translate(m_viewMatrix, glm::vec3(0.0f, 0.0f, -4.0f));
    m_projMatrix = glm::perspective(
        glm::radians(45.0f), 1920.0f / 1080.0f, 0.1f, 100.0f);
}

int RasterizationRenderer::registerObject(const std::vector<float> &vertices,
    const std::vector<unsigned int> &indices, const std::string &texturePath,
    bool isLight)
{

    int id;
    if (!m_freeSlots.empty()) {
        // Reuse a free slot
        id = m_freeSlots.back();
        m_freeSlots.pop_back();
    } else {
        // Add new slot
        id = m_renderObjects.size();
        m_renderObjects.emplace_back();
    }

    RenderObject &obj = m_renderObjects[id];

    // Setup OpenGL resources
    glGenVertexArrays(1, &obj.VAO);
    glGenBuffers(1, &obj.VBO);

    // Bind and setup vertex data
    glBindVertexArray(obj.VAO);
    glBindBuffer(GL_ARRAY_BUFFER, obj.VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float),
        vertices.data(), GL_STATIC_DRAW);

    // Only create EBO if indices are provided
    if (!indices.empty()) {
        glGenBuffers(1, &obj.EBO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, obj.EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER,
            indices.size() * sizeof(unsigned int), indices.data(),
            GL_STATIC_DRAW);
        obj.indexCount = indices.size();
        obj.useIndices = true;
    } else {
        obj.EBO = 0;
        obj.indexCount = vertices.size() / 8;
        obj.useIndices = false;
    }

    // position attribute (3 floats)
    glVertexAttribPointer(
        0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);
    // texture coord attribute (2 floats)
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float),
        (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    // normal vector attribute
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float),
        (void *)(5 * sizeof(float)));
    glEnableVertexAttribArray(2);

    obj.texture = loadTexture(texturePath);
    obj.isActive = true;
    obj.isLight = isLight;

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

        // Cleanup OpenGL resources
        glDeleteVertexArrays(1, &obj.VAO);
        glDeleteBuffers(1, &obj.VBO);
        glDeleteBuffers(1, &obj.EBO);
        glDeleteTextures(1, &obj.texture);

        obj.isActive = false;
        m_freeSlots.push_back(objectId);
    }
}

void RasterizationRenderer::beginFrame()
{
    glClearColor(0.4f, 0.2f, 0.2f, 1.0f);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Set view and projection matrices once per frame
    m_lightingShader.use();
    m_lightingShader.setMat4("view", m_viewMatrix);
    m_lightingShader.setMat4("projection", m_projMatrix);
    for (size_t i = 0; i < m_renderObjects.size(); ++i) {
        const auto &obj = m_renderObjects[i];
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
    ImGuizmo::BeginFrame();
    ImGuizmo::SetRect(0, 0, 1920, 1080);
}

void RasterizationRenderer::drawAll()
{

    for (size_t i = 0; i < m_renderObjects.size(); ++i) {
        const auto &obj = m_renderObjects[i];
        if (obj.isActive) {

            // Set model matrix for this object

            if (obj.isLight) {
                m_pointLightShader.use();
                m_pointLightShader.setMat4("model", obj.modelMatrix);
            } else {
                m_lightingShader.use();
                m_lightingShader.setMat4("model", obj.modelMatrix);
            }

            glBindTexture(GL_TEXTURE_2D, obj.texture);
            glBindVertexArray(obj.VAO);
            if (!obj.useIndices) {
                glDrawArrays(GL_TRIANGLES, 0, obj.indexCount);
            } else {
                glDrawElements(
                    GL_TRIANGLES, obj.indexCount, GL_UNSIGNED_INT, 0);
            }

            // Check for OpenGL errors
            GLenum error = glGetError();
            if (error != GL_NO_ERROR && i == 0) {
                std::cout << "OpenGL error after drawing object " << i << ": "
                          << error << std::endl;
            }
        }
    }
}

void RasterizationRenderer::endFrame()
{
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glfwSwapBuffers(m_window);
    glfwPollEvents();
}

RasterizationRenderer::~RasterizationRenderer()
{
    std::cout << "Rasterized Renderer Stop" << std::endl;
}

unsigned int RasterizationRenderer::loadTexture(std::string filepath)
{
    unsigned int texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    // set the texture wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // set texture filtering parameters
    glTexParameteri(
        GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // load image, create texture and generate mipmaps
    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(true);
    unsigned char *data
        = stbi_load(filepath.c_str(), &width, &height, &nrChannels, 0);

    if (data) {
        GLenum format;
        if (nrChannels == 1) {
            format = GL_RED;
        } else if (nrChannels == 3) {
            format = GL_RGB;
        } else if (nrChannels == 4) {
            format = GL_RGBA;
        } else {
            std::cout << "Unsupported number of channels: " << nrChannels
                      << std::endl;
            format = GL_RGB;
        }

        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format,
            GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    } else {
        std::cout << "Failed to load texture: " << filepath << std::endl;
    }
    stbi_image_free(data);

    return texture;
}
