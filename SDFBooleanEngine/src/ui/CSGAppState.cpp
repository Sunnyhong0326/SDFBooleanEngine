#include "ui/CSGAppState.hpp"
#include "core/MeshUtils.hpp"


void CSGAppState::loadCSGFromJSON(const std::string& path) {
    if (!csgTree || !sdfScene) return;
    csgTree->loadNodesFromJson(path);
    sdfScene->uploadScene(csgTree->getNodes());
    sdfScene->setupCSGTreeAABB(csgTree);
    loadJson = true;
}

void CSGAppState::requestExtractMeshAsync() {
    if (!loadJson || isExtracting || !marchingCubes || !csgTree || !sdfScene) return;

    isExtracting = true;
    hasExtracted = false;

    extractThread = std::thread([this]() {
        int rootIndex = csgTree->getNumNodes() - 1;
        AABB bbox = csgTree->computeAABB(rootIndex);
        auto grid = marchingCubes->sampleGrid(csgTree, rootIndex, bbox, gridResolution);
        auto tris = marchingCubes->run(grid, 0.0f);

        {
            std::lock_guard<std::mutex> lock(meshMutex);
            extractedTris = std::move(tris);
        }

        isExtracting = false;
        hasExtracted = true;
    });
    onMeshExtracted = [this]() {
        std::lock_guard<std::mutex> lock(meshMutex);
        sdfScene->uploadMesh(extractedTris);
        std::cout << "Uploading extracted mesh, triangle count: " << extractedTris.size() << std::endl;
    };
    //sdfScene->setupMCVoxel(grid);
    /*std::cout << "triangle count" << extractedTris.size() << std::endl;
    sdfScene->uploadMesh(extractedTris);*/
}

void CSGAppState::joinBackgroundThread() {
    if (extractThread.joinable()) {
        extractThread.join();
    }
}

void CSGAppState::saveMeshToPLY(const std::string& path) {
    std::lock_guard<std::mutex> lock(meshMutex);
    if (!extractedTris.empty()) {
        exportToPLY(path, extractedTris);
    }
}
