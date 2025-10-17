#include "GeometryGenerator.hpp"

#include <cmath>

void GeometryGenerator::addVertex(std::vector<float> &vertices,
    const glm::vec3 &position, const glm::vec2 &texCoord,
    const glm::vec3 &normal)
{
    vertices.push_back(position.x);
    vertices.push_back(position.y);
    vertices.push_back(position.z);
    vertices.push_back(texCoord.x);
    vertices.push_back(texCoord.y);
    vertices.push_back(normal.x);
    vertices.push_back(normal.y);
    vertices.push_back(normal.z);
}

std::vector<float> GeometryGenerator::generateSphere(
    float radius, int sectors, int stacks)
{
    std::vector<float> vertices;
    vertices.reserve((stacks * sectors * 6 - sectors * 6) * 8);

    float sectorStep = 2.0f * M_PI / sectors;
    float stackStep = M_PI / stacks;

    std::vector<glm::vec3> positions;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec2> texCoords;
    positions.reserve((stacks + 1) * (sectors + 1));
    normals.reserve((stacks + 1) * (sectors + 1));
    texCoords.reserve((stacks + 1) * (sectors + 1));

    for (int i = 0; i <= stacks; ++i) {
        float stackAngle = M_PI / 2.0f - i * stackStep;
        float xy = radius * cosf(stackAngle);
        float z = radius * sinf(stackAngle);

        for (int j = 0; j <= sectors; ++j) {
            float sectorAngle = j * sectorStep;

            glm::vec3 position;
            position.x = xy * cosf(sectorAngle);
            position.y = xy * sinf(sectorAngle);
            position.z = z;
            positions.push_back(position);

            glm::vec3 normal = glm::normalize(position);
            normals.push_back(normal);

            glm::vec2 texCoord;
            texCoord.x = (float)j / sectors;
            texCoord.y = (float)i / stacks;
            texCoords.push_back(texCoord);
        }
    }

    for (int i = 0; i < stacks; ++i) {
        int k1 = i * (sectors + 1);
        int k2 = k1 + sectors + 1;

        for (int j = 0; j < sectors; ++j, ++k1, ++k2) {
            if (i != 0) {
                addVertex(vertices, positions[k1], texCoords[k1], normals[k1]);
                addVertex(vertices, positions[k2], texCoords[k2], normals[k2]);
                addVertex(vertices, positions[k1 + 1], texCoords[k1 + 1],
                    normals[k1 + 1]);
            }

            if (i != (stacks - 1)) {
                addVertex(vertices, positions[k1 + 1], texCoords[k1 + 1],
                    normals[k1 + 1]);
                addVertex(vertices, positions[k2], texCoords[k2], normals[k2]);
                addVertex(vertices, positions[k2 + 1], texCoords[k2 + 1],
                    normals[k2 + 1]);
            }
        }
    }

    return vertices;
}

