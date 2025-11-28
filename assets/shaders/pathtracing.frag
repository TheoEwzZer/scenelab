#version 330 core
out vec4 FragColor;
in vec3 FragPos;
uniform vec3 viewPos;
uniform mat3 viewRotationMatrix; // Precomputed rotation matrix from CPU
uniform float aspectRatio; // width / height
uniform float focalLength; // Precomputed from FOV on CPU
uniform int iFrame;
uniform sampler2D previousFrame; // Previous frame's accumulated color
uniform sampler2D triangleGeomTex;    // Geometry: v0, v1, v2, normal (3 pixels per triangle)
uniform sampler2D triangleMaterialTex; // Materials: color, emissive, specular (3 pixels per triangle)
uniform int numTriangles;

uniform sampler2D sphereGeomTex;      // Geometry: center.xyz, radius (1 pixel per sphere)
uniform sampler2D sphereMaterialTex;  // Materials: color, emissive, specular (3 pixels per sphere)
uniform int numSpheres;

uniform sampler2D planeGeomTex;       // Geometry: point.xyz, normal.xyz (2 pixels per plane)
uniform sampler2D planeMaterialTex;   // Materials: color, emissive, specular (3 pixels per plane)
uniform int numPlanes;

struct SMaterialInfo
{
    vec3 albedo;
    vec3 emissive;
    float percentSpecular;
    float roughness;
    vec3 specularColor;
};

struct SRayHitInfo
{
    float dist;
    vec3 normal;
    SMaterialInfo material;
};

// Geometry data - loaded for every intersection test (3 fetches)
struct TriangleGeom {
    vec3 v0;
    vec3 v1;
    vec3 v2;
    vec3 normal;
};

// Material data - only loaded for closest hit (3 fetches)
struct TriangleMaterial {
    vec3 color;
    vec3 emissive;
    float percentSpecular;
    float roughness;
    vec3 specularColor;
};

// Sphere geometry data
struct SphereGeom {
    vec3 center;
    float radius;
};

// Sphere material data
struct SphereMaterial {
    vec3 color;
    vec3 emissive;
    float percentSpecular;
    float roughness;
    vec3 specularColor;
};

// Plane geometry data
struct PlaneGeom {
    vec3 point;
    vec3 normal;
};

// Plane material data
struct PlaneMaterial {
    vec3 color;
    vec3 emissive;
    float percentSpecular;
    float roughness;
    vec3 specularColor;
};

// Fast geometry-only load - used during intersection testing
TriangleGeom loadTriangleGeom(int triIndex)
{
    TriangleGeom t;

    // Layout: width=3 pixels per row
    // Pixel 0: [v0.xyz, v1.x]
    vec4 p0 = texelFetch(triangleGeomTex, ivec2(0, triIndex), 0);
    t.v0 = p0.xyz;

    // Pixel 1: [v1.yz, v2.xy]
    vec4 p1 = texelFetch(triangleGeomTex, ivec2(1, triIndex), 0);
    t.v1 = vec3(p0.w, p1.xy);

    // Pixel 2: [v2.z, normal.xyz]
    vec4 p2 = texelFetch(triangleGeomTex, ivec2(2, triIndex), 0);
    t.v2 = vec3(p1.zw, p2.x);
    t.normal = p2.yzw;

    return t;
}

// Material load - only called for closest hit
TriangleMaterial loadTriangleMaterial(int triIndex)
{
    TriangleMaterial m;

    // Layout: width=3 pixels per row
    // Pixel 0: [color.xyz, percentSpecular]
    vec4 p0 = texelFetch(triangleMaterialTex, ivec2(0, triIndex), 0);
    m.color = p0.xyz;
    m.percentSpecular = p0.w;

    // Pixel 1: [emissive.xyz, roughness]
    vec4 p1 = texelFetch(triangleMaterialTex, ivec2(1, triIndex), 0);
    m.emissive = p1.xyz;
    m.roughness = p1.w;

    // Pixel 2: [specularColor.xyz, padding]
    vec4 p2 = texelFetch(triangleMaterialTex, ivec2(2, triIndex), 0);
    m.specularColor = p2.xyz;

    return m;
}

// Sphere geometry load
SphereGeom loadSphereGeom(int sphereIndex)
{
    SphereGeom s;
    // Layout: width=1 pixel per row
    // Pixel 0: [center.xyz, radius]
    vec4 p0 = texelFetch(sphereGeomTex, ivec2(0, sphereIndex), 0);
    s.center = p0.xyz;
    s.radius = p0.w;
    return s;
}

