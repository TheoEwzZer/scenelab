#include "ImageIO.hpp"
#include "renderer/interface/ARenderer.hpp"
#include <algorithm>
#include <cctype>
#include <iostream>
#include <cmath>
#include <cstring>
#include <cstdio>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/geometric.hpp>
#include "imgui.h"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glad/gl.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

ImageIO::ImageIO(std::unique_ptr<ARenderer> &renderer,
    std::vector<GameObject> &gameObjects, const Camera &camera) :
    m_renderer(renderer),
    m_gameObjects(gameObjects), m_camera(camera)
{
}

bool ImageIO::isValidImageFile(const std::string &path) const
{
    size_t dotPos = path.find_last_of(".");
    if (dotPos == std::string::npos) {
        return false;
    }

    std::string ext = path.substr(dotPos + 1);
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

    return (ext == "png" || ext == "jpg" || ext == "jpeg" || ext == "gif");
}

glm::vec3 ImageIO::screenToWorldPosition(double mouseX, double mouseY) const
{
    // Convert mouse position (pixels) to normalized device coordinates [-1, 1]
    float ndcX = static_cast<float>((mouseX / 1920.0) * 2.0 - 1.0);
    float ndcY = static_cast<float>(1.0 - (mouseY / 1080.0) * 2.0);

    glm::vec4 clipCoords(ndcX, ndcY, -1.0f, 1.0f);

    // Go from clip space to camera space (eye space)
    glm::vec4 eyeCoords
        = glm::inverse(m_camera.getProjectionMatrix()) * clipCoords;
    eyeCoords = glm::vec4(eyeCoords.x, eyeCoords.y, -1.0f, 0.0f);

    // Go from camera space to world space
    glm::mat4 invView = glm::inverse(m_camera.getViewMatrix());
    // camera position
    glm::vec3 rayOrigin = glm::vec3(invView[3]);
    glm::vec3 rayDirection = glm::normalize(glm::vec3(invView * eyeCoords));

    // Find where the ray intersects the ground plane (Z = 0)
    float t;
    if (std::abs(rayDirection.z) < 1e-5f) {
        t = 0.0f; // ray is parallel to the ground
    } else {
        t = -rayOrigin.z / rayDirection.z;
    }

    // Return the intersection point in world space
    return rayOrigin + t * rayDirection;
}

std::vector<float> ImageIO::createImageQuadVertices(
    float width, float height) const
{
    const float w = width;
    const float h = height;

    // Vertices with positions (3), texture coordinates (2), and normals (3)
    // Format: x, y, z, u, v, nx, ny, nz

    // clang-format off
    return { // Triangle 1
        -w * 0.5f, -h * 0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f,
        w * 0.5f, -h * 0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
        w * 0.5f, h * 0.5f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f,

        // Triangle 2
        w * 0.5f, h * 0.5f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
        -w * 0.5f, h * 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
        -w * 0.5f, -h * 0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f
    };
}

bool ImageIO::addImageObjectAtScreenPos(
    const std::string &path, double mouseX, double mouseY)
{
    if (!isValidImageFile(path)) {
        setStatusMessage(
            "Error: '" + path + "'\nis not a supported image file.", 5.0f,
            true);
        return false;
    }

    try {
        // Calculate position in world space
        glm::vec3 worldPos = screenToWorldPosition(mouseX, mouseY);

        // Create quad vertices
        std::vector<float> quadVertices = createImageQuadVertices();

        // Create new object
        GameObject newObject;
        newObject.rendererId
            = m_renderer->registerObject(quadVertices, {}, path, false);

        if (newObject.rendererId < 0) {
            setStatusMessage(
                "Error: Unable to load image '" + path + "'", 5.0f, true);
            return false;
        }

        newObject.setPosition(worldPos);
        m_gameObjects.push_back(newObject);

        setStatusMessage(
            "Image added: " + path.substr(path.find_last_of("/\\") + 1), 3.0f,
            false);
        return true;

    } catch (const std::exception &e) {
        setStatusMessage(
            "Error adding image: " + std::string(e.what()), 5.0f, true);
        return false;
    }
}

void ImageIO::startFrameExport(
    std::function<void(bool success, const std::string &message)> callback)
{
    if (m_exportSettings.frameCount <= 0) {
        setStatusMessage(
            "Error: Frame count must be greater than 0", 5.0f, true);
        if (callback) {
            callback(false, "Error: Frame count must be greater than 0");
        }
        return;
    }

    // Start export directly in ImageIO
    m_exportCurrentIndex = 0;
    m_exportCallback
        = [this, callback](bool success, const std::string &message) {
              setStatusMessage(message, 5.0f, !success);
              if (callback) {
                  callback(success, message);
              }
          };
    m_exportActive = true;
}

