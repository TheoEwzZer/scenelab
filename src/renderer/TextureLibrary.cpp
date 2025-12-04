#include "renderer/TextureLibrary.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <iostream>
#include <limits>
#include <random>
#include <utility>

#include <glad/gl.h>

#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

namespace {
constexpr std::array<glm::vec3, 6> DEFAULT_REALISTIC_SKY {
    glm::vec3(0.55f, 0.70f, 0.95f), glm::vec3(0.60f, 0.75f, 0.92f),
    glm::vec3(0.30f, 0.55f, 0.95f), glm::vec3(0.80f, 0.85f, 0.95f),
    glm::vec3(0.45f, 0.65f, 0.93f), glm::vec3(0.50f, 0.68f, 0.90f)
};

constexpr std::array<glm::vec3, 6> DEFAULT_NEON_CUBE {
    glm::vec3(0.20f, 0.85f, 0.65f), glm::vec3(0.95f, 0.35f, 0.60f),
    glm::vec3(0.25f, 0.30f, 0.90f), glm::vec3(0.85f, 0.80f, 0.25f),
    glm::vec3(0.10f, 0.10f, 0.15f), glm::vec3(0.65f, 0.20f, 0.85f)
};
} // namespace

TextureLibrary::TextureLibrary() = default;

TextureLibrary::~TextureLibrary()
{
    for (const auto &resource : m_texturePool) {
        if (resource.id != 0) {
            glDeleteTextures(1, &resource.id);
        }
    }
}

int TextureLibrary::loadTexture2D(const std::string &filepath, bool srgb)
{
    return loadTextureInternal(filepath, srgb);
}

int TextureLibrary::loadNormalMap(const std::string &filepath)
{
    if (filepath.empty()) {
        return -1;
    }

    if (const auto it = m_normalMapHandleByPath.find(filepath);
        it != m_normalMapHandleByPath.end()) {
        return it->second;
    }

    int width = 0;
    int height = 0;
    int channels = 0;
    stbi_set_flip_vertically_on_load(true);
    unsigned char *data
        = stbi_load(filepath.c_str(), &width, &height, &channels, 0);

    if (!data || width <= 0 || height <= 0 || channels == 0) {
        std::cerr << "[ERROR] Failed to load texture: " << filepath << '\n';
        if (data) {
            stbi_image_free(data);
        }
        return -1;
    }

    GLenum format = GL_RGB;
    switch (channels) {
        case 1:
            format = GL_RED;
            break;
        case 2:
            format = GL_RG;
            break;
        case 3:
            format = GL_RGB;
            break;
        case 4:
        default:
            format = GL_RGBA;
            break;
    }

    unsigned int mapId = 0;
    glGenTextures(1, &mapId);
    glBindTexture(GL_TEXTURE_2D, mapId);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(
        GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, format,
        GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    stbi_image_free(data);

    NormalMapResource res;
    res.id = mapId;
    res.size = { width, height };
    res.name = filepath;
    res.sourcePath = filepath;

    const int handle = storeNormalMapResource(std::move(res));
    if (handle >= 0) {
        m_normalMapHandleByPath[filepath] = handle;
    }
    return handle;
}

int TextureLibrary::createCheckerboardTexture(const std::string &name,
    int width, int height, const glm::vec3 &colorA, const glm::vec3 &colorB,
    int checks, bool srgb)
{
    if (width <= 0 || height <= 0 || checks <= 0) {
        return -1;
    }

    std::vector<unsigned char> buffer(
        static_cast<size_t>(width) * static_cast<size_t>(height) * 4);

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            const bool useColorA = ((x / std::max(1, width / checks))
                                       + (y / std::max(1, height / checks)))
                    % 2
                == 0;
            const glm::vec3 color = useColorA ? colorA : colorB;
            const size_t idx
                = (static_cast<size_t>(y) * static_cast<size_t>(width)
                      + static_cast<size_t>(x))
                * 4u;
            buffer[idx + 0] = static_cast<unsigned char>(
                glm::clamp(color.r, 0.0f, 1.0f) * 255.0f);
            buffer[idx + 1] = static_cast<unsigned char>(
                glm::clamp(color.g, 0.0f, 1.0f) * 255.0f);
            buffer[idx + 2] = static_cast<unsigned char>(
                glm::clamp(color.b, 0.0f, 1.0f) * 255.0f);
            buffer[idx + 3] = 255;
        }
    }

    return createProceduralTexture(name, width, height, buffer, srgb);
}

