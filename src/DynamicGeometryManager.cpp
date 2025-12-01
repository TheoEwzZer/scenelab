//
// Created by clmonn on 11/26/25.
//

#include "../include/DynamicGeometryManager.hpp"

DynamicGeometryManager::DynamicGeometryManager(IRenderer &renderer) :
    m_renderer(renderer)
{
}

void DynamicGeometryManager::addCurve(std::unique_ptr<ParametricCurve> curve)
{
    m_curves.push_back(std::move(curve));
}

void DynamicGeometryManager::updateGeometry()
{
    for (const auto &curve : m_curves) {
        curve->updateGeometry(m_renderer);
    }
    for (const auto &mesh : m_triangulation) {
        if (mesh->isRenderable()) {
            mesh->updateGeometry(m_renderer);
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
