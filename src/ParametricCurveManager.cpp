//
// Created by clmonn on 11/26/25.
//

#include "../include/ParametricCurveManager.hpp"

ParametricCurveManager::ParametricCurveManager(
    ARenderer& renderer) : m_renderer(renderer)
{

}

void ParametricCurveManager::addCurve(std::unique_ptr<ParametricCurve> curve)
 {
     m_curves.push_back(std::move(curve));
 }

void ParametricCurveManager::updateGeometry()
 {
     for (const auto &curve : m_curves) {
         curve->updateGeometry(m_renderer);
     }
 }
