#pragma once
#include <vector>
#include <glm/glm.hpp>
#include "sdf/SDFNode.hpp"
#include "sdf/CSGTree.hpp"
#include "sdf/SDFEvaluator.hpp"

class CSGTree {
public:
    CSGTree() = default;

    int addSphere(glm::vec3 center, float radius, glm::vec3 color) {
        return addNode(SPHERE, -1, -1, center, glm::vec4(radius, 0.0f, 0.0f, 0.0f), color);
    }

    int addBox(glm::vec3 center, glm::vec3 size, glm::vec3 color) {
        return addNode(BOX, -1, -1, center, glm::vec4(size, 0.0f), color);
    }

    int addPlane(glm::vec3 center, glm::vec3 normal, float offset, glm::vec3 color) {
        return addNode(PLANE, -1, -1, center, glm::vec4(normal, offset), color);
    }

    int addTriPrism(const glm::vec3& center, float baseWidth, float depth, const glm::vec3& color) {
        return addNode(TRI_PRISM, -1, -1, center, glm::vec4(baseWidth, depth, 0.0f, 0.0f), color);
    }

    int unionOp(int a, int b, glm::vec3 color = glm::vec3(1.0f)) {
        return addNode(UNION, a, b, glm::vec3(0), glm::vec4(0), color);
    }

    int intersect(int a, int b, glm::vec3 color = glm::vec3(1.0f)) {
        return addNode(INTERSECT, a, b, glm::vec3(0), glm::vec4(0), color);
    }

    int subtract(int a, int b, glm::vec3 color = glm::vec3(1.0f)) {
        return addNode(SUBTRACT, a, b, glm::vec3(0), glm::vec4(0), color);
    }

    AABB computeAABB (int idx) const;

    float evalSDF(int idx, const glm::vec3& p) const;

    const std::vector<SDFNode>& getNodes() const {
        return nodes;
    }

    const int getNumNodes() const {
        return nodes.size();
    }

    const void setNodes(const std::vector<SDFNode> preConstruceNodes) {
        nodes = preConstruceNodes;
    }

private:
    std::vector<SDFNode> nodes;

    int addNode(int type, int left, int right, glm::vec3 param1, glm::vec4 param2, glm::vec3 color);

};
