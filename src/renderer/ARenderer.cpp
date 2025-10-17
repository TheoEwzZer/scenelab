#include <cstdlib>
#include <functional>
#include <glad/gl.h>
#include <unordered_map>
#include <stdlib.h>
#include <stdio.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include "renderer/interface/ARenderer.hpp"
#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>

// Static pointer to store current renderer instance for callback
static ARenderer *s_currentRenderer = nullptr;

// Callback map
std::unordered_map<int,
    std::unordered_map<int, std::vector<std::function<void()>>>>
    keyCallbacks = {};
std::function<void(double, double)> cursorCallback = NULL;
static std::function<void(const std::vector<std::string> &, double, double)>
    dropCallback;

static void error_callback(int error, const char *description)
{
    (void)error;
    fprintf(stderr, "Error: %s\n", description);
}

static void key_callback(
    GLFWwindow *window, int key, int scancode, int action, int mods)
{
    (void)window;
    (void)scancode;
    (void)mods;

    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }

    for (const auto &callback : keyCallbacks[key][action]) {
        callback();
    }
}

static void mouse_callback(
    GLFWwindow *window, int button, int action, int mods)
{
    (void)window;
    (void)mods;

    for (const auto &callback : keyCallbacks[button][action]) {
        callback();
    }
}

void ARenderer::addKeyCallback(
    int key, int action, std::function<void()> callback)
{
    keyCallbacks[key][action].push_back(callback);
}

static void cursor_callback(GLFWwindow *window, double x, double y)
{
    (void)window;

    if (cursorCallback) {
        cursorCallback(x, y);
    }
}

void ARenderer::addCursorCallback(std::function<void(double, double)> callback)
{
    cursorCallback = callback;
    glfwSetCursorPosCallback(m_window, cursor_callback);
}

static void drop_callback(GLFWwindow *window, int count, const char **paths)
{
    double mx = 0.0, my = 0.0;
    if (window) {
        glfwGetCursorPos(window, &mx, &my);
    }
    if (!dropCallback) {
        return;
    }
    std::vector<std::string> dropped;
    dropped.reserve(static_cast<size_t>(count));
    for (int i = 0; i < count; ++i) {
        dropped.emplace_back(paths[i] ? paths[i] : "");
    }
    dropCallback(dropped, mx, my);
}

void ARenderer::addDropCallback(std::function<void(
        const std::vector<std::string> &paths, double mouseX, double mouseY)>
        callback)
{
    dropCallback = std::move(callback);
    glfwSetDropCallback(m_window, drop_callback);
}

ARenderer::ARenderer()
{
    // Set the current renderer instance for callbacks
    s_currentRenderer = this;

    // Window Creation

    glfwSetErrorCallback(error_callback);

    if (!glfwInit()) {
        exit(EXIT_FAILURE);
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    m_window = glfwCreateWindow(1920, 1080, "Simple example", NULL, NULL);
    if (!m_window) {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwSetKeyCallback(m_window, key_callback);
    glfwSetMouseButtonCallback(m_window, mouse_callback);

    glfwMakeContextCurrent(m_window);
    gladLoadGL(glfwGetProcAddress);
    glfwSwapInterval(1);
}

void ARenderer::init()
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(m_window, true);
    ImGui_ImplOpenGL3_Init("#version 330");
}

ARenderer::~ARenderer()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(m_window);

    glfwTerminate();
    exit(EXIT_SUCCESS);
}

bool ARenderer::shouldWindowClose() { return glfwWindowShouldClose(m_window); }
