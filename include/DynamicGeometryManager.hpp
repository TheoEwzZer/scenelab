//
// Created by clmonn on 11/26/25.
//

#ifndef SCENELAB_PARAMETRICCURVEMANAGER_H
#define SCENELAB_PARAMETRICCURVEMANAGER_H

#include "shapes/ParametricCurve.hpp"
#include "shapes/Triangulation.hpp"

#include <memory>

class DynamicGeometryManager {
public:
    explicit DynamicGeometryManager(std::unique_ptr<IRenderer> &renderer);

    Triangulation *getLastEmpty();
    void addCurve(std::unique_ptr<ParametricCurve> curve);
    void addMesh(std::unique_ptr<Triangulation> mesh);
    void updateGeometry();
    void invalidateRenderables();

private:
    std::unique_ptr<IRenderer> &m_renderer;
    std::vector<std::unique_ptr<ParametricCurve>> m_curves;
    std::vector<std::unique_ptr<Triangulation>> m_triangulation;
};

#endif // SCENELAB_PARAMETRICCURVEMANAGER_H
