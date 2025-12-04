#pragma once

#include "ShaderProgram.hpp"
#include <glm/glm.hpp>
#include <string>
#include <unordered_map>

class IBLManager {
public:
    struct IBLTextures {
        unsigned int irradianceMap = 0; // Diffuse IBL cubemap
        unsigned int prefilterMap = 0; // Specular IBL cubemap (mipmapped)
        unsigned int brdfLUT = 0; // 2D BRDF integration LUT
        bool valid = false;
    };

    IBLManager();
    ~IBLManager();

    IBLManager(const IBLManager &) = delete;
    IBLManager &operator=(const IBLManager &) = delete;

    // Generate IBL textures from an environment cubemap
    IBLTextures generateIBLFromCubemap(
        unsigned int envCubemap, int cubemapSize = 512);

    // Get IBL textures for a given environment cubemap
    const IBLTextures *getIBLTextures(unsigned int envCubemap) const;

    // Bind IBL textures to shader (uses texture units 5, 6, 7)
    void bindIBLTextures(
        const ShaderProgram &shader, const IBLTextures &textures) const;

    // Get or generate BRDF LUT (shared across all environments)
    unsigned int getBRDFLUT();

private:
    void initializeCaptureFBO(int size);
    void cleanupCaptureFBO();

    unsigned int generateIrradianceMap(unsigned int envCubemap, int size = 32);
    unsigned int generatePrefilterMap(unsigned int envCubemap, int size = 128);
    unsigned int generateBRDFLUT(int size = 512);

    void renderCube() const;

    // Capture framebuffer
    unsigned int m_captureFBO = 0;
    unsigned int m_captureRBO = 0;

    // Shaders for IBL generation
    ShaderProgram m_irradianceShader;
    ShaderProgram m_prefilterShader;
    ShaderProgram m_brdfShader;

    // Cube geometry for rendering
    unsigned int m_cubeVAO = 0;
    unsigned int m_cubeVBO = 0;

    // Quad geometry for BRDF LUT
    unsigned int m_quadVAO = 0;
    unsigned int m_quadVBO = 0;

    // Shared BRDF LUT (same for all environments)
    unsigned int m_brdfLUT = 0;

    // Cache of generated IBL textures per environment cubemap
    std::unordered_map<unsigned int, IBLTextures> m_iblCache;

    // View matrices for cubemap faces
    static const glm::mat4 s_captureViews[6];
    static const glm::mat4 s_captureProjection;
};
