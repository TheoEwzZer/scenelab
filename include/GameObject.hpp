#pragma once
#include <glm/glm.hpp>
#include <string>

class GameObject {
private:
    glm::vec3 m_position { 0.0f }, m_rotation { 0.0f }, m_scale { 1.0f };
    static const auto OBJ_MAX_NAME_SIZE = 25;
    mutable glm::mat4 m_modelMatrix { 1.0f };
    mutable bool m_transformDirty = true;

    glm::vec3 m_aabbCorner1 { 0.0f };
    glm::vec3 m_aabbCorner2 { 0.0f };
    bool m_isBoundingBoxActive { false };

public:
    int rendererId = -1;
    char m_name[OBJ_MAX_NAME_SIZE + 1] = { "Object" };

    void setPosition(const glm::vec3 &pos);
    void setRotation(const glm::vec3 &rot);
    void setScale(const glm::vec3 &scale);

    void setName(const std::string &name);

    const glm::vec3 &getPosition() const { return m_position; }

    const glm::vec3 &getRotation() const { return m_rotation; }

    const glm::vec3 &getScale() const { return m_scale; }

    bool hasTransformChanged() const { return m_transformDirty; }

    const glm::mat4 &getModelMatrix() const;

    void setAABB(const glm::vec3 &corner1, const glm::vec3 &corner2);

    [[nodiscard]] const glm::vec3 &getAABBCorner1() const
    {
        return m_aabbCorner1;
    }

    [[nodiscard]] const glm::vec3 &getAABBCorner2() const
    {
        return m_aabbCorner2;
    }

    void setBoundingBoxActive(bool active) { m_isBoundingBoxActive = active; }

    [[nodiscard]] bool isBoundingBoxActive() const
    {
        return m_isBoundingBoxActive;
    }
};
