#version 330 core
out vec4 FragColor;
in vec3 FragPos;
uniform vec3 viewPos;
uniform mat3 viewRotationMatrix; // Precomputed rotation matrix from CPU
uniform float aspectRatio; // width / height
uniform float focalLength; // Precomputed from FOV on CPU
uniform int iFrame;
uniform sampler2D previousFrame; // Previous frame's accumulated color
uniform sampler2D
    triangleGeomTex; // Geometry: v0, v1, v2, normal (3 pixels per triangle)
uniform sampler2D triangleMaterialTex; // Materials: color, emissive, specular
                                       // (3 pixels per triangle)
uniform int numTriangles;

uniform sampler2D
    sphereGeomTex; // Geometry: center.xyz, radius (1 pixel per sphere)
uniform sampler2D sphereMaterialTex; // Materials: color, emissive, specular (3
                                     // pixels per sphere)
uniform int numSpheres;

uniform sampler2D
    planeGeomTex; // Geometry: point.xyz, normal.xyz (2 pixels per plane)
uniform sampler2D planeMaterialTex; // Materials: color, emissive, specular (3
                                    // pixels per plane)
uniform int numPlanes;

// BVH acceleration structure
uniform sampler2D bvhNodeTex;  // BVH nodes: 2 pixels per node
uniform sampler2D bvhPrimTex;  // BVH primitives: 1 pixel per primitive [type, index, 0, 0]
uniform int numBVHNodes;

struct SMaterialInfo {
    vec3 albedo;
    vec3 emissive;
    float percentSpecular;
    float roughness;
    vec3 specularColor;
    float indexOfRefraction;
    float refractionChance;
};

struct SRayHitInfo {
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

// Material data - only loaded for closest hit (4 fetches)
struct TriangleMaterial {
    vec3 color;
    vec3 emissive;
    float percentSpecular;
    float roughness;
    vec3 specularColor;
    float indexOfRefraction;
    float refractionChance;
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
    float indexOfRefraction;
    float refractionChance;
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
    float indexOfRefraction;
    float refractionChance;
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

    // Layout: width=4 pixels per row
    // Pixel 0: [color.xyz, percentSpecular]
    vec4 p0 = texelFetch(triangleMaterialTex, ivec2(0, triIndex), 0);
    m.color = p0.xyz;
    m.percentSpecular = p0.w;

    // Pixel 1: [emissive.xyz, roughness]
    vec4 p1 = texelFetch(triangleMaterialTex, ivec2(1, triIndex), 0);
    m.emissive = p1.xyz;
    m.roughness = p1.w;

    // Pixel 2: [specularColor.xyz, indexOfRefraction]
    vec4 p2 = texelFetch(triangleMaterialTex, ivec2(2, triIndex), 0);
    m.specularColor = p2.xyz;
    m.indexOfRefraction = p2.w;

    // Pixel 3: [refractionChance, padding, padding, padding]
    vec4 p3 = texelFetch(triangleMaterialTex, ivec2(3, triIndex), 0);
    m.refractionChance = p3.x;

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

    // Layout: width=4 pixels per row
    // Pixel 0: [color.xyz, percentSpecular]
    vec4 p0 = texelFetch(sphereMaterialTex, ivec2(0, sphereIndex), 0);
    m.color = p0.xyz;
    m.percentSpecular = p0.w;

    // Pixel 1: [emissive.xyz, roughness]
    vec4 p1 = texelFetch(sphereMaterialTex, ivec2(1, sphereIndex), 0);
    m.emissive = p1.xyz;
    m.roughness = p1.w;

    // Pixel 2: [specularColor.xyz, indexOfRefraction]
    vec4 p2 = texelFetch(sphereMaterialTex, ivec2(2, sphereIndex), 0);
    m.specularColor = p2.xyz;
    m.indexOfRefraction = p2.w;

    // Pixel 3: [refractionChance, padding, padding, padding]
    vec4 p3 = texelFetch(sphereMaterialTex, ivec2(3, sphereIndex), 0);
    m.refractionChance = p3.x;

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

