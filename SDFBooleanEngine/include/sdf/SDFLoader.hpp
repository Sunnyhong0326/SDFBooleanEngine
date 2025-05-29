#include <nlohmann/json.hpp>
#include <fstream>

using json = nlohmann::json;

std::vector<SDFNode> loadNodesFromJson(const std::string& filename) {
    std::ifstream in(filename);
    json j;
    in >> j;

    std::vector<SDFNode> nodes;

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

    return nodes;
}
