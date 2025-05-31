#include "ui/CSGAppUI.hpp"
#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>
#include <iostream>
#include <cstring>

CSGAppUI::CSGAppUI(CSGAppState& state) : state_(state) {
    strcpy_s(meshSavePath_, sizeof(meshSavePath_), "output/mesh.ply");
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
        std::string path = openFileDialog("JSON Files (*.json)\0*.json\0");
        if (!path.empty()) {
            state_.loadCSGFromJSON(path);
        }
    }
}

void CSGAppUI::drawExtractMeshSection() {
    if (!state_.loadJson)
        return;

    if (!state_.isExtracting) {
        if (ImGui::Button("Extract Mesh (Marching Cubes)")) {
            state_.requestExtractMeshAsync();
        }
    }
    else {
        ImGui::Text("Extracting mesh... (please wait)");
    }

    if (state_.hasExtracted) {
        std::lock_guard<std::mutex> lock(state_.meshMutex);
        ImGui::Text("Mesh ready: %zu triangles", state_.extractedTris.size());
    }
    ImGui::DragInt3("Res (X,Y,Z)", glm::value_ptr(state_.gridResolution), 1, 8, 512);

}

void CSGAppUI::drawSaveMeshSection() {
    if (!state_.loadJson || !state_.hasExtracted)
        return;

    if (ImGui::Button("Save Mesh to .ply")) {
        std::string path = saveFileDialog("PLY Files (*.ply)\0*.json\0");
        state_.saveMeshToPLY(path);
    }
}

void CSGAppUI::drawVisualizationToggles() {
    ImGui::Checkbox("Show CSG AABB Wireframe", &state_.showAABB);
    ImGui::Checkbox("Render Mesh (instead of Raymarch)", &state_.useMeshRenderer);
}
