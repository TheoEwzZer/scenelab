#include "GeometryImguiWindow.hpp"
#include "ImGuiFileDialog.h"
#include "imgui.h"

#include <filesystem>
#include <format>

void GeometryImguiWindow::render(bool *p_open)
{
    if (p_open && !*p_open) {
        return;
    }

    if (!ImGui::Begin("Geometry", p_open)) {
        ImGui::End();
        return;
    }

    ImGui::Text("Primitives");

    ImGui::SeparatorText("Sphere");
    ImGui::SliderFloat("Radius", &m_sphereRadius, 0.1f, 2.0f);
    ImGui::SliderInt("Sectors (vertical slices)", &m_sphereSectors, 3, 64);
    ImGui::SliderInt("Stacks (horizontal slices)", &m_sphereStacks, 2, 32);
    if (ImGui::Button("Spawn Sphere") && onSpawnSphere) {
        onSpawnSphere(m_sphereRadius, m_sphereSectors, m_sphereStacks);
        m_sphereCount++;
    }

    ImGui::Spacing();
    ImGui::SeparatorText("Cube");
    ImGui::SliderFloat("Cube Size", &m_cubeSize, 0.1f, 2.0f);
    if (ImGui::Button("Spawn Cube") && onSpawnCube) {
        onSpawnCube(m_cubeSize);
        m_cubeCount++;
    }

    ImGui::Spacing();
    ImGui::SeparatorText("Cylinder");
    ImGui::SliderFloat("Cylinder Radius", &m_cylinderRadius, 0.1f, 2.0f);
    ImGui::SliderFloat("Height", &m_cylinderHeight, 0.1f, 5.0f);
    ImGui::SliderInt("Cylinder Sectors", &m_cylinderSectors, 3, 64);
    if (ImGui::Button("Spawn Cylinder") && onSpawnCylinder) {
        onSpawnCylinder(m_cylinderRadius, m_cylinderHeight, m_cylinderSectors);
        m_cylinderCount++;
    }

    ImGui::Spacing();
    ImGui::SeparatorText("Plane");
    ImGui::SliderFloat("Plane Width", &m_planeWidth, 0.5f, 20.0f);
    ImGui::SliderFloat("Plane Height", &m_planeHeight, 0.5f, 20.0f);
    ImGui::InputFloat3("Normal", m_planeNormal);
    if (ImGui::Button("Spawn Plane") && onSpawnPlane) {
        glm::vec3 normal(m_planeNormal[0], m_planeNormal[1], m_planeNormal[2]);
        if (glm::length(normal) < 0.001f) {
            normal = glm::vec3(0.0f, 1.0f, 0.0f);
        }
        onSpawnPlane(m_planeWidth, m_planeHeight, glm::normalize(normal));
        m_planeCount++;
    }

    ImGui::Spacing();
    ImGui::SeparatorText("Parametric curve");
    ImGui::SliderInt("Control Point", &m_nbControlPoint, 5, 10);
    if (ImGui::Button("Spawn curve") && onSpawnParametricCurve) {
        onSpawnParametricCurve(m_nbControlPoint);
        m_curveCount++;
    }

    ImGui::Spacing();
    ImGui::SeparatorText("Triangles mesh");
    if (ImGui::Button("Add point") && onAddPoint) {
        onAddPoint();
        m_sphereCount++;
    }
    if (ImGui::Button("Generate triangulation") && onGenerateMesh) {
        onGenerateMesh();
        m_meshCount++;
    }

    ImGui::NewLine();
    ImGui::SeparatorText("3D obj");
    if (ImGui::Button("Load Model")) {
        IGFD::FileDialogConfig config;
        config.path = "../assets/objects";
        ImGuiFileDialog::Instance()->OpenDialog(
            "ChooseFileDlgKey", "Choose File", ".obj", config);
    }
    // display
    if (ImGuiFileDialog::Instance()->Display("ChooseFileDlgKey",
            ImGuiWindowFlags_NoCollapse, ImVec2(100, 100))) {
        if (ImGuiFileDialog::Instance()->IsOk()) { // action if OK
            std::string fullpath
                = ImGuiFileDialog::Instance()->GetFilePathName();
            std::filesystem::path p(fullpath);
            std::string filename = p.stem().string();

            if (onLoadModel) {
                onLoadModel(filename, fullpath);
            }
        }

        // close
        ImGuiFileDialog::Instance()->Close();
    }

    const auto &models = m_modelLibrary.getModels();
    if (!models.empty()) {
        ImGui::NewLine();

        ImGui::Text("Loaded models:");

        std::vector<std::string> modelsToRemove;

        if (ImGui::BeginListBox("##loaded_models_list",
                ImVec2(0, 5 * ImGui::GetTextLineHeightWithSpacing()))) {
            for (const auto &[filepath, entry] : models) {
                ImGui::PushID(filepath.c_str());

                if (ImGui::Button(
                        std::format("Spawn {}", entry.name).c_str())) {
                    if (onSpawnModelInstance) {
                        onSpawnModelInstance(entry.name, filepath);
                    }
                }

                ImGui::SameLine();

                if (ImGui::Button("x")) {
                    modelsToRemove.push_back(filepath);
                }

                ImGui::PopID();
            }
            ImGui::EndListBox();
        }

        for (const auto &filepath : modelsToRemove) {
            m_modelLibrary.removeModel(filepath);
        }
    }

    ImGui::End();
}
