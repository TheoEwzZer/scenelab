//
// Created by clmonn on 11/25/25.
//

#include "../include/ParametricCurve.hpp"

void ParametricCurve::addControlPoint(SceneGraph::Node *node)
{
    m_controlPoints.push_back(node);
}

void ParametricCurve::registerCurve(const int id)
{
    m_renderableId = id;
}

float ParametricCurve::tj(
    const glm::vec3 &p0, const glm::vec3 &p1, float alpha)
{
    return pow(glm::length(p1 - p0), alpha);
}

glm::vec3 ParametricCurve::catmullRomSample(const glm::vec3 &P0,
    const glm::vec3 &P1, const glm::vec3 &P2, const glm::vec3 &P3, float t)
{
    const float alpha = 0.5f;
    const float t0 = 0.0f;
    const float t1 = t0 + tj(P0, P1, alpha);
    const float t2 = t1 + tj(P1, P2, alpha);
    const float t3 = t2 + tj(P2, P3, alpha);

    const float tt = t1 + t * (t2 - t1);

    auto interp = [](const glm::vec3 &A, const glm::vec3 &B, float ta, float tb, float t) {
        if (fabs(tb - ta) < 1e-6f) return A;
        float l = (t - ta) / (tb - ta);
        return A + l * (B - A);
    };

    const glm::vec3 A1 = interp(P0, P1, t0, t1, tt);
    const glm::vec3 A2 = interp(P1, P2, t1, t2, tt);
    const glm::vec3 A3 = interp(P2, P3, t2, t3, tt);

    const glm::vec3 B1 = interp(A1, A2, t0, t2, tt);
    const glm::vec3 B2 = interp(A2, A3, t1, t3, tt);

    const glm::vec3 C = interp(B1, B2, t1, t2, tt);
    return C;
}

std::vector<float> ParametricCurve::buildCatmullRomPoints(
    const std::vector<glm::vec3> &controls, const int samplesPerSegment)
{
    std::vector<float> out;
    const int n = controls.size();
    if (n < 2) return out;

    // For endpoints, mirror extrapolation (common choice) so curve touches first and last point.
    // Use virtual points: P_-1 = P0 + (P0 - P1) and P_{n} = P_{n-1} + (P_{n-1} - P_{n-2})
    auto get = [&](const int i)->glm::vec3 {
        if (i < 0) return controls[0] + (controls[0] - controls[1]);
        if (i >= n) return controls[n-1] + (controls[n-1] - controls[n-2]);
        return controls[i];
    };

    for (int i = 0; i < n - 1; ++i) {
        // segment between P_i and P_{i+1} uses P_{i-1},P_i,P_{i+1},P_{i+2}
        glm::vec3 P0 = get(i - 1);
        glm::vec3 P1 = get(i);
        glm::vec3 P2 = get(i + 1);
        glm::vec3 P3 = get(i + 2);

        // sample from t=0 to t=1 (inclusive at end only for last segment)
        const int steps = samplesPerSegment;
        for (int s = 0; s < steps; ++s) {
            const float t = static_cast<float>(s) / static_cast<float>(steps);
            glm::vec3 sample = catmullRomSample(P0, P1, P2, P3, t);
            out.push_back(sample.x);
            out.push_back(sample.y);
            out.push_back(sample.z);
        }
    }
    // include final control point
    out.push_back(controls.back().x);
    out.push_back(controls.back().y);
    out.push_back(controls.back().z);
    return out;
}


void ParametricCurve::updateGeometry(ARenderer &renderer)
{
    std::vector<glm::vec3> controls;

    for (auto *point : m_controlPoints) {
        if (point) {
            auto& obj = point->getData();
            controls.push_back(obj.getPosition());
        }
    }
    renderer.updateGeometry(m_renderableId, buildCatmullRomPoints(controls));
}


