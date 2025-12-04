#include "TextureManager.hpp"

#include "ImGuiFileDialog.h"
#include "imgui.h"

#include <algorithm>
#include <array>
#include <string>
#include <vector>

TextureManager::TextureManager(SceneGraph &sceneGraph,
    TransformManager &transformManager, RasterizationRenderer &renderer) :
    m_sceneGraph(sceneGraph),
    m_transformManager(transformManager), m_renderer(renderer)
{
}

void TextureManager::renderUI(bool *p_open)
{
    if (p_open && !*p_open) {
        return;
    }

    if (!ImGui::Begin("Texture", p_open)) {
        ImGui::End();
        return;
    }

    renderSelectionPanel();

    ImGui::SeparatorText("Texture Library");
    renderTextureLibraryPanel();

    ImGui::SeparatorText("Normal mapping");
    renderNormalMapping();

    renderMapSelectionPanel();

    ImGui::SeparatorText("Tone Mapping");
    renderToneMappingPanel();

    ImGui::SeparatorText("Cubemap / Environment");
    renderCubemapPanel();

    ImGui::End();
}

void TextureManager::renderSelectionPanel()
{
    const auto &selectedNodes = m_transformManager.getSelectedNodes();

    std::vector<int> objectIds;
    objectIds.reserve(selectedNodes.size());
    for (auto *node : selectedNodes) {
        if (!node) {
            continue;
        }
        int rendererId = node->getData().rendererId;
        if (rendererId >= 0) {
            objectIds.push_back(rendererId);
        }
    }

    if (objectIds.empty()) {
        ImGui::TextDisabled("Select an object to edit its texture.");
        return;
    }

    const auto &textures = m_renderer.getTextureResources();
    std::vector<int> textureHandles;
    std::vector<const char *> textureNames;
    textureHandles.reserve(textures.size());
    textureNames.reserve(textures.size());
    for (const auto &tex : textures) {
        if (tex.target != TextureTarget::Texture2D) {
            continue;
        }
        textureHandles.push_back(tex.handle);
        textureNames.push_back(tex.name.c_str());
    }

    if (objectIds.size() == 1) {
        const int objectId = objectIds.front();
        bool useTexture = m_renderer.getObjectUseTexture(objectId);
        if (ImGui::Checkbox("Use Texture", &useTexture)) {
            m_renderer.setObjectUseTexture(objectId, useTexture);
        }

        if (!textureNames.empty()) {
            int currentHandle = m_renderer.getObjectTextureHandle(objectId);
            int currentIndex = 0;
            for (size_t i = 0; i < textureHandles.size(); ++i) {
                if (textureHandles[i] == currentHandle) {
                    currentIndex = static_cast<int>(i);
                    break;
                }
            }
            if (ImGui::Combo("Texture", &currentIndex, textureNames.data(),
                    static_cast<int>(textureNames.size()))) {
                if (currentIndex >= 0
                    && currentIndex
                        < static_cast<int>(textureHandles.size())) {
                    m_renderer.assignTextureToObject(
                        objectId, textureHandles[currentIndex]);
                    m_renderer.setObjectUseTexture(objectId, true);
                }
            }
        } else {
            ImGui::TextDisabled("No textures available.");
        }

        int filterIndex
            = static_cast<int>(m_renderer.getObjectFilter(objectId));
        if (ImGui::Combo("Filter", &filterIndex,
                RasterizationRenderer::FILTER_MODE_LABELS.data(),
                static_cast<int>(
                    RasterizationRenderer::FILTER_MODE_LABELS.size()))) {
            m_renderer.setObjectFilter(
                objectId, static_cast<FilterMode>(filterIndex));
        }

        int textureHandle = m_renderer.getObjectTextureHandle(objectId);
        if (const TextureResource *res
            = m_renderer.getTextureResource(textureHandle)) {
            ImGui::Text("Selected texture: %s", res->name.c_str());
            ImGui::Text("Size: %dx%d", static_cast<int>(res->size.x),
                static_cast<int>(res->size.y));
            ImGui::Text(
                "Origin: %s", res->procedural ? "Procedural" : "File system");
        }

    } else {
        ImGui::Text("%zu objects selected", objectIds.size());

        int sharedFilter
            = static_cast<int>(m_renderer.getObjectFilter(objectIds.front()));
        for (size_t i = 1; i < objectIds.size(); ++i) {
            if (static_cast<int>(m_renderer.getObjectFilter(objectIds[i]))
                != sharedFilter) {
                sharedFilter = -1;
                break;
            }
        }
        if (sharedFilter == -1) {
            ImGui::TextDisabled("Filters differ across selection.");
        }
        int filterSelection = std::max(sharedFilter, 0);
        if (ImGui::Combo("Filter (applies to all)", &filterSelection,
                RasterizationRenderer::FILTER_MODE_LABELS.data(),
                static_cast<int>(
                    RasterizationRenderer::FILTER_MODE_LABELS.size()))) {
            applyFilterToSelection(static_cast<FilterMode>(filterSelection));
        }

        if (!textureNames.empty()) {
            if (m_selectedTextureIndex
                >= static_cast<int>(textureNames.size())) {
                m_selectedTextureIndex = 0;
            }
            ImGui::Combo("Texture##multi", &m_selectedTextureIndex,
                textureNames.data(), static_cast<int>(textureNames.size()));
            if (ImGui::Button("Assign To Selection")) {
                if (m_selectedTextureIndex >= 0
                    && m_selectedTextureIndex
                        < static_cast<int>(textureHandles.size())) {
                    assignTextureToSelection(
                        textureHandles[m_selectedTextureIndex]);
                }
            }
        } else {
            ImGui::TextDisabled("No textures available to assign.");
        }
    }
}