int TextureLibrary::createNoiseTexture(
    const std::string &name, int width, int height, float frequency, bool srgb)
{
    if (width <= 0 || height <= 0) {
        return -1;
    }

    std::vector<unsigned char> buffer(
        static_cast<size_t>(width) * static_cast<size_t>(height) * 4);

    std::mt19937 rng(std::random_device {}());
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            const float nx = static_cast<float>(x) / static_cast<float>(width);
            const float ny
                = static_cast<float>(y) / static_cast<float>(height);
            const float value = 0.5f
                    * (1.0f
                        + std::sin(
                            (nx + ny) * frequency * glm::two_pi<float>()))
                    * 0.7f
                + 0.3f * dist(rng);
            const unsigned char c = static_cast<unsigned char>(
                glm::clamp(value, 0.0f, 1.0f) * 255.0f);
            const size_t idx
                = (static_cast<size_t>(y) * static_cast<size_t>(width)
                      + static_cast<size_t>(x))
                * 4u;
            buffer[idx + 0] = c;
            buffer[idx + 1] = c;
            buffer[idx + 2] = c;
            buffer[idx + 3] = 255;
        }
    }

    return createProceduralTexture(name, width, height, buffer, srgb);
}

int TextureLibrary::createSolidColorTexture(const std::string &name,
    const glm::vec3 &color, int width, int height, bool srgb)
{
    if (width <= 0 || height <= 0) {
        return -1;
    }

    std::vector<unsigned char> buffer(
        static_cast<size_t>(width) * static_cast<size_t>(height) * 4);

    const unsigned char r
        = static_cast<unsigned char>(glm::clamp(color.r, 0.0f, 1.0f) * 255.0f);
    const unsigned char g
        = static_cast<unsigned char>(glm::clamp(color.g, 0.0f, 1.0f) * 255.0f);
    const unsigned char b
        = static_cast<unsigned char>(glm::clamp(color.b, 0.0f, 1.0f) * 255.0f);

    for (size_t i = 0; i < buffer.size(); i += 4) {
        buffer[i + 0] = r;
        buffer[i + 1] = g;
        buffer[i + 2] = b;
        buffer[i + 3] = 255;
    }

    return createProceduralTexture(name, width, height, buffer, srgb);
}

int TextureLibrary::createColoredCubemap(const std::string &name,
    const std::array<glm::vec3, 6> &faceColors, int edgeSize, bool srgb)
{
    std::array<std::vector<unsigned char>, 6> faces;
    const size_t faceSize
        = static_cast<size_t>(edgeSize) * static_cast<size_t>(edgeSize) * 4;
    for (size_t i = 0; i < faces.size(); ++i) {
        faces[i].resize(faceSize);
        for (size_t p = 0; p < faceSize; p += 4) {
            faces[i][p + 0] = static_cast<unsigned char>(
                glm::clamp(faceColors[i].r, 0.0f, 1.0f) * 255.0f);
            faces[i][p + 1] = static_cast<unsigned char>(
                glm::clamp(faceColors[i].g, 0.0f, 1.0f) * 255.0f);
            faces[i][p + 2] = static_cast<unsigned char>(
                glm::clamp(faceColors[i].b, 0.0f, 1.0f) * 255.0f);
            faces[i][p + 3] = 255;
        }
    }

    return createCubemap(name, edgeSize, faces, srgb);
}

