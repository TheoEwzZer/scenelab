#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;
layout (location = 2) in vec3 aNormal;

out vec2 TexCoord;

out vec3 GouraudColor;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

#define MAX_POINT_LIGHTS 16
#define MAX_SPOT_LIGHTS 16
#define MAX_DIRECTIONAL_LIGHTS 16

vec3 FragPos = vec3(0);

struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    vec3 emissive;
    float shininess;
};

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

struct LightOutput {
    vec3 L;
    vec3 H;
    vec3 color;
};

uniform int lightingModel;
uniform vec3 viewPosition;
uniform vec3 ambientLightColor;
uniform Material objectMaterial;
uniform int NB_DIR_LIGHTS;
uniform int NB_POINT_LIGHTS;
uniform int NB_SPOT_LIGHTS;
uniform PointLight pointLights[MAX_POINT_LIGHTS];
uniform DirectionalLight directionalLights[MAX_DIRECTIONAL_LIGHTS];
uniform SpotLight spotLights[MAX_SPOT_LIGHTS];

vec3 getViewVector() {
    return normalize(viewPosition - FragPos);
}

LightOutput prepareDirLight(DirectionalLight L)
{
    LightOutput Loutput;
    Loutput.L = normalize(-L.direction);
    Loutput.color = L.color;
    Loutput.H = normalize(Loutput.L + getViewVector());
    return Loutput;
}

LightOutput preparePointLight(PointLight L)
{
    LightOutput Loutput;

    vec3 lightV = L.position - FragPos;
    float dist = length(lightV);
    float attenuation = 1.0 / (L.ke + L.kl * dist + L.kq * (dist * dist));

    Loutput.L = normalize(lightV);
    Loutput.color = L.color * attenuation;
    Loutput.H = normalize(Loutput.L + getViewVector());

    return Loutput;
}

LightOutput prepareSpotLight(SpotLight L)
{
    LightOutput Loutput;

    vec3 lightV = L.position - FragPos;
    float dist = length(lightV);
    float attenuation = 1.0 / (L.ke + L.kl * dist + L.kq * (dist * dist));

    Loutput.L = normalize(lightV);
    vec3 spotDirection = normalize(L.direction);

    float intensity = attenuation * pow(max(dot(-Loutput.L, spotDirection), 0.0), L.p);
    Loutput.color = L.color * intensity;
    Loutput.H = normalize(Loutput.L + getViewVector());
    return Loutput;
}

vec3 modelGouraud(vec3 N, LightOutput LOutput, vec3 diffuseTextureColor)
{
    float diffuseFactor = max(dot(N, LOutput.L), 0.0);
    vec3 diffuse = LOutput.color * diffuseFactor * objectMaterial.diffuse * diffuseTextureColor;

    vec3 specular = vec3(0,0,0);
    if (diffuseFactor > 0.0) {
        vec3 R = normalize(2.0 * dot(N, LOutput.L) * N - LOutput.L);
        float lspecular = pow(max(dot(getViewVector(), R), 0.0), objectMaterial.shininess);
        specular = LOutput.color * lspecular * objectMaterial.specular;
    }

    return diffuse + specular;
}

void main()
{
    gl_Position = projection * view * model * vec4(aPos, 1.0);
    FragPos = vec3(model * vec4(aPos, 1.0));
    TexCoord = aTexCoord;

    vec3 Normal = mat3(transpose(inverse(model))) * aNormal;
    Normal = normalize(Normal);


    LightOutput LOutput;
    vec3 totalLight = vec3(0.0);
    vec3 ambient = ambientLightColor * objectMaterial.ambient;
    vec3 emissive = objectMaterial.emissive;
    for(int i = 0; i < NB_DIR_LIGHTS; i++) {
        LOutput = prepareDirLight(directionalLights[i]);
        totalLight += modelGouraud(Normal, LOutput, vec3(1,1,1));
    }
    for(int i = 0; i < NB_POINT_LIGHTS; i++) {
        LOutput = preparePointLight(pointLights[i]);
        totalLight += modelGouraud(Normal, LOutput, vec3(1,1,1));
    }
    for(int i = 0; i < NB_SPOT_LIGHTS; i++) {
        LOutput = prepareSpotLight(spotLights[i]);
        totalLight += modelGouraud(Normal, LOutput, vec3(1,1,1));
    }

    GouraudColor =  ambient + emissive + totalLight;
}
