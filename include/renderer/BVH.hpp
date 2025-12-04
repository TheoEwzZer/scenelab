#pragma once

#include <glm/glm.hpp>
#include <vector>

// Forward declarations
struct Triangle;
struct AnalyticalSphereData;

struct AABB {
    glm::vec3 min { std::numeric_limits<float>::max() };
    glm::vec3 max { std::numeric_limits<float>::lowest() };

    void expand(const glm::vec3 &point)
    {
        min = glm::min(min, point);
        max = glm::max(max, point);
    }

    void expand(const AABB &other)
    {
        min = glm::min(min, other.min);
        max = glm::max(max, other.max);
    }

    glm::vec3 centroid() const { return (min + max) * 0.5f; }

    glm::vec3 extent() const { return max - min; }

    float surfaceArea() const
    {
        glm::vec3 e = extent();
        return 2.0f * (e.x * e.y + e.y * e.z + e.z * e.x);
    }

    int longestAxis() const
    {
        glm::vec3 e = extent();
        if (e.x > e.y && e.x > e.z) {
            return 0;
        }
        if (e.y > e.z) {
            return 1;
        }
        return 2;
    }
};

// Primitive types for BVH
enum class BVHPrimitiveType : uint8_t { Triangle = 0, Sphere = 1 };

// A primitive reference in the BVH (can be triangle or sphere)
struct BVHPrimitive {
    BVHPrimitiveType type;
    int originalIndex; // Index in original triangle/sphere array
    AABB bounds;
    glm::vec3 centroid;
};

struct BVHNode {
    AABB bounds;
    int leftChild = -1; // -1 if leaf
    int rightChild = -1; // -1 if leaf
    int primitiveStart = 0; // Start index in reordered primitive array
    int primitiveCount = 0; // Number of primitives (0 if internal node)

    bool isLeaf() const { return primitiveCount > 0; }
};

class BVH {
public:
    // Build BVH from triangles and spheres
    // Arrays will be reordered in place to match BVH leaf order
    void build(std::vector<Triangle> &triangles,
        std::vector<AnalyticalSphereData> &spheres);

    const std::vector<BVHNode> &getNodes() const { return m_nodes; }

    int getNodeCount() const { return static_cast<int>(m_nodes.size()); }

    // Get the reordered primitive list (for GPU upload)
    const std::vector<BVHPrimitive> &getPrimitives() const
    {
        return m_primitives;
    }

    int getPrimitiveCount() const
    {
        return static_cast<int>(m_primitives.size());
    }

    void clear()
    {
        m_nodes.clear();
        m_primitives.clear();
    }

private:
    std::vector<BVHNode> m_nodes;
    std::vector<BVHPrimitive> m_primitives;

    static constexpr int MAX_LEAF_PRIMITIVES = 4;
    static constexpr int SAH_BUCKETS = 12;
    static constexpr float TRAVERSAL_COST = 1.0f;
    static constexpr float INTERSECTION_COST = 1.0f;

    // Build BVH recursively, returns node index
    int buildRecursive(
        std::vector<int> &indices, int start, int end, int depth);

    // Find best split using SAH
    struct SplitResult {
        int axis = -1;
        float position = 0.0f;
        float cost = std::numeric_limits<float>::max();
        int splitIndex = -1;
    };

    SplitResult findBestSplit(const std::vector<int> &indices, int start,
        int end, const AABB &nodeBounds, const AABB &centroidBounds);
};