int TextureLibrary::loadCubemapFromEquirectangular(const std::string &name,
    const std::string &equirectPath, int resolution, bool srgb)
{
    if (equirectPath.empty() || resolution <= 0) {
        return -1;
    }

    int width = 0;
    int height = 0;
    int channels = 0;
    stbi_set_flip_vertically_on_load(false);
    unsigned char *data
        = stbi_load(equirectPath.c_str(), &width, &height, &channels, 0);

    if (!data || width <= 0 || height <= 0 || channels == 0) {
        std::cerr << "[ERROR] Failed to load equirectangular image: "
                  << equirectPath << '\n';
        if (data) {
            stbi_image_free(data);
        }
        return -1;
    }

    std::array<std::vector<unsigned char>, 6> faces;
    const size_t faceSize = static_cast<size_t>(resolution)
        * static_cast<size_t>(resolution) * 4;
    for (auto &face : faces) {
        face.resize(faceSize);
    }

    const float invRes = 1.0f / static_cast<float>(resolution);
    const float pi = glm::pi<float>();

    for (int faceIdx = 0; faceIdx < 6; ++faceIdx) {
        for (int y = 0; y < resolution; ++y) {
            for (int x = 0; x < resolution; ++x) {
                const float u
                    = (static_cast<float>(x) + 0.5f) * invRes * 2.0f - 1.0f;
                const float v
                    = (static_cast<float>(y) + 0.5f) * invRes * 2.0f - 1.0f;

                glm::vec3 dir;
                switch (faceIdx) {
                    case 0:
                        dir = glm::normalize(glm::vec3(1.0f, -v, -u));
                        break;
                    case 1:
                        dir = glm::normalize(glm::vec3(-1.0f, -v, u));
                        break;
                    case 2:
                        dir = glm::normalize(glm::vec3(u, 1.0f, v));
                        break;
                    case 3:
                        dir = glm::normalize(glm::vec3(u, -1.0f, -v));
                        break;
                    case 4:
                        dir = glm::normalize(glm::vec3(u, -v, 1.0f));
                        break;
                    case 5:
                        dir = glm::normalize(glm::vec3(-u, -v, -1.0f));
                        break;
                }

                const float theta = std::atan2(dir.z, dir.x);
                const float phi = std::acos(glm::clamp(dir.y, -1.0f, 1.0f));

                const float uEqui = (theta / pi + 1.0f) * 0.5f;
                const float vEqui = phi / pi;

                const int srcX = static_cast<int>(
                    glm::clamp(uEqui, 0.0f, 1.0f) * static_cast<float>(width));
                const int srcY = static_cast<int>(glm::clamp(vEqui, 0.0f, 1.0f)
                    * static_cast<float>(height));

                const int srcIdx = (srcY * width + srcX) * channels;
                const size_t dstIdx
                    = (static_cast<size_t>(y) * static_cast<size_t>(resolution)
                          + static_cast<size_t>(x))
                    * 4u;

                if (channels >= 3) {
                    faces[faceIdx][dstIdx + 0] = data[srcIdx + 0];
                    faces[faceIdx][dstIdx + 1] = data[srcIdx + 1];
                    faces[faceIdx][dstIdx + 2] = data[srcIdx + 2];
                    faces[faceIdx][dstIdx + 3]
                        = channels >= 4 ? data[srcIdx + 3] : 255;
                } else {
                    faces[faceIdx][dstIdx + 0] = data[srcIdx];
                    faces[faceIdx][dstIdx + 1] = data[srcIdx];
                    faces[faceIdx][dstIdx + 2] = data[srcIdx];
                    faces[faceIdx][dstIdx + 3] = 255;
                }
            }
        }
    }

    stbi_image_free(data);

    const int handle = createCubemap(name, resolution, faces, srgb);
    if (handle >= 0) {
        if (auto *res
            = const_cast<TextureResource *>(getTextureResource(handle))) {
            res->procedural = false;
            res->sourcePath = equirectPath;
        }
    }
    return handle;
}

void TextureLibrary::ensureDefaultTextures()
{
    if (!m_texturePool.empty()) {
        return;
    }

    createProceduralTexture("Default White", 1, 1,
        std::vector<unsigned char> { 255, 255, 255, 255 }, false);

    createCheckerboardTexture("Procedural Checkerboard 1", 256, 256,
        glm::vec3(0.9f, 0.9f, 0.9f), glm::vec3(0.2f, 0.2f, 0.25f), 16, true);

    createNoiseTexture("Procedural Noise 1", 192, 192, 5.0f, false);
}

void TextureLibrary::ensureDefaultCubemaps()
{
    if (!m_cubemapHandles.empty()) {
        return;
    }

    createColoredCubemap("Realistic Sky", DEFAULT_REALISTIC_SKY, 128, true);
    createColoredCubemap("Neon Cube", DEFAULT_NEON_CUBE, 64, true);
}

const TextureResource *TextureLibrary::getTextureResource(int handle) const
{
    if (handle < 0 || handle >= static_cast<int>(m_texturePool.size())) {
        return nullptr;
    }
    return &m_texturePool[handle];
}

const NormalMapResource *TextureLibrary::getNormalMapResource(int handle) const
{
    if (handle < 0 || handle >= static_cast<int>(m_normalMapPool.size())) {
        return nullptr;
    }
    return &m_normalMapPool[handle];
}

void TextureLibrary::setActiveCubemap(int cubemapHandle)
{
    const TextureResource *res = getTextureResource(cubemapHandle);
    if (!res || res->target != TextureTarget::Cubemap) {
        return;
    }
    m_activeCubemap = cubemapHandle;
}

int TextureLibrary::storeTextureResource(TextureResource &&resource)
{
    if (resource.id == 0) {
        return -1;
    }
    resource.handle = static_cast<int>(m_texturePool.size());
    m_texturePool.push_back(std::move(resource));
    return m_texturePool.back().handle;
}

int TextureLibrary::storeNormalMapResource(NormalMapResource &&resource)
{
    if (resource.id == 0) {
        return -1;
    }
    resource.handle = static_cast<int>(m_normalMapPool.size());
    m_normalMapPool.push_back(std::move(resource));
    return m_normalMapPool.back().handle;
}

