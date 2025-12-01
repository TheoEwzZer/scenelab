#ifndef GEOMETRYIMGUIWINDOW_H
#define GEOMETRYIMGUIWINDOW_H

#include "ModelLibrary.hpp"

#include <cstddef>
#include <functional>
#include <glm/glm.hpp>
#include <string>

struct MaterialProperties {
    glm::vec3 color = glm::vec3(1.0f);
    glm::vec3 emissive = glm::vec3(0.0f);
    float percentSpecular = 0.0f;
    float roughness = 0.5f;
    glm::vec3 specularColor = glm::vec3(1.0f);
};

class GeometryImguiWindow {
public:
    void render(bool *p_open = nullptr);

    std::function<void(float radius, int sectors, int stacks)> onSpawnSphere;
    std::function<void(float size)> onSpawnCube;
    std::function<void(float radius, float height, int sectors)>
        onSpawnCylinder;
    std::function<void(int controlPoints)> onSpawnParametricCurve;
    std::function<void()> onAddPoint;
    std::function<void()> onGenerateMesh;
    std::function<void(float width, float height, const glm::vec3 &normal)>
        onSpawnPlane;
    std::function<void(const std::string &objName, const std::string &objPath)>
        onLoadModel;
    std::function<void(const std::string &name, const std::string &filepath)>
        onSpawnModelInstance;

    std::size_t m_sphereCount { 0 };
    std::size_t m_cubeCount { 0 };
    std::size_t m_cylinderCount { 0 };
    std::size_t m_curveCount { 0 };
    std::size_t m_meshCount { 0 };
    std::size_t m_planeCount { 0 };

    ModelLibrary m_modelLibrary;

private:
    float m_sphereRadius { 0.5f };
    int m_sphereSectors { 36 };
    int m_sphereStacks { 18 };

    float m_cubeSize { 1.0f };

    float m_cylinderRadius { 0.5f };
    float m_cylinderHeight { 1.0f };
    int m_cylinderSectors { 36 };

    int m_nbControlPoint { 5 };

    float m_planeWidth { 5.0f };
    float m_planeHeight { 5.0f };
    float m_planeNormal[3] { 0.0f, 1.0f, 0.0f };
};

#endif /* GEOMETRYIMGUIWINDOW_H */