    // Layout: width=4 pixels per row
    // Pixel 0: [color.xyz, percentSpecular]
    vec4 p0 = texelFetch(planeMaterialTex, ivec2(0, planeIndex), 0);
    m.color = p0.xyz;
    m.percentSpecular = p0.w;

    // Pixel 1: [emissive.xyz, roughness]
    vec4 p1 = texelFetch(planeMaterialTex, ivec2(1, planeIndex), 0);
    m.emissive = p1.xyz;
    m.roughness = p1.w;

    // Pixel 2: [specularColor.xyz, indexOfRefraction]
    vec4 p2 = texelFetch(planeMaterialTex, ivec2(2, planeIndex), 0);
    m.specularColor = p2.xyz;
    m.indexOfRefraction = p2.w;

    // Pixel 3: [refractionChance, padding, padding, padding]
    vec4 p3 = texelFetch(planeMaterialTex, ivec2(3, planeIndex), 0);
    m.refractionChance = p3.x;

    return m;
}

// BVH node structure
struct BVHNode {
    vec3 boundsMin;
    vec3 boundsMax;
    int leftChild;  // -1 if leaf
    int packedData; // rightChild if internal, triStart|(triCount<<16) if leaf
};

// Load BVH node from texture
BVHNode loadBVHNode(int nodeIndex)
{
    BVHNode node;
    // Pixel 0: [bounds.min.xyz, leftChild as float bits]
    vec4 p0 = texelFetch(bvhNodeTex, ivec2(0, nodeIndex), 0);
    node.boundsMin = p0.xyz;
    node.leftChild = floatBitsToInt(p0.w);

    // Pixel 1: [bounds.max.xyz, packedData as float bits]
    vec4 p1 = texelFetch(bvhNodeTex, ivec2(1, nodeIndex), 0);
    node.boundsMax = p1.xyz;
    node.packedData = floatBitsToInt(p1.w);

    return node;
}

// BVH Primitive structure
struct BVHPrimitive {
    int type;           // 0 = triangle, 1 = sphere
    int originalIndex;  // Index in triangle/sphere array
};

// Load BVH primitive from texture
BVHPrimitive loadBVHPrimitive(int primIndex)
{
    BVHPrimitive prim;
    vec4 p = texelFetch(bvhPrimTex, ivec2(0, primIndex), 0);
    prim.type = int(p.x);
    prim.originalIndex = floatBitsToInt(p.y);
    return prim;
}

// Fast ray-AABB intersection test using slab method
bool intersectAABB(vec3 rayPos, vec3 invRayDir, vec3 bmin, vec3 bmax,
    float tMin, float tMax)
{
    vec3 t0 = (bmin - rayPos) * invRayDir;
    vec3 t1 = (bmax - rayPos) * invRayDir;
    vec3 tmin = min(t0, t1);
    vec3 tmax = max(t0, t1);
    float enter = max(max(tmin.x, tmin.y), max(tmin.z, tMin));
    float exit = min(min(tmax.x, tmax.y), min(tmax.z, tMax));
    return enter <= exit && exit > 0.0;
}

const float c_epsilon = 0.0001f;
const float c_pi = 3.14159265359f;
const float c_twopi = 2.0f * c_pi;
const float c_rayPosNormalNudge = 0.01f;
const int c_numBounces = 10;
const float c_superFar = 10000.0f;
const float c_minimumRayHitTime = 0.1f;
const int c_numRendersPerFrame = 1;

bool TestTriangleTrace(in vec3 rayPos, in vec3 rayDir, inout SRayHitInfo info,
    in vec3 a, in vec3 b, in vec3 c, in vec3 precomputedNormal)
{
    float hit;
    vec3 barycentricCoord;

    // Use precomputed normal (already normalized) - avoids cross product per
    // ray
    vec3 e0 = b - a;
    vec3 e1 = a - c;
    vec3 triangleNormal = precomputedNormal;

    // Unnormalized normal for intersection math (scale doesn't affect
    // barycentric coords)
    vec3 unnormalizedNormal = cross(e1, e0);
    float valueDot = 1.0 / dot(unnormalizedNormal, rayDir);

    vec3 e2 = valueDot * (a - rayPos);
    vec3 i = cross(rayDir, e2);

    barycentricCoord.y = dot(i, e1);
    barycentricCoord.z = dot(i, e0);
    barycentricCoord.x = 1.0 - (barycentricCoord.z + barycentricCoord.y);
    hit = dot(unnormalizedNormal, e2);

    bool hitTest = (hit > c_epsilon)
        && (barycentricCoord.x > 0 && barycentricCoord.y > 0
            && barycentricCoord.z > 0);

    if (hitTest) {
        if (hit > c_minimumRayHitTime && hit < info.dist) {
            info.dist = hit;
            info.normal = triangleNormal; // Already normalized from CPU
            return true;
        }
    }

    return false;
}

// Ray-sphere intersection test
bool TestSphereTrace(in vec3 rayPos, in vec3 rayDir, inout SRayHitInfo info,
    in vec3 center, in float radius)
{
    vec3 oc = rayPos - center;
    float a = dot(rayDir, rayDir);
    float b = 2.0 * dot(oc, rayDir);
    float c = dot(oc, oc) - radius * radius;
    float discriminant = b * b - 4.0 * a * c;

    if (discriminant < 0.0) {
        return false;
    }

    float sqrtDisc = sqrt(discriminant);
    float t = (-b - sqrtDisc) / (2.0 * a);

    // Check first intersection
    if (t < c_minimumRayHitTime || t >= info.dist) {
        // Try second intersection
        t = (-b + sqrtDisc) / (2.0 * a);
        if (t < c_minimumRayHitTime || t >= info.dist) {
            return false;
        }
    }

    info.dist = t;
    vec3 hitPoint = rayPos + t * rayDir;
    info.normal = normalize(hitPoint - center);
    return true;
}

// Ray-plane intersection test (infinite plane)
bool TestPlaneTrace(in vec3 rayPos, in vec3 rayDir, inout SRayHitInfo info,
    in vec3 point, in vec3 normal)
{
    float denom = dot(normal, rayDir);

    // Check if ray is parallel to plane
    if (abs(denom) < c_epsilon) {
        return false;
    }

    float t = dot(point - rayPos, normal) / denom;

    if (t < c_minimumRayHitTime || t >= info.dist) {
        return false;
    }

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
    for (int i = 0; i < 5; ++i) { // Statistically sufficient iterations
        p = vec3(RandomFloat01(state) * 2.0f - 1.0f,
            RandomFloat01(state) * 2.0f - 1.0f,
            RandomFloat01(state) * 2.0f - 1.0f);
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

    // Precompute inverse ray direction for AABB tests
    vec3 invRayDir = 1.0 / rayDir;

    // Use BVH traversal for triangles AND spheres
    if (numBVHNodes > 0) {
        // Stack-based BVH traversal
        int stack[64];
        int stackPtr = 0;
        stack[stackPtr++] = 0; // Start with root node

        while (stackPtr > 0) {
            int nodeIdx = stack[--stackPtr];
            BVHNode node = loadBVHNode(nodeIdx);

            // Test AABB intersection
            if (!intersectAABB(rayPos, invRayDir, node.boundsMin, node.boundsMax,
                    c_minimumRayHitTime, hitInfo.dist)) {
                continue;
            }

            if (node.leftChild == -1) {
                // Leaf node: test primitives (triangles or spheres)
                int primStart = node.packedData & 0xFFFF;
                int primCount = (node.packedData >> 16) & 0xFFFF;

                for (int i = 0; i < primCount; i++) {
                    BVHPrimitive prim = loadBVHPrimitive(primStart + i);

                    if (prim.type == 0) {
                        // Triangle
                        TriangleGeom geom = loadTriangleGeom(prim.originalIndex);
                        if (TestTriangleTrace(rayPos, rayDir, hitInfo, geom.v0,
                                geom.v1, geom.v2, geom.normal)) {
                            closestPrimitiveType = 0;
                            closestIndex = prim.originalIndex;
                        }
                    } else {
                        // Sphere
                        SphereGeom geom = loadSphereGeom(prim.originalIndex);
                        if (TestSphereTrace(rayPos, rayDir, hitInfo,
                                geom.center, geom.radius)) {
                            closestPrimitiveType = 1;
                            closestIndex = prim.originalIndex;
                        }
                    }
                }
            } else {
                // Internal node: push children onto stack
                // Push right child first so left is processed first
                stack[stackPtr++] = node.packedData; // rightChild
                stack[stackPtr++] = node.leftChild;
            }
        }
    } else {
        // Fallback: linear traversal if no BVH
        for (int triIndex = 0; triIndex < numTriangles; ++triIndex) {
            TriangleGeom geom = loadTriangleGeom(triIndex);
            if (TestTriangleTrace(rayPos, rayDir, hitInfo, geom.v0, geom.v1,
                    geom.v2, geom.normal)) {
                closestPrimitiveType = 0;
                closestIndex = triIndex;
            }
        }

        // Fallback for spheres too
        for (int sphereIndex = 0; sphereIndex < numSpheres; ++sphereIndex) {
            SphereGeom geom = loadSphereGeom(sphereIndex);
            if (TestSphereTrace(
                    rayPos, rayDir, hitInfo, geom.center, geom.radius)) {
                closestPrimitiveType = 1;
                closestIndex = sphereIndex;
            }
        }
    }

    // Test planes (keep linear - typically few planes, infinite extent)
    for (int planeIndex = 0; planeIndex < numPlanes; ++planeIndex) {
        PlaneGeom geom = loadPlaneGeom(planeIndex);
        if (TestPlaneTrace(rayPos, rayDir, hitInfo, geom.point, geom.normal)) {
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
            hitInfo.material.indexOfRefraction = mat.indexOfRefraction;
            hitInfo.material.refractionChance = mat.refractionChance;
        } else if (closestPrimitiveType == 1) {
            // Sphere
            SphereMaterial mat = loadSphereMaterial(closestIndex);
            hitInfo.material.albedo = mat.color;
            hitInfo.material.emissive = mat.emissive;
            hitInfo.material.percentSpecular = mat.percentSpecular;
            hitInfo.material.roughness = mat.roughness;
            hitInfo.material.specularColor = mat.specularColor;
            hitInfo.material.indexOfRefraction = mat.indexOfRefraction;
            hitInfo.material.refractionChance = mat.refractionChance;
        } else if (closestPrimitiveType == 2) {
            // Plane
            PlaneMaterial mat = loadPlaneMaterial(closestIndex);
            hitInfo.material.albedo = mat.color;
            hitInfo.material.emissive = mat.emissive;
            hitInfo.material.percentSpecular = mat.percentSpecular;
            hitInfo.material.roughness = mat.roughness;
            hitInfo.material.specularColor = mat.specularColor;
            hitInfo.material.indexOfRefraction = mat.indexOfRefraction;
            hitInfo.material.refractionChance = mat.refractionChance;
        }
    }
}

// Fresnel-Schlick approximation for reflectance
float FresnelSchlick(float cosTheta, float ior1, float ior2)
{
    float r0 = (ior1 - ior2) / (ior1 + ior2);
    r0 = r0 * r0;
    return r0 + (1.0 - r0) * pow(1.0 - cosTheta, 5.0);
}

vec3 GetColorForRay(
    in vec3 startRayPos, in vec3 startRayDir, inout uint rngState)
{
    // initialize
    vec3 ret = vec3(0.0f, 0.0f, 0.0f);
    vec3 throughput = vec3(1.0f, 1.0f, 1.0f);
    vec3 rayPos = startRayPos;
    vec3 rayDir = startRayDir;
    float currentIOR = 1.0; // Start in air

    for (int bounceIndex = 0; bounceIndex <= c_numBounces; ++bounceIndex) {
        // shoot a ray out into the world
        SRayHitInfo hitInfo;
        hitInfo.dist = c_superFar;
        TestSceneTrace(rayPos, rayDir, hitInfo);

        // if the ray missed, we are done
        if (hitInfo.dist == c_superFar) {
            break;
        }

        // add in emissive lighting
        ret += hitInfo.material.emissive * throughput;

        // Check if this material is refractive
        bool isRefractive = hitInfo.material.refractionChance > 0.0
            && hitInfo.material.indexOfRefraction > 1.0;

        if (isRefractive) {
            // Determine if we're entering or exiting the material
            bool entering = dot(rayDir, hitInfo.normal) < 0.0;
            vec3 normal = entering ? hitInfo.normal : -hitInfo.normal;

            float n1
                = entering ? currentIOR : hitInfo.material.indexOfRefraction;
            float n2 = entering ? hitInfo.material.indexOfRefraction : 1.0;
            float eta = n1 / n2;

            float cosTheta = abs(dot(-rayDir, normal));
            float sinTheta2 = eta * eta * (1.0 - cosTheta * cosTheta);

            // Calculate Fresnel reflectance
            float reflectance = FresnelSchlick(cosTheta, n1, n2);

            // Check for total internal reflection
            bool totalInternalReflection = sinTheta2 > 1.0;

            // Decide: reflect, refract, or diffuse
            float rand = RandomFloat01(rngState);
            float refractionProb
                = hitInfo.material.refractionChance * (1.0 - reflectance);

            if (totalInternalReflection || rand > refractionProb) {
                // Reflect (including specular and diffuse)
                float specularChance = hitInfo.material.percentSpecular;
                bool isSpecular = RandomFloat01(rngState) < specularChance;

                vec3 diffuseRayDir
                    = normalize(normal + RandomUnitVector(rngState));
                vec3 specularRayDir = reflect(rayDir, normal);
                specularRayDir = normalize(mix(specularRayDir, diffuseRayDir,
                    hitInfo.material.roughness * hitInfo.material.roughness));

                rayPos = (rayPos + rayDir * hitInfo.dist)
                    + normal * c_rayPosNormalNudge;
                rayDir = isSpecular ? specularRayDir : diffuseRayDir;
                throughput *= isSpecular ? hitInfo.material.specularColor
                                         : hitInfo.material.albedo;
            } else {
                // Refract using Snell's law
                vec3 refractDir = eta * rayDir
                    + (eta * cosTheta - sqrt(max(0.0, 1.0 - sinTheta2)))
                        * normal;
                refractDir = normalize(refractDir);

                // Move ray position through the surface
                rayPos = (rayPos + rayDir * hitInfo.dist)
                    - normal * c_rayPosNormalNudge;
                rayDir = refractDir;

                // Update current IOR for next intersection
                currentIOR
                    = entering ? hitInfo.material.indexOfRefraction : 1.0;

                // Glass typically doesn't absorb much light (use albedo for
                // tinted glass)
                throughput *= hitInfo.material.albedo;
            }
        } else {
            // Non-refractive material: original diffuse/specular logic
            rayPos = (rayPos + rayDir * hitInfo.dist)
                + hitInfo.normal * c_rayPosNormalNudge;

            bool isSpecular
                = RandomFloat01(rngState) < hitInfo.material.percentSpecular;

            vec3 diffuseRayDir
                = normalize(hitInfo.normal + RandomUnitVector(rngState));
            vec3 specularRayDir = reflect(rayDir, hitInfo.normal);
            specularRayDir = normalize(mix(specularRayDir, diffuseRayDir,
                hitInfo.material.roughness * hitInfo.material.roughness));
            rayDir = isSpecular ? specularRayDir : diffuseRayDir;

            throughput *= isSpecular ? hitInfo.material.specularColor
                                     : hitInfo.material.albedo;
        }

        // Russian Roulette: terminate rays with negligible contribution
        float maxThroughput
            = max(throughput.r, max(throughput.g, throughput.b));
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
    uint rngState = uint(pixelCoord.x) * uint(1973)
        + uint(pixelCoord.y) * uint(9277) + uint(iFrame) * uint(26699);
    rngState = rngState | uint(1);

    vec3 rayPosition = viewPos;

    // Render multiple samples per frame for faster convergence
    vec3 currentColor = vec3(0.0);
    for (int i = 0; i < c_numRendersPerFrame; ++i) {
        // Add sub-pixel jitter for anti-aliasing
        vec2 jitter
            = vec2(RandomFloat01(rngState), RandomFloat01(rngState)) - 0.5;
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
        float weight = float(c_numRendersPerFrame)
            / float(iFrame * c_numRendersPerFrame + c_numRendersPerFrame);
        accumulatedColor = mix(previousColor, currentColor, weight);
    }

    FragColor = vec4(accumulatedColor, 1.0);
}
