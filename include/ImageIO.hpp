#pragma once

#include <string>
#include <vector>
#include <functional>
#include <memory>

#include <glm/glm.hpp>
#include "GameObject.hpp"
#include "Camera.hpp"

class ARenderer;
struct GLFWwindow;

class ImageIO {
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

    std::vector<GameObject> &m_gameObjects;

    const Camera &m_camera;

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
    ImageIO(std::unique_ptr<ARenderer> &renderer,
        std::vector<GameObject> &gameObjects, const Camera &camera);

    ~ImageIO() = default;

    ImageIO(const ImageIO &) = delete;
    ImageIO &operator=(const ImageIO &) = delete;

    bool addImageObjectAtScreenPos(
        const std::string &path, double mouseX, double mouseY);

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
};
