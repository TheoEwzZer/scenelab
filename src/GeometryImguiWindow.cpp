#include "GeometryImguiWindow.hpp"
#include "ImGuiFileDialog.h"
#include "imgui.h"

#include <filesystem>
#include <format>

void GeometryImguiWindow::render()
{
    ImGui::Begin("Spawn objects");

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
    ImGui::SeparatorText("Parametric curve");
    ImGui::SliderInt("Control Point", &m_nbControlPoint, 5, 10);
    if (ImGui::Button("Spawn curve") && onSpawnParametricCurve) {
        onSpawnParametricCurve(m_nbControlPoint);
        m_curveCount++;
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