void TextureManager::renderMapSelectionPanel()
{
    const auto &selectedNodes = m_transformManager.getSelectedNodes();

    std::vector<int> objectIds;
    objectIds.reserve(selectedNodes.size());
    for (auto *node : selectedNodes) {
        if (!node) {
            continue;
        }
        int rendererId = node->getData().rendererId;
        if (rendererId >= 0) {
            objectIds.push_back(rendererId);
        }
    }

    if (objectIds.empty()) {
        ImGui::TextDisabled("Select an object to edit its normal map.");
        return;
    }

    const auto &normalMap = m_renderer.getNormalMapResources();
    std::vector<int> mapHandle;
    std::vector<const char *> mapName;
    mapHandle.reserve(normalMap.size());
    mapName.reserve(normalMap.size());
    for (const auto &normal : normalMap) {
        mapHandle.push_back(normal.handle);
        mapName.push_back(normal.name.c_str());
    }

    if (objectIds.size() == 1) {
        const int objectId = objectIds.front();
        bool useNormalMap = m_renderer.getObjectUseNormalMap(objectId);
        if (ImGui::Checkbox("Use Normal Map", &useNormalMap)) {
            m_renderer.setObjectUseNormalMap(objectId, useNormalMap);
        }

        if (!mapName.empty()) {
            const int currentHandle
                = m_renderer.getObjectNormalMapHandle(objectId);
            int currentIndex = 0;
            for (size_t i = 0; i < mapHandle.size(); ++i) {
                if (mapHandle[i] == currentHandle) {
                    currentIndex = static_cast<int>(i);
                    break;
                }
            }
            if (ImGui::Combo("Normal Map", &currentIndex, mapName.data(),
                    static_cast<int>(mapName.size()))) {
                if (currentIndex >= 0
                    && currentIndex < static_cast<int>(mapHandle.size())) {
                    m_renderer.assignNormalMapToObject(
                        objectId, mapHandle[currentIndex]);
                    m_renderer.setObjectUseNormalMap(objectId, true);
                }
            }

            if (mapName.size() == 1) {
                int onlyHandle = mapHandle[0];
                if (currentHandle != onlyHandle) {
                    m_renderer.assignNormalMapToObject(objectId, onlyHandle);
                    m_renderer.setObjectUseNormalMap(objectId, false);
                }
            }
        } else {
            ImGui::TextDisabled("No map available.");
        }

        int normalMapHandle = m_renderer.getObjectNormalMapHandle(objectId);
        if (const NormalMapResource *res
            = m_renderer.getNormalMapResource(normalMapHandle)) {
            ImGui::Text("Selected normal map: %s", res->name.c_str());
            ImGui::Text("Size: %dx%d", static_cast<int>(res->size.x),
                static_cast<int>(res->size.y));
        }

    } else {
        ImGui::Text("%zu objects selected", objectIds.size());
        if (!mapName.empty()) {
            if (m_selectedNormalMapIndex >= static_cast<int>(mapName.size())) {
                m_selectedNormalMapIndex = 0;
            }
            ImGui::Combo("Map##multi", &m_selectedNormalMapIndex,
                mapName.data(), static_cast<int>(mapName.size()));
            if (ImGui::Button("Assign To Selection")) {
                if (m_selectedNormalMapIndex >= 0
                    && m_selectedNormalMapIndex
                        < static_cast<int>(mapHandle.size())) {
                    assignNormalMapToSelection(
                        mapHandle[m_selectedNormalMapIndex]);
                }
            }
        } else {
            ImGui::TextDisabled("No normal map available to assign.");
        }
    }
}

