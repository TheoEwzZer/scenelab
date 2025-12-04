#include "OBJLoader.hpp"

#include <format>
#include <fstream>
#include <iostream>
#include <sstream>

static void addVertex(GData &data, const std::vector<glm::vec3> &positions,
    const std::vector<glm::vec2> &texCoords,
    const std::vector<glm::vec3> &normals, int posIdx, int texIdx, int normIdx)
{
    glm::vec3 pos
        = (posIdx > 0 && posIdx <= static_cast<int>(positions.size()))
        ? positions[posIdx - 1]
        : glm::vec3(0.0f);

    glm::vec2 tex
        = (texIdx > 0 && texIdx <= static_cast<int>(texCoords.size()))
        ? texCoords[texIdx - 1]
        : glm::vec2(0.0f);

    glm::vec3 norm
        = (normIdx > 0 && normIdx <= static_cast<int>(normals.size()))
        ? normals[normIdx - 1]
        : glm::vec3(0.0f, 0.0f, 1.0f);

    Vertex::addVertex(data.vertices, pos, tex, norm);
    Vertex::computeTangents(data.vertices);
}

GData OBJLoader::loadOBJ(
    const std::string &filename, const std::string &filepath)
{
    GData data;

    std::vector<glm::vec3> positions;
    std::vector<glm::vec2> texCoords;
    std::vector<glm::vec3> normals;

    glm::vec3 minBounds(std::numeric_limits<float>::max());
    glm::vec3 maxBounds(std::numeric_limits<float>::lowest());

    std::ifstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "[ERROR] Failed to open OBJ file: " << filename
                  << std::endl;
        return data;
    }

    std::string line;
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string prefix;
        iss >> prefix;

        if (prefix == "v") {
            glm::vec3 pos;
            iss >> pos.x >> pos.y >> pos.z;
            positions.push_back(pos);

            minBounds = glm::min(minBounds, pos);
            maxBounds = glm::max(maxBounds, pos);

        } else if (prefix == "vt") {
            glm::vec2 tex;
            iss >> tex.x >> tex.y;
            texCoords.push_back(tex);

        } else if (prefix == "vn") {
            glm::vec3 norm;
            iss >> norm.x >> norm.y >> norm.z;
            normals.push_back(norm);

        } else if (prefix == "f") {
            std::string vertexStr;
            std::vector<std::array<int, 3>> faceIndices;

            while (iss >> vertexStr) {
                int posIdx = 0, texIdx = 0, normIdx = 0;

                std::replace(vertexStr.begin(), vertexStr.end(), '/', ' ');
                std::istringstream vertexStream(vertexStr);

                vertexStream >> posIdx;
                if (vertexStream >> texIdx) {
                    vertexStream >> normIdx;
                }

                faceIndices.push_back({ posIdx, texIdx, normIdx });
            }

            if (faceIndices.size() == 3) {
                addVertex(data, positions, texCoords, normals,
                    faceIndices[0][0], faceIndices[0][1], faceIndices[0][2]);
                addVertex(data, positions, texCoords, normals,
                    faceIndices[1][0], faceIndices[1][1], faceIndices[1][2]);
                addVertex(data, positions, texCoords, normals,
                    faceIndices[2][0], faceIndices[2][1], faceIndices[2][2]);

            } else if (faceIndices.size() == 4) {
                addVertex(data, positions, texCoords, normals,
                    faceIndices[0][0], faceIndices[0][1], faceIndices[0][2]);
                addVertex(data, positions, texCoords, normals,
                    faceIndices[1][0], faceIndices[1][1], faceIndices[1][2]);
                addVertex(data, positions, texCoords, normals,
                    faceIndices[2][0], faceIndices[2][1], faceIndices[2][2]);

                addVertex(data, positions, texCoords, normals,
                    faceIndices[0][0], faceIndices[0][1], faceIndices[0][2]);
                addVertex(data, positions, texCoords, normals,
                    faceIndices[2][0], faceIndices[2][1], faceIndices[2][2]);
                addVertex(data, positions, texCoords, normals,
                    faceIndices[3][0], faceIndices[3][1], faceIndices[3][2]);
            }
        }
    }

    file.close();

    if (positions.empty()) {
        std::cerr << "[ERROR] No vertices found in OBJ file: " << filepath
                  << std::endl;
        return data;
    }

    if (data.vertices.empty()) {
        std::cerr << "[ERROR] No faces found in OBJ file: " << filepath
                  << std::endl;
        return data;
    }

    data.aabbCorner1 = minBounds;
    data.aabbCorner2 = maxBounds;

    return data;
}
