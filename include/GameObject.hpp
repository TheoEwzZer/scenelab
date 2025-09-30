#pragma once
#include <glm/glm.hpp>

class GameObject {
protected:
    glm::vec3 m_position { 0.0f }, m_rotation { 0.0f }, m_scale { 1.0f };
    mutable glm::mat4 m_modelMatrix { 1.0f };
    mutable bool m_transformDirty = true;

public:
    int rendererId = -1;

    void setPosition(const glm::vec3 &pos);
    void setRotation(const glm::vec3 &rot);
    void setScale(const glm::vec3 &scale);

    const glm::vec3 &getPosition() const { return m_position; }

    const glm::vec3 &getRotation() const { return m_rotation; }

    const glm::vec3 &getScale() const { return m_scale; }

    bool hasTransformChanged() const { return m_transformDirty; }

    const glm::mat4 &getModelMatrix() const;
};