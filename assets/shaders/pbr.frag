#version 330 core
out vec4 FragColor;

in vec2 TexCoord;
in vec3 WorldPos;
in vec3 Normal;

// Material parameters
struct PBRMaterial {
    vec3 albedo;
    vec3 emissive;
    float metallic;
    float roughness;
    float ao;
};

// Light structures (same as lighting.frag for compatibility)
#define MAX_POINT_LIGHTS 16
#define MAX_SPOT_LIGHTS 16
#define MAX_DIRECTIONAL_LIGHTS 16

struct PointLight {
    vec3 position;
    vec3 color;
    float ke;
    float kl;
    float kq;
};

struct DirectionalLight {
    vec3 color;
    vec3 direction;
};

struct SpotLight {
    vec3 position;
    vec3 color;
    vec3 direction;
    float ke;
    float kl;
    float kq;
    float p;
};

// Uniforms
uniform sampler2D ourTexture;
uniform bool useTexture;

uniform PBRMaterial material;
uniform vec3 viewPosition;
uniform vec3 ambientLightColor;

uniform int NB_DIR_LIGHTS;
uniform int NB_POINT_LIGHTS;
uniform int NB_SPOT_LIGHTS;
uniform PointLight pointLights[MAX_POINT_LIGHTS];
uniform DirectionalLight directionalLights[MAX_DIRECTIONAL_LIGHTS];
uniform SpotLight spotLights[MAX_SPOT_LIGHTS];

// IBL textures (will be used in Phase 2)
uniform samplerCube irradianceMap;
uniform samplerCube prefilterMap;
uniform sampler2D brdfLUT;
uniform bool useIBL;

// Tone mapping
uniform int toneMappingMode;
uniform float toneExposure;

const float PI = 3.14159265359;

// Normal Distribution Function - GGX/Trowbridge-Reitz
float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float nom = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / max(denom, 0.0001);
}

// Geometry function - Schlick-GGX
float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;

    float nom = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / max(denom, 0.0001);
}

// Geometry function - Smith
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

// Fresnel equation - Schlick approximation
vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

// Fresnel with roughness for IBL
vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
    return F0
        + (max(vec3(1.0 - roughness), F0) - F0)
        * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

// Calculate PBR lighting for a single light
vec3 calculatePBRLight(vec3 N, vec3 V, vec3 L, vec3 radiance, vec3 albedo,
    float metallic, float roughness)
{
    vec3 H = normalize(V + L);

    // Calculate base reflectivity (F0)
    // Dielectrics: 0.04, Metals: albedo color
    vec3 F0 = vec3(0.04);
    F0 = mix(F0, albedo, metallic);

    // Cook-Torrance BRDF
    float NDF = DistributionGGX(N, H, roughness);
    float G = GeometrySmith(N, V, L, roughness);
    vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);

    // Specular contribution
    vec3 numerator = NDF * G * F;
    float denominator
        = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
    vec3 specular = numerator / denominator;

    // Energy conservation
    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - metallic; // Metals have no diffuse

    float NdotL = max(dot(N, L), 0.0);

    return (kD * albedo / PI + specular) * radiance * NdotL;
}

vec3 applyToneMapping(vec3 color)
{
    vec3 mapped = color * toneExposure;

    if (toneMappingMode == 1) { // Reinhard
        mapped = mapped / (mapped + vec3(1.0));
        mapped = pow(mapped, vec3(1.0 / 2.2));
    } else if (toneMappingMode == 2) { // ACES
        const float A = 2.51;
        const float B = 0.03;
        const float C = 2.43;
        const float D = 0.59;
        const float E = 0.14;
        mapped = clamp(
            (mapped * (A * mapped + B)) / (mapped * (C * mapped + D) + E), 0.0,
            1.0);
        mapped = pow(mapped, vec3(1.0 / 2.2));
    }

    return mapped;
}

void main()
{
    vec3 N = normalize(Normal);
    vec3 V = normalize(viewPosition - WorldPos);

    // Get albedo from texture or material
    vec3 albedo
        = useTexture ? texture(ourTexture, TexCoord).rgb : material.albedo;
    // Convert from sRGB to linear space if needed
    albedo = pow(albedo, vec3(2.2));

    float metallic = material.metallic;
    float roughness = material.roughness;
    float ao = material.ao;

    // Calculate reflectance at normal incidence
    vec3 F0 = vec3(0.04);
    F0 = mix(F0, albedo, metallic);

    // Accumulate lighting
    vec3 Lo = vec3(0.0);

    // Directional lights
    for (int i = 0; i < NB_DIR_LIGHTS; i++) {
        vec3 L = normalize(-directionalLights[i].direction);
        vec3 radiance = directionalLights[i].color;
        Lo += calculatePBRLight(
            N, V, L, radiance, albedo, metallic, roughness);
    }

    // Point lights
    for (int i = 0; i < NB_POINT_LIGHTS; i++) {
        vec3 lightVec = pointLights[i].position - WorldPos;
        float distance = length(lightVec);
        vec3 L = normalize(lightVec);

        float attenuation = 1.0
            / (pointLights[i].ke + pointLights[i].kl * distance
                + pointLights[i].kq * distance * distance);
        vec3 radiance = pointLights[i].color * attenuation;

        Lo += calculatePBRLight(
            N, V, L, radiance, albedo, metallic, roughness);
    }

    // Spot lights
    for (int i = 0; i < NB_SPOT_LIGHTS; i++) {
        vec3 lightVec = spotLights[i].position - WorldPos;
        float distance = length(lightVec);
        vec3 L = normalize(lightVec);

        float attenuation = 1.0
            / (spotLights[i].ke + spotLights[i].kl * distance
                + spotLights[i].kq * distance * distance);

        vec3 spotDir = normalize(spotLights[i].direction);
        float intensity = pow(max(dot(-L, spotDir), 0.0), spotLights[i].p);

        vec3 radiance = spotLights[i].color * attenuation * intensity;

        Lo += calculatePBRLight(
            N, V, L, radiance, albedo, metallic, roughness);
    }

    // Ambient lighting (simple for now, IBL in Phase 2)
    vec3 ambient;
    if (useIBL) {
        // IBL ambient (Phase 2)
        vec3 R = reflect(-V, N);
        vec3 F = fresnelSchlickRoughness(max(dot(N, V), 0.0), F0, roughness);
        vec3 kS = F;
        vec3 kD = 1.0 - kS;
        kD *= 1.0 - metallic;

        vec3 irradiance = texture(irradianceMap, N).rgb;
        vec3 diffuse = irradiance * albedo;

        const float MAX_REFLECTION_LOD = 4.0;
        vec3 prefilteredColor
            = textureLod(prefilterMap, R, roughness * MAX_REFLECTION_LOD).rgb;
        vec2 brdf = texture(brdfLUT, vec2(max(dot(N, V), 0.0), roughness)).rg;
        vec3 specular = prefilteredColor * (F * brdf.x + brdf.y);

        ambient = (kD * diffuse + specular) * ao;
    } else {
        // Simple ambient fallback
        ambient = ambientLightColor * albedo * ao;
    }

    vec3 color = ambient + Lo + material.emissive;

    // Tone mapping
    vec3 toneMapped = (toneMappingMode == 0) ? color : applyToneMapping(color);

    FragColor = vec4(toneMapped, 1.0);
}