std::vector<float> GeometryGenerator::generateCube(float size)
{
    std::vector<float> vertices;
    vertices.reserve(36 * 8);
    float halfSize = size * 0.5f;

    glm::vec3 normal = glm::vec3(0.0f, 0.0f, 1.0f);
    addVertex(vertices, glm::vec3(-halfSize, -halfSize, halfSize),
        glm::vec2(0.0f, 0.0f), normal);
    addVertex(vertices, glm::vec3(halfSize, -halfSize, halfSize),
        glm::vec2(1.0f, 0.0f), normal);
    addVertex(vertices, glm::vec3(halfSize, halfSize, halfSize),
        glm::vec2(1.0f, 1.0f), normal);
    addVertex(vertices, glm::vec3(halfSize, halfSize, halfSize),
        glm::vec2(1.0f, 1.0f), normal);
    addVertex(vertices, glm::vec3(-halfSize, halfSize, halfSize),
        glm::vec2(0.0f, 1.0f), normal);
    addVertex(vertices, glm::vec3(-halfSize, -halfSize, halfSize),
        glm::vec2(0.0f, 0.0f), normal);

    normal = glm::vec3(0.0f, 0.0f, -1.0f);
    addVertex(vertices, glm::vec3(halfSize, -halfSize, -halfSize),
        glm::vec2(0.0f, 0.0f), normal);
    addVertex(vertices, glm::vec3(-halfSize, -halfSize, -halfSize),
        glm::vec2(1.0f, 0.0f), normal);
    addVertex(vertices, glm::vec3(-halfSize, halfSize, -halfSize),
        glm::vec2(1.0f, 1.0f), normal);
    addVertex(vertices, glm::vec3(-halfSize, halfSize, -halfSize),
        glm::vec2(1.0f, 1.0f), normal);
    addVertex(vertices, glm::vec3(halfSize, halfSize, -halfSize),
        glm::vec2(0.0f, 1.0f), normal);
    addVertex(vertices, glm::vec3(halfSize, -halfSize, -halfSize),
        glm::vec2(0.0f, 0.0f), normal);

    normal = glm::vec3(-1.0f, 0.0f, 0.0f);
    addVertex(vertices, glm::vec3(-halfSize, -halfSize, -halfSize),
        glm::vec2(0.0f, 0.0f), normal);
    addVertex(vertices, glm::vec3(-halfSize, -halfSize, halfSize),
        glm::vec2(1.0f, 0.0f), normal);
    addVertex(vertices, glm::vec3(-halfSize, halfSize, halfSize),
        glm::vec2(1.0f, 1.0f), normal);
    addVertex(vertices, glm::vec3(-halfSize, halfSize, halfSize),
        glm::vec2(1.0f, 1.0f), normal);
    addVertex(vertices, glm::vec3(-halfSize, halfSize, -halfSize),
        glm::vec2(0.0f, 1.0f), normal);
    addVertex(vertices, glm::vec3(-halfSize, -halfSize, -halfSize),
        glm::vec2(0.0f, 0.0f), normal);

    normal = glm::vec3(1.0f, 0.0f, 0.0f);
    addVertex(vertices, glm::vec3(halfSize, -halfSize, halfSize),
        glm::vec2(0.0f, 0.0f), normal);
    addVertex(vertices, glm::vec3(halfSize, -halfSize, -halfSize),
        glm::vec2(1.0f, 0.0f), normal);
    addVertex(vertices, glm::vec3(halfSize, halfSize, -halfSize),
        glm::vec2(1.0f, 1.0f), normal);
    addVertex(vertices, glm::vec3(halfSize, halfSize, -halfSize),
        glm::vec2(1.0f, 1.0f), normal);
    addVertex(vertices, glm::vec3(halfSize, halfSize, halfSize),
        glm::vec2(0.0f, 1.0f), normal);
    addVertex(vertices, glm::vec3(halfSize, -halfSize, halfSize),
        glm::vec2(0.0f, 0.0f), normal);

    normal = glm::vec3(0.0f, -1.0f, 0.0f);
    addVertex(vertices, glm::vec3(-halfSize, -halfSize, -halfSize),
        glm::vec2(0.0f, 0.0f), normal);
    addVertex(vertices, glm::vec3(halfSize, -halfSize, -halfSize),
        glm::vec2(1.0f, 0.0f), normal);
    addVertex(vertices, glm::vec3(halfSize, -halfSize, halfSize),
        glm::vec2(1.0f, 1.0f), normal);
    addVertex(vertices, glm::vec3(halfSize, -halfSize, halfSize),
        glm::vec2(1.0f, 1.0f), normal);
    addVertex(vertices, glm::vec3(-halfSize, -halfSize, halfSize),
        glm::vec2(0.0f, 1.0f), normal);
    addVertex(vertices, glm::vec3(-halfSize, -halfSize, -halfSize),
        glm::vec2(0.0f, 0.0f), normal);

    normal = glm::vec3(0.0f, 1.0f, 0.0f);
    addVertex(vertices, glm::vec3(-halfSize, halfSize, halfSize),
        glm::vec2(0.0f, 0.0f), normal);
    addVertex(vertices, glm::vec3(halfSize, halfSize, halfSize),
        glm::vec2(1.0f, 0.0f), normal);
    addVertex(vertices, glm::vec3(halfSize, halfSize, -halfSize),
        glm::vec2(1.0f, 1.0f), normal);
    addVertex(vertices, glm::vec3(halfSize, halfSize, -halfSize),
        glm::vec2(1.0f, 1.0f), normal);
    addVertex(vertices, glm::vec3(-halfSize, halfSize, -halfSize),
        glm::vec2(0.0f, 1.0f), normal);
    addVertex(vertices, glm::vec3(-halfSize, halfSize, halfSize),
        glm::vec2(0.0f, 0.0f), normal);

    return vertices;
}

