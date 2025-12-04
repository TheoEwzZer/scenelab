#include "renderer/BVH.hpp"
#include "renderer/implementation/PathTracingRenderer.hpp"
#include <algorithm>
#include <numeric>

void BVH::build(std::vector<Triangle> &triangles,
    std::vector<AnalyticalSphereData> &spheres)
{
    clear();

    size_t totalPrimitives = triangles.size() + spheres.size();
    if (totalPrimitives == 0) {
        return;
    }

    // Build primitive list with AABBs and centroids
    m_primitives.reserve(totalPrimitives);

    // Add triangles
    for (size_t i = 0; i < triangles.size(); i++) {
        BVHPrimitive prim;
        prim.type = BVHPrimitiveType::Triangle;
        prim.originalIndex = static_cast<int>(i);

        // Compute AABB for triangle
        const Triangle &tri = triangles[i];
        prim.bounds.expand(tri.v0);
        prim.bounds.expand(tri.v1);
        prim.bounds.expand(tri.v2);

        // Centroid is center of triangle
        prim.centroid = (tri.v0 + tri.v1 + tri.v2) / 3.0f;

        m_primitives.push_back(prim);
    }

    // Add spheres
    for (size_t i = 0; i < spheres.size(); i++) {
        BVHPrimitive prim;
        prim.type = BVHPrimitiveType::Sphere;
        prim.originalIndex = static_cast<int>(i);

        // Compute AABB for sphere (box that contains the sphere)
        const AnalyticalSphereData &sphere = spheres[i];
        glm::vec3 radiusVec(sphere.radius);
        prim.bounds.min = sphere.center - radiusVec;
        prim.bounds.max = sphere.center + radiusVec;

        // Centroid is center of sphere
        prim.centroid = sphere.center;

        m_primitives.push_back(prim);
    }

    // Create index array
    std::vector<int> indices(m_primitives.size());
    std::iota(indices.begin(), indices.end(), 0);

    // Reserve space for nodes (roughly 2N-1 for N primitives)
    m_nodes.reserve(2 * m_primitives.size());

    // Build recursively
    buildRecursive(indices, 0, static_cast<int>(m_primitives.size()), 0);

    // Reorder primitives according to BVH order
    std::vector<BVHPrimitive> reorderedPrimitives(m_primitives.size());
    for (size_t i = 0; i < indices.size(); i++) {
        reorderedPrimitives[i] = m_primitives[indices[i]];
    }
    m_primitives = std::move(reorderedPrimitives);

    // Also reorder the original arrays to match
    // First, build mapping from new index to old index for each type
    std::vector<Triangle> reorderedTriangles;
    std::vector<AnalyticalSphereData> reorderedSpheres;
    reorderedTriangles.reserve(triangles.size());
    reorderedSpheres.reserve(spheres.size());

    // Update originalIndex to point to new positions
    int newTriIdx = 0;
    int newSphereIdx = 0;
    for (auto &prim : m_primitives) {
        if (prim.type == BVHPrimitiveType::Triangle) {
            reorderedTriangles.push_back(triangles[prim.originalIndex]);
            prim.originalIndex = newTriIdx++;
        } else {
            reorderedSpheres.push_back(spheres[prim.originalIndex]);
            prim.originalIndex = newSphereIdx++;
        }
    }

    triangles = std::move(reorderedTriangles);
    spheres = std::move(reorderedSpheres);
}

