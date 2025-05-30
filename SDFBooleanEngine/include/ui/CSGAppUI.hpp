#pragma once
#include <string>
#include "ui/CSGAppState.hpp"

class CSGAppUI {
public:
    explicit CSGAppUI(CSGAppState& state);

    void draw();

private:
    CSGAppState& state_;
    char meshSavePath_[512] = "output/mesh.ply";

    void drawLoadCSGSection();
    void drawExtractMeshSection();
    void drawSaveMeshSection();
    void drawVisualizationToggles();
};