void ImageIO::updateMessageTimer(float deltaTime)
{
    if (m_statusMessage.timer > 0.0f) {
        m_statusMessage.timer -= deltaTime;
    }
}

bool ImageIO::isExportInProgress() const { return m_exportActive; }

void ImageIO::setStatusMessage(
    const std::string &message, float duration, bool isError)
{
    m_statusMessage.text = message;
    m_statusMessage.timer = duration;
    m_statusMessage.isError = isError;
}

void ImageIO::handleFrameExport(GLFWwindow *window)
{
    if (m_exportActive) {
        captureAndWriteCurrentFrame(window);
    }
}

void ImageIO::captureAndWriteCurrentFrame(GLFWwindow *window)
{
    if (!m_exportActive || !window) {
        return;
    }

    int width = 0;
    int height = 0;
    glfwGetFramebufferSize(window, &width, &height);
    if (width <= 0 || height <= 0) {
        if (m_exportCallback) {
            m_exportCallback(false, "Error: Invalid framebuffer size");
        }
        m_exportActive = false;
        return;
    }

    const int channels = 3;
    const size_t bufferSize = static_cast<size_t>(width) * height * channels;
    std::vector<unsigned char> pixels(bufferSize);

    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glReadBuffer(GL_BACK);
    glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, pixels.data());

    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        if (m_exportCallback) {
            m_exportCallback(
                false, "Error: Failed to read pixels from framebuffer");
        }
        m_exportActive = false;
        return;
    }

    // Flip vertically because OpenGL origin is bottom-left
    for (int y = 0; y < height / 2; ++y) {
        int oppositeY = height - 1 - y;
        for (int x = 0; x < width * channels; ++x) {
            std::swap(pixels[y * width * channels + x],
                pixels[oppositeY * width * channels + x]);
        }
    }

    char filename[1024];
    snprintf(filename, sizeof(filename), "%s/%s_%05d.png",
        m_exportSettings.outputDir.c_str(), m_exportSettings.baseName.c_str(),
        m_exportCurrentIndex);

    int result = stbi_write_png(
        filename, width, height, channels, pixels.data(), width * channels);

    if (!result) {
        if (m_exportCallback) {
            m_exportCallback(false, "Error: Failed to write PNG file");
        }
        m_exportActive = false;
        return;
    }

    m_exportCurrentIndex += 1;
    if (m_exportCurrentIndex >= m_exportSettings.frameCount) {
        m_exportActive = false;
        if (m_exportCallback) {
            m_exportCallback(true,
                "Frame sequence export completed: "
                    + std::to_string(m_exportSettings.frameCount)
                    + " frames saved");
        }
    }
}

void ImageIO::renderUI()
{
    ImGui::Begin("Image IO");

    ImGui::Text("Image Import");
    ImGui::Text("Drag and drop image files into the window");
    ImGui::Text("Supported formats: PNG, JPG, JPEG, GIF");

    if (m_statusMessage.timer > 0.0f) {
        if (m_statusMessage.isError) {
            ImGui::PushStyleColor(
                ImGuiCol_Text, ImVec4(1.0f, 0.3f, 0.3f, 1.0f));
        } else {
            ImGui::PushStyleColor(
                ImGuiCol_Text, ImVec4(0.3f, 1.0f, 0.3f, 1.0f));
        }
        ImGui::Text("%s", m_statusMessage.text.c_str());
        ImGui::PopStyleColor();
    }

    ImGui::Separator();

    ImGui::Text("Frame Sequence Export");

    char outputDirBuffer[512];
    char baseNameBuffer[128];
    strncpy(outputDirBuffer, m_exportSettings.outputDir.c_str(),
        sizeof(outputDirBuffer) - 1);
    outputDirBuffer[sizeof(outputDirBuffer) - 1] = '\0';
    strncpy(baseNameBuffer, m_exportSettings.baseName.c_str(),
        sizeof(baseNameBuffer) - 1);
    baseNameBuffer[sizeof(baseNameBuffer) - 1] = '\0';

    if (ImGui::InputText(
            "Output Directory", outputDirBuffer, sizeof(outputDirBuffer))) {
        m_exportSettings.outputDir = std::string(outputDirBuffer);
    }

    if (ImGui::InputText(
            "Base Name", baseNameBuffer, sizeof(baseNameBuffer))) {
        m_exportSettings.baseName = std::string(baseNameBuffer);
    }

    ImGui::InputInt("Frame Count", &m_exportSettings.frameCount);
    if (m_exportSettings.frameCount < 1) {
        m_exportSettings.frameCount = 1;
    }

    bool exportInProgress = isExportInProgress();
    if (!exportInProgress) {
        if (ImGui::Button("Start Export")) {
            startFrameExport(nullptr);
        }
    } else {
        ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.2f, 1.0f), "Exporting...");
    }

    ImGui::End();
}