std::vector<float> GeometryGenerator::generateCylinder(
    float radius, float height, int sectors)
{
    std::vector<float> vertices;
    vertices.reserve((sectors * 12 + sectors * 6) * 8);
    float halfHeight = height * 0.5f;
    float sectorStep = 2.0f * M_PI / sectors;

    std::vector<glm::vec3> sidePositionsTop;
    std::vector<glm::vec3> sidePositionsBottom;
    std::vector<glm::vec3> sideNormals;
    sidePositionsTop.reserve(sectors + 1);
    sidePositionsBottom.reserve(sectors + 1);
    sideNormals.reserve(sectors + 1);

    for (int i = 0; i <= sectors; ++i) {
        float sectorAngle = i * sectorStep;
        float x = cosf(sectorAngle);
        float z = sinf(sectorAngle);

        sidePositionsTop.push_back(
            glm::vec3(radius * x, halfHeight, radius * z));
        sidePositionsBottom.push_back(
            glm::vec3(radius * x, -halfHeight, radius * z));
        sideNormals.push_back(glm::normalize(glm::vec3(x, 0.0f, z)));
    }

    for (int i = 0; i < sectors; ++i) {
        addVertex(vertices, sidePositionsBottom[i],
            glm::vec2((float)i / sectors, 0.0f), sideNormals[i]);
        addVertex(vertices, sidePositionsTop[i],
            glm::vec2((float)i / sectors, 1.0f), sideNormals[i]);
        addVertex(vertices, sidePositionsTop[i + 1],
            glm::vec2((float)(i + 1) / sectors, 1.0f), sideNormals[i + 1]);

        addVertex(vertices, sidePositionsTop[i + 1],
            glm::vec2((float)(i + 1) / sectors, 1.0f), sideNormals[i + 1]);
        addVertex(vertices, sidePositionsBottom[i + 1],
            glm::vec2((float)(i + 1) / sectors, 0.0f), sideNormals[i + 1]);
        addVertex(vertices, sidePositionsBottom[i],
            glm::vec2((float)i / sectors, 0.0f), sideNormals[i]);
    }

    glm::vec3 topCenter(0.0f, halfHeight, 0.0f);
    glm::vec3 topNormal(0.0f, 1.0f, 0.0f);

    for (int i = 0; i < sectors; ++i) {
        float angle1 = i * sectorStep;
        float angle2 = (i + 1) * sectorStep;

        glm::vec3 p1(radius * cosf(angle1), halfHeight, radius * sinf(angle1));
        glm::vec3 p2(radius * cosf(angle2), halfHeight, radius * sinf(angle2));

        addVertex(vertices, topCenter, glm::vec2(0.5f, 0.5f), topNormal);
        addVertex(vertices, p1,
            glm::vec2(0.5f + 0.5f * cosf(angle1), 0.5f + 0.5f * sinf(angle1)),
            topNormal);
        addVertex(vertices, p2,
            glm::vec2(0.5f + 0.5f * cosf(angle2), 0.5f + 0.5f * sinf(angle2)),
            topNormal);
    }

    glm::vec3 bottomCenter(0.0f, -halfHeight, 0.0f);
    glm::vec3 bottomNormal(0.0f, -1.0f, 0.0f);

    for (int i = 0; i < sectors; ++i) {
        float angle1 = i * sectorStep;
        float angle2 = (i + 1) * sectorStep;

        glm::vec3 p1(
            radius * cosf(angle1), -halfHeight, radius * sinf(angle1));
        glm::vec3 p2(
            radius * cosf(angle2), -halfHeight, radius * sinf(angle2));

        addVertex(vertices, bottomCenter, glm::vec2(0.5f, 0.5f), bottomNormal);
        addVertex(vertices, p2,
            glm::vec2(0.5f + 0.5f * cosf(angle2), 0.5f + 0.5f * sinf(angle2)),
            bottomNormal);
        addVertex(vertices, p1,
            glm::vec2(0.5f + 0.5f * cosf(angle1), 0.5f + 0.5f * sinf(angle1)),
            bottomNormal);
    }

    return vertices;
}