int BVH::buildRecursive(
    std::vector<int> &indices, int start, int end, int depth)
{
    int nodeIndex = static_cast<int>(m_nodes.size());
    m_nodes.push_back(BVHNode {});
    BVHNode &node = m_nodes[nodeIndex];

    // Compute bounds for this node
    AABB centroidBounds;
    for (int i = start; i < end; i++) {
        node.bounds.expand(m_primitives[indices[i]].bounds);
        centroidBounds.expand(m_primitives[indices[i]].centroid);
    }

    int numPrimitives = end - start;

    // Create leaf if few primitives or max depth reached
    if (numPrimitives <= MAX_LEAF_PRIMITIVES || depth > 32) {
        node.primitiveStart = start;
        node.primitiveCount = numPrimitives;
        return nodeIndex;
    }

    // Find best split using SAH
    SplitResult split
        = findBestSplit(indices, start, end, node.bounds, centroidBounds);

    // If no good split found, create leaf
    if (split.axis == -1 || split.splitIndex == start
        || split.splitIndex == end) {
        node.primitiveStart = start;
        node.primitiveCount = numPrimitives;
        return nodeIndex;
    }

    // Partition primitives
    int mid = split.splitIndex;

    // Sort indices by centroid along split axis
    std::nth_element(indices.begin() + start, indices.begin() + mid,
        indices.begin() + end, [this, axis = split.axis](int a, int b) {
            return m_primitives[a].centroid[axis]
                < m_primitives[b].centroid[axis];
        });

    // Recursively build children
    node.leftChild = buildRecursive(indices, start, mid, depth + 1);
    // Need to re-get reference after potential reallocation
    m_nodes[nodeIndex].rightChild
        = buildRecursive(indices, mid, end, depth + 1);

    return nodeIndex;
}

BVH::SplitResult BVH::findBestSplit(const std::vector<int> &indices, int start,
    int end, const AABB &nodeBounds, const AABB &centroidBounds)
{
    SplitResult best;
    int numPrimitives = end - start;

    // Cost of not splitting (making a leaf)
    float leafCost = static_cast<float>(numPrimitives) * INTERSECTION_COST;

    // Try each axis
    for (int axis = 0; axis < 3; axis++) {
        // Skip if extent is zero
        if (centroidBounds.extent()[axis] < 1e-6f) {
            continue;
        }

        // Initialize buckets
        struct Bucket {
            int count = 0;
            AABB bounds;
        };

        std::vector<Bucket> buckets(SAH_BUCKETS);

        float axisMin = centroidBounds.min[axis];
        float axisExtent = centroidBounds.extent()[axis];

        // Assign primitives to buckets
        for (int i = start; i < end; i++) {
            int primIdx = indices[i];
            float centroid = m_primitives[primIdx].centroid[axis];
            int bucketIdx = static_cast<int>(
                SAH_BUCKETS * (centroid - axisMin) / axisExtent);
            bucketIdx = std::clamp(bucketIdx, 0, SAH_BUCKETS - 1);
            buckets[bucketIdx].count++;
            buckets[bucketIdx].bounds.expand(m_primitives[primIdx].bounds);
        }

        // Compute costs for each split
        // Precompute prefix sums from left
        std::vector<int> countLeft(SAH_BUCKETS);
        std::vector<AABB> boundsLeft(SAH_BUCKETS);
        countLeft[0] = buckets[0].count;
        boundsLeft[0] = buckets[0].bounds;
        for (int i = 1; i < SAH_BUCKETS; i++) {
            countLeft[i] = countLeft[i - 1] + buckets[i].count;
            boundsLeft[i] = boundsLeft[i - 1];
            boundsLeft[i].expand(buckets[i].bounds);
        }

        // Compute costs sweeping from right
        int countRight = 0;
        AABB boundsRight;

        for (int i = SAH_BUCKETS - 1; i > 0; i--) {
            countRight += buckets[i].count;
            boundsRight.expand(buckets[i].bounds);

            int countL = countLeft[i - 1];
            if (countL == 0 || countRight == 0) {
                continue;
            }

            float saLeft = boundsLeft[i - 1].surfaceArea();
            float saRight = boundsRight.surfaceArea();
            float saParent = nodeBounds.surfaceArea();

            if (saParent < 1e-6f) {
                continue;
            }

            float cost = TRAVERSAL_COST
                + INTERSECTION_COST * (countL * saLeft + countRight * saRight)
                    / saParent;

            if (cost < best.cost && cost < leafCost) {
                best.cost = cost;
                best.axis = axis;
                best.position = axisMin
                    + axisExtent * static_cast<float>(i) / SAH_BUCKETS;
                best.splitIndex = start + countL;
            }
        }
    }

    return best;
}
