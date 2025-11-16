#ifndef GEOMETRYIMGUIWINDOW_H
#define GEOMETRYIMGUIWINDOW_H

#include "ModelLibrary.hpp"

#include <cstddef>
#include <functional>
#include <glm/glm.hpp>
#include <string>

class GeometryImguiWindow {
public:
    void render();

    std::function<void(float radius, int sectors, int stacks)> onSpawnSphere;
    std::function<void(float size)> onSpawnCube;
    std::function<void(float radius, float height, int sectors)>
        onSpawnCylinder;
    std::function<void(const std::string &objName, const std::string &objPath)>
        onLoadModel;
    std::function<void(const std::string &name, const std::string &filepath)>
        onSpawnModelInstance;

    std::size_t m_sphereCount { 0 };
    std::size_t m_cubeCount { 0 };
    std::size_t m_cylinderCount { 0 };

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
};

#endif /* GEOMETRYIMGUIWINDOW_H */
