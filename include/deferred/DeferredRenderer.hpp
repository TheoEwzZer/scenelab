#pragma once

#include <glad/gl.h>
#include <glm/glm.hpp>
#include <vector>
#include "ShaderProgram.hpp"

class DeferredRenderer {
public:
    DeferredRenderer();
    ~DeferredRenderer();

    // Initialize G-Buffer with given dimensions
    bool initialize(int width, int height);

    // Resize G-Buffer when window size changes
    void resize(int width, int height);

    // Begin geometry pass - bind G-Buffer for writing
    void beginGeometryPass();

    // End geometry pass
    void endGeometryPass();

    // Perform lighting pass
    void lightingPass(
        const ShaderProgram &lightingShader, const glm::vec3 &viewPos);

    // Bind G-Buffer textures for reading
    void bindGBufferTextures(const ShaderProgram &shader) const;

    // Get G-Buffer textures for debug visualization
    unsigned int getPositionTexture() const { return m_gPosition; }

    unsigned int getNormalTexture() const { return m_gNormal; }

    unsigned int getAlbedoSpecTexture() const { return m_gAlbedoSpec; }

    unsigned int getMetallicRoughnessTexture() const
    {
        return m_gMetallicRoughness;
    }

    // Get framebuffer for direct access
    unsigned int getGBufferFBO() const { return m_gBufferFBO; }

    // Check if deferred rendering is enabled
    bool isEnabled() const { return m_enabled; }

    void setEnabled(bool enabled) { m_enabled = enabled; }

    // Get dimensions
    int getWidth() const { return m_width; }

    int getHeight() const { return m_height; }

private:
    void cleanup();
    void setupQuad();

    unsigned int m_gBufferFBO = 0;
    unsigned int m_gPosition = 0;
    unsigned int m_gNormal = 0;
    unsigned int m_gAlbedoSpec = 0;
    unsigned int m_gMetallicRoughness = 0;
    unsigned int m_rboDepth = 0;

    unsigned int m_quadVAO = 0;
    unsigned int m_quadVBO = 0;

    int m_width = 0;
    int m_height = 0;
    bool m_enabled = false;
    bool m_initialized = false;
};
