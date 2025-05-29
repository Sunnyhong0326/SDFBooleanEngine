#pragma once
#include <vector>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <memory>
#include "sdf/SDFNode.hpp"
#include "sdf/CSGTree.hpp"
#include "core/MarchingCube.hpp"

class SDFSceneGL {
public:
    SDFSceneGL();
    ~SDFSceneGL();

    void uploadScene(const std::vector<SDFNode>& nodes);
    void bindScene(GLuint ssboBinding = 0) const;

    // Raymarch CSG Tree
    void setupQuad();
    void setRaymarchUniforms(GLuint shaderID, const glm::vec3& camPos, const glm::mat4& invViewProj, const glm::vec2& iResolution);
    void renderQuad() const;

    // AABB
    void setupCSGTreeAABB(const std::unique_ptr<CSGTree>& csg);
    void setCSGTreeAABBUniforms(GLuint shaderID, const glm::mat4& viewProjMat);
    void renderCSGTreeAABB() const;

    // Marching cube voxels
    void setupMCVoxel(std::vector<GridCell> voxels);
    void setMCVoxelUniforms(GLuint shaderID, const glm::mat4& viewProjMat);
    void renderMCVoxel() const;

private:
    // CSG Tree
    GLuint ssbo = 0;
    GLuint quadVAO = 0, quadVBO = 0;
    // CSG AABB lines
    GLuint aabbVAO = 0, aabbVBO = 0;
    int AABBLineCount = 0;
    glm::vec3 lineColor = glm::vec3(1.0, 1.0, 0.0);
    // Marching cube voxel grid AABB
    GLuint voxelVAO = 0, voxelModelVBO = 0;
    std::vector<glm::mat4> voxelModels;
    int voxelLineCount = 0;
};