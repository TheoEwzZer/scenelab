//
// Created by clmonn on 11/13/25.
//

#ifndef SCENELAB_LIGHT_H
#define SCENELAB_LIGHT_H

#include "RenderableObject.hpp"
#include "glm/fwd.hpp"

class Light : public RenderableObject {
    public:
    enum Type {Directional, Point, Spot, TypeEnd};

    Light(const std::vector<float> &vertices,
        const std::vector<unsigned int> &indices, int textureHandle = -1);
    Light(const std::vector<float> &vertices,
        const std::vector<unsigned int> &indices, const glm::vec3 &color);

    void useShader(ShaderProgram &shader) const override;

    void setDirectional(const glm::vec3 &color);
    void setPoint(const glm::vec3 &color, float ke, float kl, float kq);
    void setSpot(const glm::vec3 &color, float ke, float kl, float kq, float p);

    Type getType(void) const {return m_type;};
    std::string getNameStr() const;

    void setUniforms(int uniformID, const ShaderProgram &lightingShader) const;

    void draw([[maybe_unused]] const ShaderProgram &vectorial,
        [[maybe_unused]] const ShaderProgram &pointLight,
        const ShaderProgram &lighting,
        const TextureLibrary &textures) const override;

protected:
    void init(const std::vector<float> &vertices,
        const std::vector<unsigned int> &indices);

    glm::vec3 m_color = {1.0,1.0,1.0};
    glm::vec3 m_direction = {-1.0,-1.0,-1.0};
    float m_kc;
    float m_kl;
    float m_kq;
    float m_p;

    Type m_type = Directional;
};

#endif // SCENELAB_LIGHT_H
