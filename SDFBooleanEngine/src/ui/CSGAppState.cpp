#include "ui/CSGAppUI.h"
#include "ui/CSGAppState.h" // Your application logic

#include <imgui.h>

CSGAppUI::CSGAppUI(CSGAppState& state) : state_(state) {
    strncpy(meshSavePath_, "output/mesh.ply", sizeof(meshSavePath_));
}

void CSGAppUI::draw() {
    ImGui::Begin("CSG SDF Controller");

    drawLoadCSGSection();
    drawExtractMeshSection();
    drawSaveMeshSection();
    ImGui::Separator();
    drawVisualizationToggles();

    ImGui::End();
}

void CSGAppUI::drawLoadCSGSection() {
    if (ImGui::Button("Load CSG JSON")) {
        std::string path = openFileDialog(".json"); // Implement this yourself
        if (!path.empty()) {
            state_.loadCSGFromJSON(path);
        }
    }
}

void CSGAppUI::drawExtractMeshSection() {
    if (ImGui::Button("Extract Mesh (Marching Cubes)")) {
        state_.extractMeshFromCSG();
    }
}

void CSGAppUI::drawSaveMeshSection() {
    ImGui::InputText("Mesh Save Path", meshSavePath_, IM_ARRAYSIZE(meshSavePath_));
    if (ImGui::Button("Save Mesh to .ply")) {
        state_.saveMeshToPLY(meshSavePath_);
    }
}

void CSGAppUI::drawVisualizationToggles() {
    ImGui::Checkbox("Show CSG AABB Wireframe", &state_.showAABB);
    ImGui::Checkbox("Render Mesh (instead of Raymarch)", &state_.useMeshRenderer);
}
