#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord_as_rg;
layout (location = 2) in vec3 aNormal_as_ba;

out vec3 FragPos;
out vec4 c2DColor;
out vec2 TexCoord;
out vec3 Normal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    gl_Position = projection * view * model * vec4(aPos, 1.0);
    FragPos = vec3(model * vec4(aPos, 1.0));

    c2DColor = vec4(aTexCoord_as_rg.x, aTexCoord_as_rg.y, aNormal_as_ba.x, aNormal_as_ba.y);
    TexCoord = vec2(0,0);
    Normal = vec3(0,0,1);
}
