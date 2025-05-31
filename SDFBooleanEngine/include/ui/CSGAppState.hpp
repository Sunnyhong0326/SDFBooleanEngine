#pragma once
#include <string>
#include <vector>
#include <thread>
#include <atomic>
#include <mutex>

#include "sdf/CSGTree.hpp"
#include "core/MarchingCube.hpp"
#include "sdf/SDFSceneGL.hpp"

struct CSGAppState {
    bool showAABB = false;
    bool useMeshRenderer = false;
    bool loadJson = false;

    CSGTree* csgTree = nullptr;
    MarchingCubes* marchingCubes = nullptr;
    SDFSceneGL* sdfScene = nullptr;

    std::vector<Triangle> extractedTris;
    std::mutex meshMutex;

    glm::ivec3 gridResolution = glm::ivec3(128, 128, 128);
    std::atomic<bool> isExtracting = false;
    std::atomic<bool> hasExtracted = false;
    std::thread extractThread;
    std::function<void()> onMeshExtracted = nullptr;

    void loadCSGFromJSON(const std::string& path);
    void requestExtractMeshAsync();
    void saveMeshToPLY(const std::string& path);
    void joinBackgroundThread(); 
};