void TextureManager::renderTextureLibraryPanel()
{
    if (ImGui::Button("Import Texture From File")) {
        IGFD::FileDialogConfig config;
        config.path = "../assets";
        config.countSelectionMax = 1;
        ImGuiFileDialog::Instance()->OpenDialog("TextureImportDialog",
            "Select Texture",
            "Image files{.png,.jpg,.jpeg,.bmp,.tga,.gif,.hdr,.exr}", config);
    }

    if (ImGuiFileDialog::Instance()->Display("TextureImportDialog",
            ImGuiWindowFlags_NoCollapse, ImVec2(520, 400))) {
        if (ImGuiFileDialog::Instance()->IsOk()) {
            std::string filePath
                = ImGuiFileDialog::Instance()->GetFilePathName();
            const int handle = m_renderer.loadTexture2D(filePath, m_useSRGB);
            if (handle >= 0) {
                const auto &textures = m_renderer.getTextureResources();
                int filteredIndex = 0;
                for (const auto &tex : textures) {
                    if (tex.target != TextureTarget::Texture2D) {
                        continue;
                    }
                    if (tex.handle == handle) {
                        m_selectedTextureIndex = filteredIndex;
                        break;
                    }
                    ++filteredIndex;
                }
            }
        }
        ImGuiFileDialog::Instance()->Close();
    }

    ImGui::Checkbox("sRGB", &m_useSRGB);
    ImGui::ColorEdit3("Checker Color A", &m_checkerColorA.x);
    ImGui::ColorEdit3("Checker Color B", &m_checkerColorB.x);
    ImGui::SliderInt("Checker cells", &m_checkerChecks, 2, 64);
    if (ImGui::Button("Create Checkerboard Texture")) {
        const std::string name = "Checkerboard "
            + std::to_string(
                static_cast<int>(m_renderer.getTextureResources().size()) + 1);
        const int handle = m_renderer.createCheckerboardTexture(name, 256, 256,
            m_checkerColorA, m_checkerColorB, m_checkerChecks, m_useSRGB);
        if (handle >= 0) {
            const auto &textures = m_renderer.getTextureResources();
            int filteredIndex = 0;
            for (const auto &tex : textures) {
                if (tex.target != TextureTarget::Texture2D) {
                    continue;
                }
                if (tex.handle == handle) {
                    m_selectedTextureIndex = filteredIndex;
                    break;
                }
                ++filteredIndex;
            }
        }
    }

    ImGui::SliderFloat(
        "Noise frequency", &m_noiseFrequency, 0.5f, 20.0f, "%.2f");
    if (ImGui::Button("Create Noise Texture")) {
        const std::string name = "Noise "
            + std::to_string(
                static_cast<int>(m_renderer.getTextureResources().size()) + 1);
        const int handle = m_renderer.createNoiseTexture(
            name, 192, 192, m_noiseFrequency, m_useSRGB);
        if (handle >= 0) {
            const auto &textures = m_renderer.getTextureResources();
            int filteredIndex = 0;
            for (const auto &tex : textures) {
                if (tex.target != TextureTarget::Texture2D) {
                    continue;
                }
                if (tex.handle == handle) {
                    m_selectedTextureIndex = filteredIndex;
                    break;
                }
                ++filteredIndex;
            }
        }
    }

    ImGui::Separator();
    ImGui::ColorEdit3("Solid Color", &m_solidColor.x);
    if (ImGui::Button("Create Solid Color Texture")) {
        const std::string name = "Solid Color "
            + std::to_string(
                static_cast<int>(m_renderer.getTextureResources().size()) + 1);
        const int handle = m_renderer.createSolidColorTexture(
            name, m_solidColor, 1, 1, m_useSRGB);
        if (handle >= 0) {
            const auto &textures = m_renderer.getTextureResources();
            int filteredIndex = 0;
            for (const auto &tex : textures) {
                if (tex.target != TextureTarget::Texture2D) {
                    continue;
                }
                if (tex.handle == handle) {
                    m_selectedTextureIndex = filteredIndex;
                    break;
                }
                ++filteredIndex;
            }
        }
    }

    const auto &textures2D = m_renderer.getTextureResources();
    bool has2DTextures = false;
    for (const auto &tex : textures2D) {
        if (tex.target == TextureTarget::Texture2D) {
            has2DTextures = true;
            break;
        }
    }
    if (has2DTextures) {
        ImGui::SeparatorText("Available Textures");
        ImGui::BeginChild(
            "texture_list", ImVec2(0, 140), true, ImGuiWindowFlags_None);
        for (const auto &tex : textures2D) {
            if (tex.target != TextureTarget::Texture2D) {
                continue;
            }
            ImGui::BulletText("%s%s (%dx%d)", tex.name.c_str(),
                tex.procedural ? " [procedural]" : "",
                static_cast<int>(tex.size.x), static_cast<int>(tex.size.y));
        }
        ImGui::EndChild();
    } else {
        ImGui::TextDisabled("No textures generated yet.");
    }
}

