#include "GameObject.hpp"
#include <cstring>
#include <glm/gtc/matrix_transform.hpp>

void GameObject::setPosition(const glm::vec3 &pos)
{
    if (m_position != pos) {
        m_position = pos;
        m_transformDirty = true;
    }
}

void GameObject::setRotation(const glm::vec3 &rot)
{
    if (m_rotation != rot) {
        m_rotation = rot;
        m_transformDirty = true;
    }
}

void GameObject::setScale(const glm::vec3 &scale)
{
    if (m_scale != scale) {
        m_scale = scale;
        m_transformDirty = true;
    }
}

void GameObject::setName(const std::string &name)
{
    std::strncpy(m_name, name.c_str(), OBJ_MAX_NAME_SIZE);
}

const glm::mat4 &GameObject::getLocalMatrix() const
{
    if (m_transformDirty) {
        m_localMatrix = glm::mat4(1.0f);
        m_localMatrix = glm::translate(m_localMatrix, m_position);

        glm::mat4 rotationMatrix = glm::mat4(1.0f);
        rotationMatrix
            = glm::rotate(rotationMatrix, m_rotation.z, glm::vec3(0, 0, 1));
        rotationMatrix
            = glm::rotate(rotationMatrix, m_rotation.y, glm::vec3(0, 1, 0));
        rotationMatrix
            = glm::rotate(rotationMatrix, m_rotation.x, glm::vec3(1, 0, 0));
        m_localMatrix = m_localMatrix * rotationMatrix;

        m_localMatrix = glm::scale(m_localMatrix, m_scale);
        m_transformDirty = false;
    }
    return m_localMatrix;
}

const glm::mat4 &GameObject::getWorldMatrix(
    const glm::mat4 &parentMatrix) const
{
    // Compute world matrix as parent * local
    m_worldMatrix = parentMatrix * getLocalMatrix();
    return m_worldMatrix;
}

void GameObject::setAABB(const glm::vec3 &corner1, const glm::vec3 &corner2)
{
    m_aabbCorner1 = corner1;
    m_aabbCorner2 = corner2;
}