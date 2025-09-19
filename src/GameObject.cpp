#include "GameObject.hpp"
#include <glm/gtc/matrix_transform.hpp>

void GameObject::setPosition(const glm::vec3 &pos) {
    if (m_position != pos) {
        m_position = pos;
        m_transformDirty = true;
    }
}

void GameObject::setRotation(const glm::vec3& rot) {
    if (m_rotation != rot) {
        m_rotation = rot;
        m_transformDirty = true;
    }
}

void GameObject::setScale(const glm::vec3& scale) {
    if (m_scale != scale) {
        m_scale = scale;
        m_transformDirty = true;
    }
}

const glm::mat4& GameObject::getModelMatrix() const {
    if (m_transformDirty) {
        m_modelMatrix = glm::mat4(1.0f);
        m_modelMatrix = glm::translate(m_modelMatrix, m_position);

        glm::mat4 rotationMatrix = glm::mat4(1.0f);
        rotationMatrix = glm::rotate(rotationMatrix, m_rotation.z, glm::vec3(0, 0, 1));
        rotationMatrix = glm::rotate(rotationMatrix, m_rotation.y, glm::vec3(0, 1, 0));
        rotationMatrix = glm::rotate(rotationMatrix, m_rotation.x, glm::vec3(1, 0, 0));
        m_modelMatrix = m_modelMatrix * rotationMatrix;

        m_modelMatrix = glm::scale(m_modelMatrix, m_scale);
        m_transformDirty = false;
    }
    return m_modelMatrix;
}