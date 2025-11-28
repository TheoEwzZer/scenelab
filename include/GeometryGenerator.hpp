/*
** IFT3100A25
** GeometryGenerator
** File description:
** Utility class for generating 3D geometric primitives procedurally
*/

#ifndef GEOMETRYGENERATOR_H
#define GEOMETRYGENERATOR_H

#include <vector>
#include <glm/glm.hpp>

// The returned vertex format is : pox.x | pox.y | pos.z | text.u | test.v |
// n.x | n.y | n.z

struct GData {
    std::vector<float> vertices;
    glm::vec3 aabbCorner1;
    glm::vec3 aabbCorner2;
};

class GeometryGenerator {
public:
    // sectors = vertical splits
    // subdivisions = horizontal splits
    static GData generateSphere(float radius, int sectors, int stacks);

    static GData generateCube(float size);

    // sectors = vertical splits
    static GData generateCylinder(float radius, float height, int sectors);

    static GData generatePlane(
        float width, float height, const glm::vec3 &normal);

private:
    static void addVertex(std::vector<float> &vertices,
        const glm::vec3 &position, const glm::vec2 &texCoord,
        const glm::vec3 &normal);
};

#endif /* GEOMETRYGENERATOR_H */