// Sphere material load
SphereMaterial loadSphereMaterial(int sphereIndex)
{
    SphereMaterial m;

    // Layout: width=3 pixels per row
    // Pixel 0: [color.xyz, percentSpecular]
    vec4 p0 = texelFetch(sphereMaterialTex, ivec2(0, sphereIndex), 0);
    m.color = p0.xyz;
    m.percentSpecular = p0.w;

    // Pixel 1: [emissive.xyz, roughness]
    vec4 p1 = texelFetch(sphereMaterialTex, ivec2(1, sphereIndex), 0);
    m.emissive = p1.xyz;
    m.roughness = p1.w;

    // Pixel 2: [specularColor.xyz, padding]
    vec4 p2 = texelFetch(sphereMaterialTex, ivec2(2, sphereIndex), 0);
    m.specularColor = p2.xyz;

    return m;
}

// Plane geometry load
PlaneGeom loadPlaneGeom(int planeIndex)
{
    PlaneGeom p;
    // Layout: width=2 pixels per row
    // Pixel 0: [point.xyz, normal.x]
    vec4 p0 = texelFetch(planeGeomTex, ivec2(0, planeIndex), 0);
    p.point = p0.xyz;

    // Pixel 1: [normal.yz, padding, padding]
    vec4 p1 = texelFetch(planeGeomTex, ivec2(1, planeIndex), 0);
    p.normal = vec3(p0.w, p1.xy);
    return p;
}

// Plane material load
PlaneMaterial loadPlaneMaterial(int planeIndex)
{
    PlaneMaterial m;

    // Layout: width=3 pixels per row
    // Pixel 0: [color.xyz, percentSpecular]
    vec4 p0 = texelFetch(planeMaterialTex, ivec2(0, planeIndex), 0);
    m.color = p0.xyz;
    m.percentSpecular = p0.w;

    // Pixel 1: [emissive.xyz, roughness]
    vec4 p1 = texelFetch(planeMaterialTex, ivec2(1, planeIndex), 0);
    m.emissive = p1.xyz;
    m.roughness = p1.w;

    // Pixel 2: [specularColor.xyz, padding]
    vec4 p2 = texelFetch(planeMaterialTex, ivec2(2, planeIndex), 0);
    m.specularColor = p2.xyz;

    return m;
}

const float c_epsilon = 0.0001f;
const float c_pi = 3.14159265359f;
const float c_twopi = 2.0f * c_pi;
const float c_rayPosNormalNudge = 0.01f;
const int c_numBounces = 30;
const float c_superFar = 10000.0f;
const float c_minimumRayHitTime = 0.1f;
const int c_numRendersPerFrame = 30;

bool TestTriangleTrace(in vec3 rayPos, in vec3 rayDir, inout SRayHitInfo info, in vec3 a, in vec3 b, in vec3 c, in vec3 precomputedNormal)
{
    float hit;
    vec3 barycentricCoord;

    // Use precomputed normal (already normalized) - avoids cross product per ray
    vec3 e0 = b - a;
    vec3 e1 = a - c;
    vec3 triangleNormal = precomputedNormal;

    // Unnormalized normal for intersection math (scale doesn't affect barycentric coords)
    vec3 unnormalizedNormal = cross(e1, e0);
    float valueDot = 1.0 / dot(unnormalizedNormal, rayDir);

    vec3 e2 = valueDot * (a - rayPos);
    vec3 i = cross(rayDir, e2);

    barycentricCoord.y = dot(i, e1);
    barycentricCoord.z = dot(i, e0);
    barycentricCoord.x = 1.0 - (barycentricCoord.z + barycentricCoord.y);
    hit = dot(unnormalizedNormal, e2);

    bool hitTest = (hit > c_epsilon) && (barycentricCoord.x > 0 && barycentricCoord.y > 0 && barycentricCoord.z > 0);

    if (hitTest)
    {
        if (hit > c_minimumRayHitTime && hit < info.dist)
        {
            info.dist = hit;
            info.normal = triangleNormal;  // Already normalized from CPU
            return true;
        }
    }

    return false;
}

