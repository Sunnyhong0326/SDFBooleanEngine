#pragma once
#include <string>

struct CSGAppState {
    bool showAABB = false;
    bool useMeshRenderer = false;

    void loadCSGFromJSON(const std::string& path);
    void extractMeshFromCSG();
    void saveMeshToPLY(const std::string& path);
};
