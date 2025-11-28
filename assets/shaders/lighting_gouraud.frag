#version 330 core

out vec4 FragColor;

in vec3 GouraudColor;
in vec2 TexCoord;

uniform sampler2D ourTexture;
uniform bool useTexture;

uniform int filterMode;
uniform vec2 texelSize;
uniform int toneMappingMode;
uniform float toneExposure;
const vec3 LUMA_WEIGHTS = vec3(0.2126, 0.7152, 0.0722);

vec3 applyKernel(const float kernel[9])
{
    vec3 accum = vec3(0.0);
    vec2 offsets[9] = vec2[](
        vec2(-1.0, -1.0), vec2(0.0, -1.0), vec2(1.0, -1.0),
        vec2(-1.0,  0.0), vec2(0.0,  0.0), vec2(1.0,  0.0),
        vec2(-1.0,  1.0), vec2(0.0,  1.0), vec2(1.0,  1.0)
    );

    for (int i = 0; i < 9; ++i) {
        vec2 sampleOffset = offsets[i] * texelSize;
        accum += kernel[i] * texture(ourTexture, TexCoord + sampleOffset).rgb;
    }
    return accum;
}

vec3 applyFilter(vec3 baseColor)
{
    if (!useTexture || filterMode == 0) {
        return baseColor;
    }

    if (filterMode == 1) {
        float luminance = dot(baseColor, LUMA_WEIGHTS);
        return vec3(luminance);
    }

    if (texelSize.x <= 0.0 || texelSize.y <= 0.0) {
        return baseColor;
    }

    if (filterMode == 2) {
        const float sharpenKernel[9] = float[](
            0.0, -1.0, 0.0,
           -1.0,  5.0, -1.0,
            0.0, -1.0, 0.0
        );
        return clamp(applyKernel(sharpenKernel), 0.0, 1.0);
    }

    if (filterMode == 3) {
        const float edgeKernel[9] = float[](
           -1.0, -1.0, -1.0,
           -1.0,  8.0, -1.0,
           -1.0, -1.0, -1.0
        );
        return clamp(applyKernel(edgeKernel), 0.0, 1.0);
    }

    if (filterMode == 4) {
        const float blurKernel[9] = float[](
            1.0/9.0, 1.0/9.0, 1.0/9.0,
            1.0/9.0, 1.0/9.0, 1.0/9.0,
            1.0/9.0, 1.0/9.0, 1.0/9.0
        );
        return clamp(applyKernel(blurKernel), 0.0, 1.0);
    }

    return baseColor;
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
            (mapped * (A * mapped + B)) / (mapped * (C * mapped + D) + E),
            0.0, 1.0);
        mapped = pow(mapped, vec3(1.0 / 2.2));
    }

    return mapped;
}


void main()
{
    vec4 sampledColor
        = useTexture ? texture(ourTexture, TexCoord) : vec4(1.0, 1.0, 1.0, 1.0);
    vec3 diffuseTextureColor = applyFilter(sampledColor.rgb);
    vec3 shaded = diffuseTextureColor * GouraudColor;
    vec3 toneMapped = applyToneMapping(shaded);
    vec3 finalColor = (toneMappingMode == 0) ? shaded : toneMapped;
    FragColor = vec4(finalColor, 1.0);
}