// Ray-sphere intersection test
bool TestSphereTrace(in vec3 rayPos, in vec3 rayDir, inout SRayHitInfo info, in vec3 center, in float radius)
{
    vec3 oc = rayPos - center;
    float a = dot(rayDir, rayDir);
    float b = 2.0 * dot(oc, rayDir);
    float c = dot(oc, oc) - radius * radius;
    float discriminant = b * b - 4.0 * a * c;

    if (discriminant < 0.0) return false;

    float sqrtDisc = sqrt(discriminant);
    float t = (-b - sqrtDisc) / (2.0 * a);

    // Check first intersection
    if (t < c_minimumRayHitTime || t >= info.dist) {
        // Try second intersection
        t = (-b + sqrtDisc) / (2.0 * a);
        if (t < c_minimumRayHitTime || t >= info.dist) return false;
    }

    info.dist = t;
    vec3 hitPoint = rayPos + t * rayDir;
    info.normal = normalize(hitPoint - center);
    return true;
}

// Ray-plane intersection test (infinite plane)
bool TestPlaneTrace(in vec3 rayPos, in vec3 rayDir, inout SRayHitInfo info, in vec3 point, in vec3 normal)
{
    float denom = dot(normal, rayDir);

    // Check if ray is parallel to plane
    if (abs(denom) < c_epsilon) return false;

    float t = dot(point - rayPos, normal) / denom;

    if (t < c_minimumRayHitTime || t >= info.dist) return false;

    info.dist = t;
    // Return normal facing the ray
    info.normal = (denom > 0.0) ? -normal : normal;
    return true;
}

uint wang_hash(inout uint seed)
{
    seed = uint(seed ^ uint(61)) ^ uint(seed >> uint(16));
    seed *= uint(9);
    seed = seed ^ (seed >> 4);
    seed *= uint(0x27d4eb2d);
    seed = seed ^ (seed >> 15);
    return seed;
}

float RandomFloat01(inout uint state)
{
    return float(wang_hash(state)) / 4294967296.0;
}

vec3 RandomUnitVector(inout uint state)
{
    // Rejection sampling - avoids expensive sin/cos/sqrt
    vec3 p;
    float lenSq;
    for (int i = 0; i < 5; ++i) {  // Statistically sufficient iterations
        p = vec3(
            RandomFloat01(state) * 2.0f - 1.0f,
            RandomFloat01(state) * 2.0f - 1.0f,
            RandomFloat01(state) * 2.0f - 1.0f
        );
        lenSq = dot(p, p);
        if (lenSq <= 1.0f && lenSq > 0.0001f) {
            break;
        }
    }
    return p * inversesqrt(lenSq);
}

void TestSceneTrace(in vec3 rayPos, in vec3 rayDir, inout SRayHitInfo hitInfo)
{
    // Track closest hit: 0=triangle, 1=sphere, 2=plane
    int closestPrimitiveType = -1;
    int closestIndex = -1;

    // Test triangles
    for (int triIndex = 0; triIndex < numTriangles; ++triIndex) {
        TriangleGeom geom = loadTriangleGeom(triIndex);
        if (TestTriangleTrace(rayPos, rayDir, hitInfo,
              geom.v0,
              geom.v1,
              geom.v2,
              geom.normal))
        {
            closestPrimitiveType = 0;
            closestIndex = triIndex;
        }
    }

    // Test spheres
    for (int sphereIndex = 0; sphereIndex < numSpheres; ++sphereIndex) {
        SphereGeom geom = loadSphereGeom(sphereIndex);
        if (TestSphereTrace(rayPos, rayDir, hitInfo, geom.center, geom.radius))
        {
            closestPrimitiveType = 1;
            closestIndex = sphereIndex;
        }
    }

    // Test planes
    for (int planeIndex = 0; planeIndex < numPlanes; ++planeIndex) {
        PlaneGeom geom = loadPlaneGeom(planeIndex);
        if (TestPlaneTrace(rayPos, rayDir, hitInfo, geom.point, geom.normal))
        {
            closestPrimitiveType = 2;
            closestIndex = planeIndex;
        }
    }

    // Load materials only for closest hit
    if (closestIndex >= 0) {
        if (closestPrimitiveType == 0) {
            // Triangle
            TriangleMaterial mat = loadTriangleMaterial(closestIndex);
            hitInfo.material.albedo = mat.color;
            hitInfo.material.emissive = mat.emissive;
            hitInfo.material.percentSpecular = mat.percentSpecular;
            hitInfo.material.roughness = mat.roughness;
            hitInfo.material.specularColor = mat.specularColor;
        } else if (closestPrimitiveType == 1) {
            // Sphere
            SphereMaterial mat = loadSphereMaterial(closestIndex);
            hitInfo.material.albedo = mat.color;
            hitInfo.material.emissive = mat.emissive;
            hitInfo.material.percentSpecular = mat.percentSpecular;
            hitInfo.material.roughness = mat.roughness;
            hitInfo.material.specularColor = mat.specularColor;
        } else if (closestPrimitiveType == 2) {
            // Plane
            PlaneMaterial mat = loadPlaneMaterial(closestIndex);
            hitInfo.material.albedo = mat.color;
            hitInfo.material.emissive = mat.emissive;
            hitInfo.material.percentSpecular = mat.percentSpecular;
            hitInfo.material.roughness = mat.roughness;
            hitInfo.material.specularColor = mat.specularColor;
        }
    }
}

