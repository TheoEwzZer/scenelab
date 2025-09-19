#include <cstdio>
#include <cstdlib>
#include <iostream>

#include <GLFW/glfw3.h>
#include <glad/gl.h>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

static constexpr int kInitialWindowWidth = 1280;
static constexpr int kInitialWindowHeight = 720;

static void glfw_error_callback(int error_code, const char *description)
{
    std::fprintf(stderr, "[GLFW ERROR %d] %s\n", error_code, description);
}

static void framebuffer_size_callback(
    GLFWwindow *windows, int width, int height)
{
    glViewport(0, 0, width, height);
}

int main()
{
    glfwSetErrorCallback(glfw_error_callback);

    if (!glfwInit()) {
        std::fprintf(stderr, "Failed to initialize GLFW.\n");
        return EXIT_FAILURE;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
#endif

    GLFWwindow *window = glfwCreateWindow(kInitialWindowWidth,
        kInitialWindowHeight, "scenelab", nullptr, nullptr);
    if (!window) {
        std::fprintf(stderr, "Failed to create GLFW window.\n");
        glfwTerminate();
        return EXIT_FAILURE;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    if (gladLoadGL(glfwGetProcAddress) == 0) {
        std::fprintf(stderr, "Failed to initialize GLAD.\n");
        glfwDestroyWindow(window);
        glfwTerminate();
        return EXIT_FAILURE;
    }

    int fbWidth = 0, fbHeight = 0;
    glfwGetFramebufferSize(window, &fbWidth, &fbHeight);
    glViewport(0, 0, fbWidth, fbHeight);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330 core");

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        }

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        static bool show_demo_window = true;
        static bool show_another_window = false;
        static ImVec4 clear_color = ImVec4(0.12f, 0.12f, 0.12f, 1.00f);

        if (show_demo_window) {
            ImGui::ShowDemoWindow(&show_demo_window);
        }

        {
            static float f = 0.0f;
            static int counter = 0;

            ImGui::Begin("Hello, scenelab!");
            ImGui::Text("This is a simple ImGui window.");
            ImGui::Checkbox("Demo Window", &show_demo_window);
            ImGui::Checkbox("Another Window", &show_another_window);

            ImGui::SliderFloat("float", &f, 0.0f, 1.0f);
            ImGui::ColorEdit3("clear color", (float *)&clear_color);

            if (ImGui::Button("Button")) {
                counter++;
            }
            ImGui::SameLine();
            ImGui::Text("counter = %d", counter);

            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
                1000.0f / io.Framerate, io.Framerate);
            ImGui::End();
        }

        if (show_another_window) {
            ImGui::Begin("Another Window", &show_another_window);
            ImGui::Text("Hello from another window!");
            if (ImGui::Button("Close Me")) {
                show_another_window = false;
            }
            ImGui::End();
        }

        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(clear_color.x * clear_color.w,
            clear_color.y * clear_color.w, clear_color.z * clear_color.w,
            clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return EXIT_SUCCESS;
}
