#pragma once
#include <vector>
#include <glm/glm.hpp>
#include "sdf/SDFNode.hpp"

class SDFBuilder {
public:
    SDFBuilder() = default;

    int addSphere(glm::vec3 center, float radius, glm::vec3 color) {
        return addNode(SPHERE, -1, -1, center, radius, color);
    }

    int addBox(glm::vec3 center, glm::vec3 size, glm::vec3 color) {
        return addNode(BOX, -1, -1, center, glm::vec4(size, 0.0f), color);
    }

    int addPlane(glm::vec3 center, glm::vec3 normal, float offset, glm::vec3 color) {
        return addNode(PLANE, -1, -1, center, glm::vec4(normal, offset), color);
    }

    int addTriPrism(const glm::vec3& center, float baseWidth, float depth, const glm::vec3& color) {
        SDFNode node;
        node.type = TRI_PRISM;
        node.left = node.right = -1;
        node.param1 = glm::vec4(center, 0.0f);
        node.param2 = glm::vec4(baseWidth, depth, 0.0f, 0.0f);
        node.color = color;
        nodes.push_back(node);
        return nodes.size() - 1;
    }

    int unionOp(int a, int b, glm::vec3 color = glm::vec3(1.0f)) {
        return addNode(UNION, a, b, glm::vec3(0), 0.0f, color);
    }

    int intersect(int a, int b, glm::vec3 color = glm::vec3(1.0f)) {
        return addNode(INTERSECT, a, b, glm::vec3(0), 0.0f, color);
    }

    int subtract(int a, int b, glm::vec3 color = glm::vec3(1.0f)) {
        return addNode(SUBTRACT, a, b, glm::vec3(0), 0.0f, color);
    }

    const std::vector<SDFNode>& getNodes() const {
        return nodes;
    }

private:
    std::vector<SDFNode> nodes;

    int addNode(int type, int left, int right, glm::vec3 param1, glm::vec4 param2, glm::vec3 color) {
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

    int addNode(int type, int left, int right, glm::vec3 param1, float param2, glm::vec3 color) {
        SDFNode node;
        node.type = type;
        node.left = left;
        node.right = right;
        node.param1 = glm::vec4(param1, 0.0f);
        node.param2 = glm::vec4(param2, 0.0f, 0.0f, 0.0f);
        node.color = glm::vec4(color, 0.0f);
        nodes.push_back(node);
        return nodes.size() - 1;
    }
};
