#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec4 VertexColor;
in vec2 TexCoord;

uniform sampler2D ourTexture;
uniform bool useTexture;
uniform int filterMode;
uniform vec2 texelSize;

const vec3 LUMA_WEIGHTS = vec3(0.2126, 0.7152, 0.0722);

vec3 applyKernel(const float kernel[9])
{
    vec3 accum = vec3(0.0);
    vec2 offsets[9] = vec2[](vec2(-1.0, -1.0), vec2(0.0, -1.0),
        vec2(1.0, -1.0), vec2(-1.0, 0.0), vec2(0.0, 0.0), vec2(1.0, 0.0),
        vec2(-1.0, 1.0), vec2(0.0, 1.0), vec2(1.0, 1.0));

    for (int i = 0; i < 9; ++i) {
        vec2 sampleOffset = offsets[i] * texelSize;
        accum += kernel[i] * texture(ourTexture, TexCoord + sampleOffset).rgb;
    }
    return accum;
}

vec3 applyFilter(vec3 baseColor)
{
    if (filterMode == 0 || texelSize.x <= 0.0 || texelSize.y <= 0.0) {
        return baseColor;
    }

    if (filterMode == 1) {
        float luminance = dot(baseColor, LUMA_WEIGHTS);
        return vec3(luminance);
    }

    if (filterMode == 2) {
        const float sharpenKernel[9]
            = float[](0.0, -1.0, 0.0, -1.0, 5.0, -1.0, 0.0, -1.0, 0.0);
        return clamp(applyKernel(sharpenKernel), 0.0, 1.0);
    }

    if (filterMode == 3) {
        const float edgeKernel[9]
            = float[](-1.0, -1.0, -1.0, -1.0, 8.0, -1.0, -1.0, -1.0, -1.0);
        return clamp(applyKernel(edgeKernel), 0.0, 1.0);
    }

    if (filterMode == 4) {
        const float blurKernel[9] = float[](1.0 / 9.0, 1.0 / 9.0, 1.0 / 9.0,
            1.0 / 9.0, 1.0 / 9.0, 1.0 / 9.0, 1.0 / 9.0, 1.0 / 9.0, 1.0 / 9.0);
        return clamp(applyKernel(blurKernel), 0.0, 1.0);
    }

    return baseColor;
}

void main()
{
    vec4 color = VertexColor;

    if (useTexture) {
        vec4 sampled = texture(ourTexture, TexCoord);
        vec3 filtered = applyFilter(sampled.rgb);
        color = vec4(filtered, sampled.a) * VertexColor;
    }

    FragColor = color;
}