void TextureManager::renderNormalMapping()
{
    if (ImGui::Button("Import normal map From File")) {
        IGFD::FileDialogConfig config;
        config.path = "../assets";
        config.countSelectionMax = 1;
        ImGuiFileDialog::Instance()->OpenDialog("NormalMapImportDialog",
            "Select Map",
            "Image files{.png,.jpg,.jpeg,.bmp,.tga,.dds,.ktx,.ktx2}", config);
    }

    if (ImGuiFileDialog::Instance()->Display("NormalMapImportDialog",
            ImGuiWindowFlags_NoCollapse, ImVec2(520, 400))) {
        if (ImGuiFileDialog::Instance()->IsOk()) {
            std::string filePath
                = ImGuiFileDialog::Instance()->GetFilePathName();
            const int handle = m_renderer.loadNormalMap(filePath);
            if (handle >= 0) {
                const auto &normalMaps = m_renderer.getTextureResources();
                int filteredIndex = 0;
                for (const auto &normal : normalMaps) {
                    if (normal.handle == handle) {
                        m_selectedNormalMapIndex = filteredIndex;
                        break;
                    }
                    ++filteredIndex;
                }
            }
        }
        ImGuiFileDialog::Instance()->Close();
    }
}

void TextureManager::renderToneMappingPanel()
{
    int toneIndex = static_cast<int>(m_renderer.getToneMappingMode());
    if (ImGui::Combo("Operator", &toneIndex,
            RasterizationRenderer::TONEMAP_LABELS.data(),
            static_cast<int>(RasterizationRenderer::TONEMAP_LABELS.size()))) {
        m_renderer.setToneMappingMode(static_cast<ToneMappingMode>(toneIndex));
    }

    float exposure = m_renderer.getToneMappingExposure();
    if (ImGui::SliderFloat("Exposure", &exposure, 0.1f, 5.0f, "%.2f")) {
        m_renderer.setToneMappingExposure(exposure);
    }
}

