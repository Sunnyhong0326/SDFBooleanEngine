#include "sdf/CSGTree.hpp"
#include <algorithm>

int CSGTree::addNode(int type, int left, int right, glm::vec3 param1, glm::vec4 param2, glm::vec3 color) {
    SDFNode node;
    node.type = type;
    node.left = left;
    node.right = right;
    node.param1 = glm::vec4(param1, 0.0f);
    node.param2 = param2;
    node.color = glm::vec4(color, 0.0f);
    nodes.push_back(node);
    return nodes.size() - 1;
}

void CSGTree::loadNodesFromJson(const std::string& filename) {
    std::ifstream in(filename);
    json j;
    in >> j;

    nodes.clear();

    for (const auto& item : j["nodes"]) {
        SDFNode node;
        node.type = item["type"];
        node.left = item["left"];
        node.right = item["right"];
        node.pad = 0;

        auto p1 = item["param1"];
        node.param1 = glm::vec4(p1[0], p1[1], p1[2], p1[3]);

        auto p2 = item["param2"];
        node.param2 = glm::vec4(p2[0], p2[1], p2[2], p2[3]);

        auto col = item["color"];
        node.color = glm::vec3(col[0], col[1], col[2]);
        node.pad0 = 0;

        nodes.push_back(node);
    }
}


AABB CSGTree::computeAABB(int idx) const {
    const SDFNode& node = nodes[idx];
    if (node.type == SPHERE) return sphereAABB(node.param1, node.param2.x);
    if (node.type == BOX) return boxAABB(node.param1, node.param2);
    if (node.type == PLANE) return { glm::vec3(-10), glm::vec3(10) }; // Large AABB
    if (node.type == TRI_PRISM) return triPrismAABB(node.param1, glm::vec2(node.param2));

    AABB a = computeAABB(node.left);
    AABB b = computeAABB(node.right);

    if (node.type == UNION) return unionAABB(a, b);
    if (node.type == INTERSECT) return intersectAABB(a, b);
    if (node.type == SUBTRACT) return subtractAABB(a, b);
    return { glm::vec3(0), glm::vec3(0) };
}

float CSGTree::evalSDF(int idx, const glm::vec3& p) const {
    const SDFNode& node = nodes[idx];

    glm::vec3 center = glm::vec3(node.param1);
    glm::vec3 size = glm::vec3(node.param2);

    if (node.type == SPHERE) {
        return glm::length(p - center) - size.x;
    }
    if (node.type == BOX) {
        glm::vec3 q = glm::abs(p - center) - size;
        float max_component = std::max(std::max(q.x, q.y), q.z);
        return glm::length(glm::max(q, glm::vec3(0.0f))) + std::min(max_component, 0.0f);
    }
    if (node.type == PLANE) {
        return glm::dot(p - center, size);
    }
    if (node.type == TRI_PRISM) {
        glm::vec3 local = p - center;
        glm::vec3 q = glm::abs(local);
        float h_x = size.x, h_y = size.y;
        float k = std::max(q.x * 0.866025f + local.y * 0.5f, -local.y) - h_x * 0.5f;
        return std::max(q.z - h_y, k);
    }

    float da = evalSDF(node.left, p);
    float db = evalSDF(node.right, p);

    if (node.type == UNION) return std::min(da, db);
    if (node.type == INTERSECT) return std::max(da, db);
    if (node.type == SUBTRACT) return std::max(da, -db);

    return 0.0f;
}