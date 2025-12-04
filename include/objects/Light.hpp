//
// Created by clmonn on 11/13/25.
//

#ifndef SCENELAB_LIGHT_H
#define SCENELAB_LIGHT_H

#include "GeometryGenerator.hpp"
#include "RenderableObject.hpp"
#include "glm/fwd.hpp"
#include "objects/AnalyticalSphere.hpp"

class Light : public RenderableObject {
public:
    enum Type { Directional, Point, Spot, TypeEnd };

    Light();

    void useShader(ShaderProgram &shader) const override;

    void setDirectional(const glm::vec3 &color, float intensity = 1.0f);
    void setPoint(const glm::vec3 &color, float kc = 0.5f, float kl = 0.09f, float kq = 1.0f, float intensity = 1.0f);
    void setSpot(const glm::vec3 &color, float kc = 0.5f, float kl = 0.09f, float kq = 0.03f, float p = 10.0f, float intensity = 1.0f);

    std::string getNameStr() const;

    void setUniforms(int uniformID, const ShaderProgram &lightingShader) const;

    void draw([[maybe_unused]] const ShaderProgram &vectorial,
        [[maybe_unused]] const ShaderProgram &pointLight,
        const ShaderProgram &lighting,
        const TextureLibrary &textures) const override;

    void setIntensity(float intensity);

    float getIntensity() const { return m_intensity; }

    GData &getGData();

    Type getType() const;

protected:
    void init();
    void updateEmissive(); // Sync emissive with color * intensity

    glm::vec3 m_color = {1.0,1.0,1.0};
    glm::vec3 m_direction = {-1.0,-1.0,-1.0};
    float m_intensity = 1.0f;
    float m_kc;
    float m_kl;
    float m_kq;
    float m_p;

    Type m_type = Directional;
    GData m_gdata;
};

#endif // SCENELAB_LIGHT_H
