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
    if (node.type == TORUS) return torusAABB(node.param1, glm::vec2(node.param2));  // param2.xy = (R, r)
    if (node.type == CYLINDER) return cylinderAABB(node.param1, node.param2.x, node.param2.y); // x = height, y = radius
    if (node.type == CONE) return coneAABB(node.param1, glm::vec2(node.param2), node.param2.z); // x = height, y = radius
    if (node.type == CAPSULE) return capsuleAABB(glm::vec3(node.param1), glm::vec3(node.param2), node.param2.w);

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
    glm::vec3 param = glm::vec3(node.param2);

    if (node.type == SPHERE) {
        return glm::length(p - center) - param.x;
    }

    if (node.type == BOX) {
        glm::vec3 q = glm::abs(p - center) - param;
        float max_component = std::max(std::max(q.x, q.y), q.z);
        return glm::length(glm::max(q, glm::vec3(0.0f))) + std::min(max_component, 0.0f);
    }

    if (node.type == PLANE) {
        return glm::dot(p - center, param);
    }

    if (node.type == TRI_PRISM) {
        glm::vec3 local = p - center;
        glm::vec3 q = glm::abs(local);
        float h_x = param.x, h_y = param.y;
        float k = std::max(q.x * 0.866025f + local.y * 0.5f, -local.y) - h_x * 0.5f;
        return std::max(q.z - h_y, k);
    }

    if (node.type == TORUS) {
        glm::vec3 local = p - center;
        glm::vec2 t = glm::vec2(param.x, param.y); // major radius, minor radius
        glm::vec2 q = glm::vec2(glm::length(glm::vec2(local.x, local.z)) - t.x, local.y);
        return glm::length(q) - t.y;
    }

    if (node.type == CYLINDER) {
        glm::vec3 local = p - center;
        float h = param.x, r = param.y;
        glm::vec2 d = glm::abs(glm::vec2(glm::length(glm::vec2(local.x, local.z)), local.y)) - glm::vec2(r, h);
        return std::min(std::max(d.x, d.y), 0.0f) + glm::length(glm::max(d, glm::vec2(0.0f)));
    }

    if (node.type == CONE) {
        glm::vec3 local = p - center;

        glm::vec2 c = glm::vec2(param.x, param.y);  // sin(angle), cos(angle)
        float h = param.z;

        glm::vec2 q = h * glm::vec2(c.x / c.y, -1.0f);
        glm::vec2 w = glm::vec2(glm::length(glm::vec2(local.x, local.z)), local.y);

        glm::vec2 a = w - q * glm::clamp(glm::dot(w, q) / glm::dot(q, q), 0.0f, 1.0f);
        glm::vec2 b = w - q * glm::vec2(glm::clamp(w.x / q.x, 0.0f, 1.0f), 1.0f);

        float k = glm::sign(q.y);
        float d = std::min(glm::dot(a, a), glm::dot(b, b));
        float s = std::max(k * (w.x * q.y - w.y * q.x), k * (w.y - q.y));

        return std::sqrt(d) * glm::sign(s);
    }

    if (node.type == CAPSULE) {
        glm::vec3 a = glm::vec3(node.param1);         // point a
        glm::vec3 b = glm::vec3(node.param2);         // point b
        float r = node.param2.w;                      // radius
        glm::vec3 pa = p - a, ba = b - a;
        float h = glm::clamp(glm::dot(pa, ba) / glm::dot(ba, ba), 0.0f, 1.0f);
        return glm::length(pa - h * ba) - r;
    }

    // CSG ops
    float da = evalSDF(node.left, p);
    float db = evalSDF(node.right, p);

    if (node.type == UNION) return std::min(da, db);
    if (node.type == INTERSECT) return std::max(da, db);
    if (node.type == SUBTRACT) return std::max(da, -db);

    return 0.0f;
}
