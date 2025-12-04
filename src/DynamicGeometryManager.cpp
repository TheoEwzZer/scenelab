//
// Created by clmonn on 11/26/25.
//

#include "../include/DynamicGeometryManager.hpp"
#include "renderer/implementation/RasterizationRenderer.hpp"

DynamicGeometryManager::DynamicGeometryManager(
    std::unique_ptr<IRenderer> &renderer) :
    m_renderer(renderer)
{
}

void DynamicGeometryManager::addCurve(std::unique_ptr<ParametricCurve> curve)
{
    m_curves.push_back(std::move(curve));
}

void DynamicGeometryManager::updateGeometry()
{
    // Only update geometry in rasterization mode
    if (!dynamic_cast<RasterizationRenderer *>(m_renderer.get())) {
        return;
    }

    for (const auto &curve : m_curves) {
        curve->updateGeometry(*m_renderer);
    }
    for (const auto &mesh : m_triangulation) {
        if (mesh->isRenderable()) {
            mesh->updateGeometry(*m_renderer);
        }
    }
}

Triangulation *DynamicGeometryManager::getLastEmpty()
{
    for (auto it = m_triangulation.rbegin(); it != m_triangulation.rend();
         ++it) {
        if (!(*it)->isRenderable()) {
            return it->get();
        }
    }

    auto tri = std::make_unique<Triangulation>();
    Triangulation *ptr = tri.get();
    m_triangulation.push_back(std::move(tri));
    return ptr;
}

void DynamicGeometryManager::invalidateRenderables()
{
    for (const auto &curve : m_curves) {
        curve->m_renderableId = -1;
    }
    for (const auto &mesh : m_triangulation) {
        mesh->registerRenderable(-1);
    }
}