int TextureLibrary::loadTextureInternal(const std::string &filepath, bool srgb)
{
    if (filepath.empty()) {
        return -1;
    }

    if (auto it = m_textureHandleByPath.find(filepath);
        it != m_textureHandleByPath.end()) {
        return it->second;
    }

    int width = 0;
    int height = 0;
    int channels = 0;
    stbi_set_flip_vertically_on_load(true);
    unsigned char *data
        = stbi_load(filepath.c_str(), &width, &height, &channels, 0);

    if (!data || width <= 0 || height <= 0 || channels == 0) {
        std::cerr << "[ERROR] Failed to load texture: " << filepath << '\n';
        if (data) {
            stbi_image_free(data);
        }
        return -1;
    }

    GLenum format = GL_RGB;
    GLenum internalFormat = GL_RGB;
    switch (channels) {
        case 1:
            format = internalFormat = GL_RED;
            break;
        case 2:
            format = internalFormat = GL_RG;
            break;
        case 3:
            format = GL_RGB;
            internalFormat = srgb ? GL_SRGB : GL_RGB;
            break;
        case 4:
        default:
            format = GL_RGBA;
            internalFormat = srgb ? GL_SRGB_ALPHA : GL_RGBA;
            break;
    }

    unsigned int textureId = 0;
    glGenTextures(1, &textureId);
    glBindTexture(GL_TEXTURE_2D, textureId);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(
        GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format,
        GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    stbi_image_free(data);

    TextureResource res;
    res.id = textureId;
    res.target = TextureTarget::Texture2D;
    res.size = { width, height };
    res.name = filepath;
    res.procedural = false;
    res.srgb = srgb;
    res.sourcePath = filepath;

    const int handle = storeTextureResource(std::move(res));
    if (handle >= 0) {
        m_textureHandleByPath[filepath] = handle;
    }
    return handle;
}

int TextureLibrary::createProceduralTexture(const std::string &name, int width,
    int height, const std::vector<unsigned char> &data, bool srgb)
{
    if (width <= 0 || height <= 0
        || data.size()
            != static_cast<size_t>(width) * static_cast<size_t>(height) * 4) {
        return -1;
    }

    unsigned int textureId = 0;
    glGenTextures(1, &textureId);
    glBindTexture(GL_TEXTURE_2D, textureId);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    GLenum internalFormat = srgb ? GL_SRGB_ALPHA : GL_RGBA;
    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, GL_RGBA,
        GL_UNSIGNED_BYTE, data.data());
    glGenerateMipmap(GL_TEXTURE_2D);

    TextureResource res;
    res.id = textureId;
    res.target = TextureTarget::Texture2D;
    res.size = { width, height };
    res.name = name;
    res.procedural = true;
    res.srgb = srgb;

    return storeTextureResource(std::move(res));
}

int TextureLibrary::createCubemap(const std::string &name, int edgeSize,
    const std::array<std::vector<unsigned char>, 6> &faces, bool srgb)
{
    if (edgeSize <= 0) {
        return -1;
    }

    const size_t expectedFaceSize
        = static_cast<size_t>(edgeSize) * static_cast<size_t>(edgeSize) * 4;
    for (size_t i = 0; i < faces.size(); ++i) {
        if (faces[i].size() != expectedFaceSize) {
            std::cerr << "[ERROR] Cubemap '" << name << "' face " << i
                      << " has invalid size (expected " << expectedFaceSize
                      << ", got " << faces[i].size() << ")\n";
            return -1;
        }
    }

    unsigned int textureId = 0;
    glGenTextures(1, &textureId);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureId);

    GLenum internalFormat = srgb ? GL_SRGB_ALPHA : GL_RGBA;
    for (size_t face = 0; face < faces.size(); ++face) {
        glTexImage2D(
            GL_TEXTURE_CUBE_MAP_POSITIVE_X + static_cast<GLenum>(face), 0,
            internalFormat, edgeSize, edgeSize, 0, GL_RGBA, GL_UNSIGNED_BYTE,
            faces[face].data());
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    TextureResource res;
    res.id = textureId;
    res.target = TextureTarget::Cubemap;
    res.size = { edgeSize, edgeSize };
    res.name = name;
    res.procedural = true;
    res.srgb = srgb;

    const int handle = storeTextureResource(std::move(res));
    if (handle >= 0) {
        m_cubemapHandles.push_back(handle);
        if (m_activeCubemap < 0) {
            m_activeCubemap = handle;
        }
    }
    return handle;
}
