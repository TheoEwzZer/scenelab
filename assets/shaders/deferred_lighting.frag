#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

// G-Buffer textures
uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedoSpec;
uniform sampler2D gMetallicRoughness;

// IBL textures
uniform samplerCube irradianceMap;
uniform samplerCube prefilterMap;
uniform sampler2D brdfLUT;
uniform bool useIBL;

// Camera
uniform vec3 viewPos;

// Lights
const int MAX_LIGHTS = 10;
uniform int numLights;

struct Light {
    int type; // 0 = point, 1 = directional, 2 = spot
    vec3 position;
    vec3 direction;
    vec3 color;
    float intensity;
    float constant;
    float linear;
    float quadratic;
    float cutOff;
    float outerCutOff;
};

uniform Light lights[MAX_LIGHTS];

// Tone mapping
uniform bool useToneMapping;
uniform float exposure;

const float PI = 3.14159265359;

// PBR functions
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

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;

    float nom = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / max(denom, 0.0001);
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
    return F0
        + (max(vec3(1.0 - roughness), F0) - F0)
        * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

vec3 calculateLight(Light light, vec3 N, vec3 V, vec3 fragPos, vec3 albedo,
    float metallic, float roughness, vec3 F0)
{
    vec3 L;
    float attenuation = 1.0;
    float spotIntensity = 1.0;

    if (light.type == 1) {
        // Directional light
        L = normalize(-light.direction);
    } else {
        // Point or spot light
        vec3 lightVec = light.position - fragPos;
        float distance = length(lightVec);
        L = normalize(lightVec);
        attenuation = 1.0
            / (light.constant + light.linear * distance
                + light.quadratic * distance * distance);

        if (light.type == 2) {
            // Spot light
            float theta = dot(L, normalize(-light.direction));
            float epsilon = light.cutOff - light.outerCutOff;
            spotIntensity
                = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
        }
    }

    vec3 H = normalize(V + L);

    vec3 radiance
        = light.color * light.intensity * attenuation * spotIntensity;

    // Cook-Torrance BRDF
    float NDF = DistributionGGX(N, H, roughness);
    float G = GeometrySmith(N, V, L, roughness);
    vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);

    vec3 numerator = NDF * G * F;
    float denominator
        = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
    vec3 specular = numerator / denominator;

    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - metallic;

    float NdotL = max(dot(N, L), 0.0);

    return (kD * albedo / PI + specular) * radiance * NdotL;
}

void main()
{
    // Retrieve data from G-Buffer
    vec3 FragPos = texture(gPosition, TexCoords).rgb;
    vec3 Normal = texture(gNormal, TexCoords).rgb;
    vec3 Albedo = texture(gAlbedoSpec, TexCoords).rgb;
    float AO = texture(gAlbedoSpec, TexCoords).a;
    float Metallic = texture(gMetallicRoughness, TexCoords).r;
    float Roughness = texture(gMetallicRoughness, TexCoords).g;

    // Check for empty fragment (background)
    if (length(Normal) < 0.1) {
        discard;
    }

    vec3 N = normalize(Normal);
    vec3 V = normalize(viewPos - FragPos);
    vec3 R = reflect(-V, N);

    // Calculate F0 (reflectance at normal incidence)
    vec3 F0 = vec3(0.04);
    F0 = mix(F0, Albedo, Metallic);

    // Direct lighting
    vec3 Lo = vec3(0.0);
    for (int i = 0; i < numLights && i < MAX_LIGHTS; ++i) {
        Lo += calculateLight(
            lights[i], N, V, FragPos, Albedo, Metallic, Roughness, F0);
    }

    // Ambient lighting (IBL or constant)
    vec3 ambient;
    if (useIBL) {
        vec3 F = fresnelSchlickRoughness(max(dot(N, V), 0.0), F0, Roughness);
        vec3 kS = F;
        vec3 kD = 1.0 - kS;
        kD *= 1.0 - Metallic;

        vec3 irradiance = texture(irradianceMap, N).rgb;
        vec3 diffuse = irradiance * Albedo;

        const float MAX_REFLECTION_LOD = 4.0;
        vec3 prefilteredColor
            = textureLod(prefilterMap, R, Roughness * MAX_REFLECTION_LOD).rgb;
        vec2 brdf = texture(brdfLUT, vec2(max(dot(N, V), 0.0), Roughness)).rg;
        vec3 specular = prefilteredColor * (F * brdf.x + brdf.y);

        ambient = (kD * diffuse + specular) * AO;
    } else {
        ambient = vec3(0.03) * Albedo * AO;
    }

    vec3 color = ambient + Lo;

    // Tone mapping
    if (useToneMapping) {
        color = vec3(1.0) - exp(-color * exposure);
    }

    // Gamma correction
    color = pow(color, vec3(1.0 / 2.2));

    FragColor = vec4(color, 1.0);
}
