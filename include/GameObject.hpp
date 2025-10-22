#pragma once
#include <glm/glm.hpp>
#include <string>
#include <vector>

class GameObject {
protected:
    glm::vec3 m_position { 0.0f }, m_rotation { 0.0f }, m_scale { 1.0f };
    mutable glm::mat4 m_localMatrix { 1.0f };
    mutable glm::mat4 m_worldMatrix { 1.0f };
    static const auto OBJ_MAX_NAME_SIZE = 25;
    mutable bool m_transformDirty = true;
    
    glm::vec3 m_aabbCorner1 { 0.0f };
    glm::vec3 m_aabbCorner2 { 0.0f };
    bool m_isBoundingBoxActive { false };
    
public:
    mutable glm::mat4 m_localMatrix { 1.0f };
    mutable glm::mat4 m_worldMatrix { 1.0f };
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

    const glm::mat4 &getLocalMatrix() const;
    const glm::mat4 &getWorldMatrix(const glm::mat4 &parentMatrix = glm::mat4(1.0f)) const;

    // Alias for backward compatibility
    const glm::mat4 &getModelMatrix() const { return getLocalMatrix(); }

    std::vector<float> getVertices()
    {
        return (std::vector<float>(8, 0.0f));
    };

    const glm::mat4 &getLocalMatrix() const;
    const glm::mat4 &getWorldMatrix(const glm::mat4 &parentMatrix = glm::mat4(1.0f)) const;
    const glm::mat4 &getModelMatrix() const { return getLocalMatrix(); }

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
