#include "deferred/DeferredRenderer.hpp"
#include <iostream>

DeferredRenderer::DeferredRenderer() {}

DeferredRenderer::~DeferredRenderer() { cleanup(); }

void DeferredRenderer::cleanup()
{
    if (m_gBufferFBO) {
        glDeleteFramebuffers(1, &m_gBufferFBO);
        m_gBufferFBO = 0;
    }
    if (m_gPosition) {
        glDeleteTextures(1, &m_gPosition);
        m_gPosition = 0;
    }
    if (m_gNormal) {
        glDeleteTextures(1, &m_gNormal);
        m_gNormal = 0;
    }
    if (m_gAlbedoSpec) {
        glDeleteTextures(1, &m_gAlbedoSpec);
        m_gAlbedoSpec = 0;
    }
    if (m_gMetallicRoughness) {
        glDeleteTextures(1, &m_gMetallicRoughness);
        m_gMetallicRoughness = 0;
    }
    if (m_rboDepth) {
        glDeleteRenderbuffers(1, &m_rboDepth);
        m_rboDepth = 0;
    }
    if (m_quadVAO) {
        glDeleteVertexArrays(1, &m_quadVAO);
        m_quadVAO = 0;
    }
    if (m_quadVBO) {
        glDeleteBuffers(1, &m_quadVBO);
        m_quadVBO = 0;
    }
    m_initialized = false;
}

bool DeferredRenderer::initialize(int width, int height)
{
    if (width <= 0 || height <= 0) {
        std::cerr << "DeferredRenderer: Invalid dimensions" << std::endl;
        return false;
    }

    cleanup();

    m_width = width;
    m_height = height;

    // Create G-Buffer framebuffer
    glGenFramebuffers(1, &m_gBufferFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, m_gBufferFBO);

    // Position color buffer (RGB16F for world space positions)
    glGenTextures(1, &m_gPosition);
    glBindTexture(GL_TEXTURE_2D, m_gPosition);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB,
        GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(
        GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_gPosition, 0);

    // Normal color buffer (RGB16F for world space normals)
    glGenTextures(1, &m_gNormal);
    glBindTexture(GL_TEXTURE_2D, m_gNormal);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB,
        GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(
        GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, m_gNormal, 0);

    // Albedo + AO color buffer (RGBA8)
    glGenTextures(1, &m_gAlbedoSpec);
    glBindTexture(GL_TEXTURE_2D, m_gAlbedoSpec);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA,
        GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(
        GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, m_gAlbedoSpec, 0);

    // Metallic + Roughness buffer (RGBA16F for precision)
    glGenTextures(1, &m_gMetallicRoughness);
    glBindTexture(GL_TEXTURE_2D, m_gMetallicRoughness);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA,
        GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D,
        m_gMetallicRoughness, 0);

    // Tell OpenGL which color attachments we'll use
    unsigned int attachments[4] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1,
        GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3 };
    glDrawBuffers(4, attachments);

    // Create depth renderbuffer
    glGenRenderbuffers(1, &m_rboDepth);
    glBindRenderbuffer(GL_RENDERBUFFER, m_rboDepth);
    glRenderbufferStorage(
        GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, width, height);
    glFramebufferRenderbuffer(
        GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_rboDepth);

    // Check framebuffer completeness
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "DeferredRenderer: G-Buffer framebuffer not complete!"
                  << std::endl;
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        cleanup();
        return false;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Setup screen quad for lighting pass
    setupQuad();

    m_initialized = true;
    std::cout << "DeferredRenderer: G-Buffer initialized (" << width << "x"
              << height << ")" << std::endl;

    return true;
}

void DeferredRenderer::resize(int width, int height)
{
    if (width == m_width && height == m_height) {
        return;
    }
    initialize(width, height);
}

void DeferredRenderer::setupQuad()
{
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
        GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(
        0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
        (void *)(3 * sizeof(float)));
    glBindVertexArray(0);
}

void DeferredRenderer::beginGeometryPass()
{
    if (!m_initialized) {
        return;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, m_gBufferFBO);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void DeferredRenderer::endGeometryPass()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void DeferredRenderer::bindGBufferTextures(const ShaderProgram &shader) const
{
    if (!m_initialized) {
        return;
    }

    shader.use();

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_gPosition);
    shader.setInt("gPosition", 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, m_gNormal);
    shader.setInt("gNormal", 1);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, m_gAlbedoSpec);
    shader.setInt("gAlbedoSpec", 2);

    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, m_gMetallicRoughness);
    shader.setInt("gMetallicRoughness", 3);
}

void DeferredRenderer::lightingPass(
    const ShaderProgram &lightingShader, const glm::vec3 &viewPos)
{
    if (!m_initialized) {
        return;
    }

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    lightingShader.use();
    bindGBufferTextures(lightingShader);
    lightingShader.setVec3("viewPos", viewPos);

    // Render screen-filling quad
    glBindVertexArray(m_quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}
