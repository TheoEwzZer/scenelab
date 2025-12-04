#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;
layout (location = 2) in vec3 aNormal;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in vec3 aBitangent;

out vec2 TexCoord;
out vec3 Normal;
out vec3 FragPos;
out mat3 TBN;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    mat3 model3 = mat3(model);
    mat3 normalMatrix = transpose(inverse(model3));
    // Transform tangent/bitangent with model (like positions), normal with normalMatrix
    vec3 T = normalize(model3 * aTangent);
    vec3 B = normalize(model3 * aBitangent);
    vec3 N = normalize(normalMatrix * aNormal);
    // Orthonormalise tangent space (Gram–Schmidt)
    T = normalize(T - N * dot(N, T));
    B = cross(N, T);

    TBN = mat3(T, B, N);

    gl_Position = projection * view * model * vec4(aPos, 1.0);
    FragPos = vec3(model * vec4(aPos, 1.0));
    TexCoord = aTexCoord;

// pass the transformed normal to fragment shader
    Normal = N;
}
