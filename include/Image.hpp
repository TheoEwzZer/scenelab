#pragma once

#include <string>
#include <vector>
#include <functional>
#include <memory>
#include <array>

#include "SceneGraph.hpp"
#include <glm/glm.hpp>
#include "GameObject.hpp"
#include "CameraManager.hpp"

class ARenderer;
struct GLFWwindow;

class Image {
public:
    struct ExportSettings {
        std::string outputDir = "./";
        std::string baseName = "frame";
        int frameCount = 10;
    };

    struct StatusMessage {
        std::string text;
        float timer = 0.0f;
        bool isError = false;
    };

private:
    std::unique_ptr<ARenderer> &m_renderer;

    SceneGraph &m_sceneGraph;


    const CameraManager &m_cameraManager;

    ExportSettings m_exportSettings;

    StatusMessage m_statusMessage;

    bool m_exportActive = false;
    int m_exportCurrentIndex = 0;
    std::function<void(bool, const std::string &)> m_exportCallback;

    bool isValidImageFile(const std::string &path) const;

    glm::vec3 screenToWorldPosition(double mouseX, double mouseY) const;

    std::vector<float> createImageQuadVertices(
        float width = 1.0f, float height = 1.0f) const;

    void captureAndWriteCurrentFrame(GLFWwindow *window);

public:
    Image(std::unique_ptr<ARenderer> &renderer,
        SceneGraph &sceneGraph, const CameraManager &cameraManager);

    ~Image() = default;

    Image(const Image &) = delete;
    Image &operator=(const Image &) = delete;

    bool addImageObjectAtScreenPos(
        const std::string &path, double mouseX, double mouseY);

    // Callback invoked when a new image object is created
    using OnImageObjectCreated = std::function<void(SceneGraph::Node*)>;
    void setOnImageObjectCreatedCallback(OnImageObjectCreated callback) {
        m_onImageObjectCreated = callback;
    }

    void startFrameExport(
        std::function<void(bool success, const std::string &message)>
            callback);

    void updateMessageTimer(float deltaTime);

    void renderUI();

    bool isExportInProgress() const;

    void handleFrameExport(GLFWwindow *window);

    ExportSettings &getExportSettings() { return m_exportSettings; }

    const ExportSettings &getExportSettings() const
    {
        return m_exportSettings;
    }

    const StatusMessage &getStatusMessage() const { return m_statusMessage; }

    void setStatusMessage(const std::string &message, float duration = 5.0f,
        bool isError = false);

    bool consumeSelectedPaletteColor(glm::vec4 &outColor);

private:
    // Callback for when new image objects are created
    OnImageObjectCreated m_onImageObjectCreated;

    // Color palette
    std::vector<glm::vec4> m_paletteColors;
    int m_selectedPaletteIndex = 0;
    bool m_paletteChanged = false;

    // Imported images tracking for sampling
    struct ImportedImage {
        std::string path;
        bool selected = true;
    };

    std::vector<ImportedImage> m_importedImages;

    // Source image data for sampling
    struct Src {
        unsigned char *pixels = nullptr;
        int w = 0;
        int h = 0;
        int channels = 0;
    };

    // Sampling configuration
    int m_sampleOutWidth = 512;
    int m_sampleOutHeight = 512;
    int m_tileSize = 8;
    int m_sampleCounter = 0;

public:
    // Sampling helpers
    void addImportedImagePath(const std::string &path);
    bool generateSampledImageAtCenter();

private:
    // Histogram
    bool computeHistogramForPath(const std::string &path);
    std::array<float, 256> m_histogramBins = {};
    bool m_histogramAvailable = false;
    std::string m_histogramSourceName;
    int m_histogramSelectedIndex = -1;
};
