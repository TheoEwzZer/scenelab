#version 330 core
layout(location = 0) out vec3 gPosition;
layout(location = 1) out vec3 gNormal;
layout(location = 2) out vec4 gAlbedoSpec;
layout(location = 3) out vec4 gMetallicRoughness;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

// Material properties
uniform vec3 albedo;
uniform float metallic;
uniform float roughness;
uniform float ao;

// Texture flags and samplers
uniform bool hasAlbedoMap;
uniform sampler2D albedoMap;
uniform bool hasNormalMap;
uniform sampler2D normalMap;
uniform bool hasMetallicMap;
uniform sampler2D metallicMap;
uniform bool hasRoughnessMap;
uniform sampler2D roughnessMap;

vec3 getNormalFromMap()
{
    if (!hasNormalMap) {
        return normalize(Normal);
    }

    vec3 tangentNormal = texture(normalMap, TexCoords).xyz * 2.0 - 1.0;

    vec3 Q1 = dFdx(FragPos);
    vec3 Q2 = dFdy(FragPos);
    vec2 st1 = dFdx(TexCoords);
    vec2 st2 = dFdy(TexCoords);

    vec3 N = normalize(Normal);
    vec3 T = normalize(Q1 * st2.t - Q2 * st1.t);
    vec3 B = -normalize(cross(N, T));
    mat3 TBN = mat3(T, B, N);

    return normalize(TBN * tangentNormal);
}

void main()
{
    // Store position in world space
    gPosition = FragPos;

    // Store normal (with normal mapping if available)
    gNormal = getNormalFromMap();

    // Store albedo color
    vec3 finalAlbedo = albedo;
    if (hasAlbedoMap) {
        finalAlbedo = texture(albedoMap, TexCoords).rgb;
    }
    gAlbedoSpec.rgb = finalAlbedo;
    gAlbedoSpec.a = ao; // Store AO in alpha channel

    // Store metallic and roughness
    float finalMetallic = metallic;
    float finalRoughness = roughness;
    if (hasMetallicMap) {
        finalMetallic = texture(metallicMap, TexCoords).r;
    }
    if (hasRoughnessMap) {
        finalRoughness = texture(roughnessMap, TexCoords).r;
    }
    gMetallicRoughness = vec4(finalMetallic, finalRoughness, 0.0, 1.0);
}
