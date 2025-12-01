#pragma once

#include <array>
#include <string>
#include <unordered_map>
#include <vector>

#include <glm/glm.hpp>

enum class TextureTarget : int { None = 0, Texture2D, Cubemap };

struct TextureResource {
    int handle = -1;
    unsigned int id = 0;
    TextureTarget target = TextureTarget::None;
    glm::ivec2 size { 1, 1 };
    std::string name;
    bool procedural = false;
    bool srgb = false;
    std::string sourcePath;
};

struct NormalMapResource {
    int handle = -1;
    unsigned int id = 0;
    glm::ivec2 size { 1, 1 };
    std::string name;
    std::string sourcePath;
};

class TextureLibrary {
public:
    TextureLibrary();
    ~TextureLibrary();

    TextureLibrary(const TextureLibrary &) = delete;
    TextureLibrary &operator=(const TextureLibrary &) = delete;

    int loadTexture2D(const std::string &filepath, bool srgb = false);
    int loadNormalMap(const std::string &filepath);
    int createCheckerboardTexture(const std::string &name, int width,
        int height, const glm::vec3 &colorA, const glm::vec3 &colorB,
        int checks = 8, bool srgb = false);
    int createNoiseTexture(const std::string &name, int width, int height,
        float frequency = 8.0f, bool srgb = false);
    int createSolidColorTexture(const std::string &name,
        const glm::vec3 &color, int width = 1, int height = 1,
        bool srgb = false);
    int createColoredCubemap(const std::string &name,
        const std::array<glm::vec3, 6> &faceColors, int edgeSize = 32,
        bool srgb = false);
    int loadCubemapFromEquirectangular(const std::string &name,
        const std::string &equirectPath, int resolution = 512,
        bool srgb = false);

    void ensureDefaultTextures();
    void ensureDefaultCubemaps();

    const TextureResource *getTextureResource(int handle) const;

    const std::vector<TextureResource> &getTextureResources() const
    {
        return m_texturePool;
    }

    const NormalMapResource *getNormalMapResource(int handle) const;

    const std::vector<NormalMapResource> &getNormalMapResources() const
    {
        return m_normalMapPool;
    }

    const std::vector<int> &getCubemapHandles() const
    {
        return m_cubemapHandles;
    }

    void setActiveCubemap(int cubemapHandle);

    int getActiveCubemap() const { return m_activeCubemap; }

private:
    int storeTextureResource(TextureResource &&resource);
    int storeNormalMapResource(NormalMapResource &&resource);
    int loadTextureInternal(const std::string &filepath, bool srgb);
    int createProceduralTexture(const std::string &name, int width, int height,
        const std::vector<unsigned char> &data, bool srgb = false);
    int createCubemap(const std::string &name, int edgeSize,
        const std::array<std::vector<unsigned char>, 6> &faces,
        bool srgb = false);

    std::vector<TextureResource> m_texturePool;
    std::vector<NormalMapResource> m_normalMapPool;
    std::unordered_map<std::string, int> m_textureHandleByPath;
    std::unordered_map<std::string, int> m_normalMapHandleByPath;
    std::vector<int> m_cubemapHandles;
    int m_activeCubemap = -1;
};