vec3 GetColorForRay(in vec3 startRayPos, in vec3 startRayDir, inout uint rngState)
{
    // initialize
    vec3 ret = vec3(0.0f, 0.0f, 0.0f);
    vec3 throughput = vec3(1.0f, 1.0f, 1.0f);
    vec3 rayPos = startRayPos;
    vec3 rayDir = startRayDir;

    for (int bounceIndex = 0; bounceIndex <= c_numBounces; ++bounceIndex)
    {
        // shoot a ray out into the world
        SRayHitInfo hitInfo;
        hitInfo.dist = c_superFar;
        TestSceneTrace(rayPos, rayDir, hitInfo);

        // if the ray missed, we are done
        if (hitInfo.dist == c_superFar)
            break;

        // update the ray position
        rayPos = (rayPos + rayDir * hitInfo.dist) + hitInfo.normal * c_rayPosNormalNudge;

        // calculate whether we are going to do a diffuse or specular reflection ray
        bool isSpecular = RandomFloat01(rngState) < hitInfo.material.percentSpecular;

        vec3 diffuseRayDir = normalize(hitInfo.normal + RandomUnitVector(rngState));
        vec3 specularRayDir = reflect(rayDir, hitInfo.normal);
        specularRayDir = normalize(mix(specularRayDir, diffuseRayDir, hitInfo.material.roughness * hitInfo.material.roughness));
        rayDir = isSpecular ? specularRayDir : diffuseRayDir;

        // add in emissive lighting
        ret += hitInfo.material.emissive * throughput;

        // update the colorMultiplier (ternary instead of mix for 0/1 case)
        throughput *= isSpecular ? hitInfo.material.specularColor : hitInfo.material.albedo;

        // Russian Roulette: terminate rays with negligible contribution
        float maxThroughput = max(throughput.r, max(throughput.g, throughput.b));
        if (maxThroughput < 0.01) {
            break;
        }
    }

    // return pixel color
    return ret;
}

void main()
{
    vec2 pixelCoord = (vec2(FragPos.x, FragPos.y) + 1.0) * 1000;
    uint rngState = uint(pixelCoord.x) * uint(1973) + uint(pixelCoord.y) * uint(9277) + uint(iFrame) * uint(26699);
    rngState = rngState | uint(1);

    vec3 rayPosition = viewPos;

    // Render multiple samples per frame for faster convergence
    vec3 currentColor = vec3(0.0);
    for (int i = 0; i < c_numRendersPerFrame; ++i) {
        // Add sub-pixel jitter for anti-aliasing
        vec2 jitter = vec2(RandomFloat01(rngState), RandomFloat01(rngState)) - 0.5;
        vec2 pixelSize = 2.0 / vec2(textureSize(previousFrame, 0));
        vec2 jitteredPos = vec2(FragPos.x, FragPos.y) + jitter * pixelSize;

        vec2 pixelTarget2D = vec2(jitteredPos.x * aspectRatio, jitteredPos.y);
        vec3 rayDirLocal = normalize(vec3(pixelTarget2D, -focalLength));
        vec3 rayDir = viewRotationMatrix * rayDirLocal;

        currentColor += GetColorForRay(rayPosition, rayDir, rngState);
    }
    currentColor /= float(c_numRendersPerFrame);

    vec2 uv = gl_FragCoord.xy / vec2(textureSize(previousFrame, 0));

    vec3 accumulatedColor;
    if (iFrame == 0) {
        accumulatedColor = currentColor;
    } else {
        vec3 previousColor = texture(previousFrame, uv).rgb;
        float weight = float(c_numRendersPerFrame) / float(iFrame * c_numRendersPerFrame + c_numRendersPerFrame);
        accumulatedColor = mix(previousColor, currentColor, weight);
    }

    FragColor = vec4(accumulatedColor, 1.0);
}
