//
// Created by clmonn on 11/25/25.
//

#ifndef SCENELAB_PARAMETRICCURVE_H
#define SCENELAB_PARAMETRICCURVE_H

#include "../SceneGraph.hpp"
#include "../renderer/interface/IRenderer.hpp"

class ParametricCurve {
public:
    std::vector<SceneGraph::Node *> m_controlPoints;
    int m_renderableId = -1;

    void addControlPoint(SceneGraph::Node *node);
    void registerCurve(int id);
    void updateGeometry(IRenderer &renderer);

private:
    static float tj(
        const glm::vec3 &p0, const glm::vec3 &p1, float alpha = 0.5f);
    glm::vec3 catmullRomSample(const glm::vec3 &P0, const glm::vec3 &P1,
        const glm::vec3 &P2, const glm::vec3 &P3, float t);
    std::vector<float> buildCatmullRomPoints(
        const std::vector<glm::vec3> &controls, int samplesPerSegment = 16);
};

#endif // SCENELAB_PARAMETRICCURVE_H
