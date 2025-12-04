#include "pbr/IBLManager.hpp"
#include <glad/gl.h>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

// Static members
const glm::mat4 IBLManager::s_captureProjection
    = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);

const glm::mat4 IBLManager::s_captureViews[6]
    = { glm::lookAt(glm::vec3(0.0f), glm::vec3(1.0f, 0.0f, 0.0f),
            glm::vec3(0.0f, -1.0f, 0.0f)),
          glm::lookAt(glm::vec3(0.0f), glm::vec3(-1.0f, 0.0f, 0.0f),
              glm::vec3(0.0f, -1.0f, 0.0f)),
          glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f),
              glm::vec3(0.0f, 0.0f, 1.0f)),
          glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f, -1.0f, 0.0f),
              glm::vec3(0.0f, 0.0f, -1.0f)),
          glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f, 0.0f, 1.0f),
              glm::vec3(0.0f, -1.0f, 0.0f)),
          glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f, 0.0f, -1.0f),
              glm::vec3(0.0f, -1.0f, 0.0f)) };

IBLManager::IBLManager()
{
    // Initialize shaders
    m_irradianceShader.init("../assets/shaders/ibl_cubemap.vert",
        "../assets/shaders/ibl_irradiance.frag");
    m_prefilterShader.init("../assets/shaders/ibl_cubemap.vert",
        "../assets/shaders/ibl_prefilter.frag");
    m_brdfShader.init(
        "../assets/shaders/ibl_brdf.vert", "../assets/shaders/ibl_brdf.frag");

    // Setup cube VAO
    float vertices[] = { // Back face
        -1.0f, -1.0f, -1.0f, 1.0f, 1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f, 1.0f,
        -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f, -1.0f,
        // Front face
        -1.0f, -1.0f, 1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
        1.0f, -1.0f, 1.0f, 1.0f, -1.0f, -1.0f, 1.0f,
        // Left face
        -1.0f, 1.0f, 1.0f, -1.0f, 1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f, -1.0f, 1.0f, -1.0f, 1.0f, 1.0f,
        // Right face
        1.0f, 1.0f, 1.0f, 1.0f, -1.0f, -1.0f, 1.0f, 1.0f, -1.0f, 1.0f, -1.0f,
        -1.0f, 1.0f, 1.0f, 1.0f, 1.0f, -1.0f, 1.0f,
        // Bottom face
        -1.0f, -1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f, -1.0f, 1.0f, 1.0f,
        -1.0f, 1.0f, -1.0f, -1.0f, 1.0f, -1.0f, -1.0f, -1.0f,
        // Top face
        -1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, -1.0f, 1.0f, 1.0f,
        1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f, 1.0f
    };

    glGenVertexArrays(1, &m_cubeVAO);
    glGenBuffers(1, &m_cubeVBO);
    glBindBuffer(GL_ARRAY_BUFFER, m_cubeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glBindVertexArray(m_cubeVAO);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(
        0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // Setup quad VAO for BRDF LUT
    float quadVertices[] = {
        -1.0f,
        1.0f,
        0.0f,
        0.0f,
        1.0f,
        -1.0f,
        -1.0f,
        0.0f,
        0.0f,
        0.0f,
        1.0f,
        1.0f,
        0.0f,
        1.0f,
        1.0f,
        1.0f,
        -1.0f,
        0.0f,
        1.0f,
        0.0f,
    };

    glGenVertexArrays(1, &m_quadVAO);
    glGenBuffers(1, &m_quadVBO);
    glBindVertexArray(m_quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_quadVBO);
    glBufferData(
        GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(
        0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
        (void *)(3 * sizeof(float)));
    glBindVertexArray(0);
}

IBLManager::~IBLManager()
{
    if (m_cubeVAO) {
        glDeleteVertexArrays(1, &m_cubeVAO);
    }
    if (m_cubeVBO) {
        glDeleteBuffers(1, &m_cubeVBO);
    }
    if (m_quadVAO) {
        glDeleteVertexArrays(1, &m_quadVAO);
    }
    if (m_quadVBO) {
        glDeleteBuffers(1, &m_quadVBO);
    }
    if (m_brdfLUT) {
        glDeleteTextures(1, &m_brdfLUT);
    }
    cleanupCaptureFBO();

    for (auto &[id, textures] : m_iblCache) {
        if (textures.irradianceMap) {
            glDeleteTextures(1, &textures.irradianceMap);
        }
        if (textures.prefilterMap) {
            glDeleteTextures(1, &textures.prefilterMap);
        }
    }
}

void IBLManager::initializeCaptureFBO(int size)
{
    if (m_captureFBO == 0) {
        glGenFramebuffers(1, &m_captureFBO);
        glGenRenderbuffers(1, &m_captureRBO);
    }
    glBindFramebuffer(GL_FRAMEBUFFER, m_captureFBO);
    glBindRenderbuffer(GL_RENDERBUFFER, m_captureRBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, size, size);
    glFramebufferRenderbuffer(
        GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_captureRBO);
}

void IBLManager::cleanupCaptureFBO()
{
    if (m_captureFBO) {
        glDeleteFramebuffers(1, &m_captureFBO);
        m_captureFBO = 0;
    }
    if (m_captureRBO) {
        glDeleteRenderbuffers(1, &m_captureRBO);
        m_captureRBO = 0;
    }
}

void IBLManager::renderCube() const
{
    glBindVertexArray(m_cubeVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
}

unsigned int IBLManager::generateIrradianceMap(
    unsigned int envCubemap, int size)
{
    unsigned int irradianceMap;
    glGenTextures(1, &irradianceMap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMap);
    for (unsigned int i = 0; i < 6; ++i) {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, size,
            size, 0, GL_RGB, GL_FLOAT, nullptr);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    initializeCaptureFBO(size);

    m_irradianceShader.use();
    m_irradianceShader.setInt("environmentMap", 0);
    m_irradianceShader.setMat4("projection", s_captureProjection);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);

    glViewport(0, 0, size, size);
    glBindFramebuffer(GL_FRAMEBUFFER, m_captureFBO);
    for (unsigned int i = 0; i < 6; ++i) {
        m_irradianceShader.setMat4("view", s_captureViews[i]);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
            GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, irradianceMap, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        renderCube();
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    return irradianceMap;
}

unsigned int IBLManager::generatePrefilterMap(
    unsigned int envCubemap, int size)
{
    unsigned int prefilterMap;
    glGenTextures(1, &prefilterMap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, prefilterMap);
    for (unsigned int i = 0; i < 6; ++i) {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, size,
            size, 0, GL_RGB, GL_FLOAT, nullptr);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(
        GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

    m_prefilterShader.use();
    m_prefilterShader.setInt("environmentMap", 0);
    m_prefilterShader.setMat4("projection", s_captureProjection);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);

    glBindFramebuffer(GL_FRAMEBUFFER, m_captureFBO);
    const unsigned int maxMipLevels = 5;
    for (unsigned int mip = 0; mip < maxMipLevels; ++mip) {
        unsigned int mipWidth
            = static_cast<unsigned int>(size * std::pow(0.5, mip));
        unsigned int mipHeight = mipWidth;

        glBindRenderbuffer(GL_RENDERBUFFER, m_captureRBO);
        glRenderbufferStorage(
            GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, mipWidth, mipHeight);
        glViewport(0, 0, mipWidth, mipHeight);

        float roughness
            = static_cast<float>(mip) / static_cast<float>(maxMipLevels - 1);
        m_prefilterShader.setFloat("roughness", roughness);

        for (unsigned int i = 0; i < 6; ++i) {
            m_prefilterShader.setMat4("view", s_captureViews[i]);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, prefilterMap, mip);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            renderCube();
        }
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    return prefilterMap;
}

unsigned int IBLManager::generateBRDFLUT(int size)
{
    unsigned int brdfLUT;
    glGenTextures(1, &brdfLUT);
    glBindTexture(GL_TEXTURE_2D, brdfLUT);
    glTexImage2D(
        GL_TEXTURE_2D, 0, GL_RG16F, size, size, 0, GL_RG, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    initializeCaptureFBO(size);
    glFramebufferTexture2D(
        GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, brdfLUT, 0);

    glViewport(0, 0, size, size);
    m_brdfShader.use();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Render quad
    glBindVertexArray(m_quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    return brdfLUT;
}

unsigned int IBLManager::getBRDFLUT()
{
    if (m_brdfLUT == 0) {
        // Save current viewport
        GLint viewport[4];
        glGetIntegerv(GL_VIEWPORT, viewport);

        m_brdfLUT = generateBRDFLUT(512);

        // Restore viewport
        glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
    }
    return m_brdfLUT;
}

IBLManager::IBLTextures IBLManager::generateIBLFromCubemap(
    unsigned int envCubemap, int /*cubemapSize*/)
{
    // Check cache first
    auto it = m_iblCache.find(envCubemap);
    if (it != m_iblCache.end()) {
        return it->second;
    }

    IBLTextures textures;

    if (envCubemap == 0) {
        std::cerr << "IBLManager: Invalid environment cubemap" << std::endl;
        return textures;
    }

    // Save current viewport
    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);

    // Generate IBL textures
    textures.irradianceMap = generateIrradianceMap(envCubemap, 32);
    textures.prefilterMap = generatePrefilterMap(envCubemap, 128);
    textures.brdfLUT = getBRDFLUT();
    textures.valid = true;

    // Restore viewport
    glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);

    // Cache the result
    m_iblCache[envCubemap] = textures;

    return textures;
}

const IBLManager::IBLTextures *IBLManager::getIBLTextures(
    unsigned int envCubemap) const
{
    auto it = m_iblCache.find(envCubemap);
    if (it != m_iblCache.end()) {
        return &it->second;
    }
    return nullptr;
}

void IBLManager::bindIBLTextures(
    const ShaderProgram &shader, const IBLTextures &textures) const
{
    if (!textures.valid) {
        return;
    }

    shader.use();

    // Bind irradiance map to texture unit 5
    glActiveTexture(GL_TEXTURE5);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textures.irradianceMap);
    shader.setInt("irradianceMap", 5);

    // Bind prefilter map to texture unit 6
    glActiveTexture(GL_TEXTURE6);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textures.prefilterMap);
    shader.setInt("prefilterMap", 6);

    // Bind BRDF LUT to texture unit 7
    glActiveTexture(GL_TEXTURE7);
    glBindTexture(GL_TEXTURE_2D, textures.brdfLUT);
    shader.setInt("brdfLUT", 7);
}