void TextureManager::renderCubemapPanel()
{
    if (ImGui::Button("Load Equirectangular Image")) {
        IGFD::FileDialogConfig config;
        config.path = "../assets";
        config.countSelectionMax = 1;
        ImGuiFileDialog::Instance()->OpenDialog("EquirectDialog",
            "Select Equirectangular Image",
            "Image files{.png,.jpg,.jpeg,.bmp,.tga,.hdr}", config);
    }

    if (ImGuiFileDialog::Instance()->Display(
            "EquirectDialog", ImGuiWindowFlags_NoCollapse, ImVec2(520, 400))) {
        if (ImGuiFileDialog::Instance()->IsOk()) {
            std::string filePath
                = ImGuiFileDialog::Instance()->GetFilePathName();
            const std::string cubemapName = "Equirectangular Skybox "
                + std::to_string(
                    static_cast<int>(m_renderer.getCubemapHandles().size())
                    + 1);
            int newCubemapHandle = m_renderer.loadCubemapFromEquirectangular(
                cubemapName, filePath, 512, m_useSRGB);
            // Automatically set the new cubemap as active skybox
            if (newCubemapHandle >= 0) {
                m_renderer.setActiveCubemap(newCubemapHandle);
            }
        }
        ImGuiFileDialog::Instance()->Close();
    }

    ImGui::Separator();

    const auto &handles = m_renderer.getCubemapHandles();
    if (handles.empty()) {
        ImGui::TextDisabled("No cubemaps available.");
        return;
    }

    std::vector<const char *> cubemapNames;
    std::vector<int> validHandles;
    cubemapNames.reserve(handles.size());
    validHandles.reserve(handles.size());
    int activeHandle = m_renderer.getActiveCubemap();
    int activeIndex = 0;

    for (size_t i = 0; i < handles.size(); ++i) {
        const TextureResource *res = m_renderer.getTextureResource(handles[i]);
        if (!res) {
            continue;
        }
        validHandles.push_back(handles[i]);
        cubemapNames.push_back(res->name.c_str());
        if (handles[i] == activeHandle) {
            activeIndex = static_cast<int>(validHandles.size()) - 1;
        }
    }

    if (cubemapNames.empty()) {
        ImGui::TextDisabled("No valid cubemap resources.");
        return;
    }

    if (activeIndex >= static_cast<int>(cubemapNames.size())) {
        activeIndex = 0;
    }

    if (ImGui::Combo("Skybox", &activeIndex, cubemapNames.data(),
            static_cast<int>(cubemapNames.size()))) {
        if (activeIndex >= 0
            && activeIndex < static_cast<int>(validHandles.size())) {
            m_renderer.setActiveCubemap(validHandles[activeIndex]);
        }
    }
}

void TextureManager::assignTextureToSelection(int textureHandle)
{
    if (textureHandle < 0) {
        return;
    }
    const TextureResource *res = m_renderer.getTextureResource(textureHandle);
    if (!res || res->target != TextureTarget::Texture2D) {
        return;
    }
    for (auto *node : m_transformManager.getSelectedNodes()) {
        if (!node) {
            continue;
        }
        int rendererId = node->getData().rendererId;
        if (rendererId >= 0) {
            m_renderer.assignTextureToObject(rendererId, textureHandle);
            m_renderer.setObjectUseTexture(rendererId, true);
        }
    }
}

void TextureManager::assignNormalMapToSelection(int normalMapHandle)
{
    if (normalMapHandle < 0) {
        return;
    }
    if (const NormalMapResource *res
        = m_renderer.getNormalMapResource(normalMapHandle);
        !res) {
        return;
    }
    for (auto *node : m_transformManager.getSelectedNodes()) {
        if (!node) {
            continue;
        }
        if (const int rendererId = node->getData().rendererId;
            rendererId >= 0) {
            m_renderer.assignNormalMapToObject(rendererId, normalMapHandle);
            m_renderer.setObjectUseNormalMap(rendererId, true);
        }
    }
}

void TextureManager::applyFilterToSelection(FilterMode mode)
{
    for (auto *node : m_transformManager.getSelectedNodes()) {
        if (!node) {
            continue;
        }
        int rendererId = node->getData().rendererId;
        if (rendererId >= 0) {
            m_renderer.setObjectFilter(rendererId, mode);
        }
    }
}
