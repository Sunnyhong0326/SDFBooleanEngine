#include "sdf/SDFSceneGL.hpp"
#include "sdf/SDFEvaluator.hpp"
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

SDFSceneGL::SDFSceneGL() {}

SDFSceneGL::~SDFSceneGL() {
    if (ssbo) glDeleteBuffers(1, &ssbo);
    if (quadVBO) glDeleteBuffers(1, &quadVBO);
    if (quadVAO) glDeleteVertexArrays(1, &quadVAO);
}

void SDFSceneGL::uploadScene(const std::vector<SDFNode>& nodes) {
    if (ssbo == 0) glGenBuffers(1, &ssbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(SDFNode) * nodes.size(), nodes.data(), GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssbo);
}

void SDFSceneGL::bindScene(GLuint binding) const {
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, binding, ssbo);
}

void SDFSceneGL::setupQuad() {
    float quad[] = {
        -1.0f, -1.0f,
         1.0f, -1.0f,
        -1.0f,  1.0f,
         1.0f,  1.0f
    };

    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);
    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quad), quad, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);
}

void SDFSceneGL::setRaymarchUniforms(GLuint shaderID, const glm::vec3& camPos, const glm::mat4& invViewProj, const glm::vec2& iResolution) {
    glUseProgram(shaderID);
    glUniform3fv(glGetUniformLocation(shaderID, "camPos"), 1, &camPos[0]);
    glUniformMatrix4fv(glGetUniformLocation(shaderID, "invViewProj"), 1, GL_FALSE, &invViewProj[0][0]);
    glUniform2f(glGetUniformLocation(shaderID, "iResolution"), iResolution.x, iResolution.y);
}

void SDFSceneGL::renderQuad() const {
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}

void SDFSceneGL::setupCSGTreeAABB(const std::unique_ptr<CSGTree>& csgTree) {
    glGenVertexArrays(1, &aabbVAO);
    glGenBuffers(1, &aabbVBO);

    int rootIndex = csgTree->getNumNodes() - 1;
    AABB aabb = csgTree->computeAABB(rootIndex);
    std::vector<glm::vec3> aabbLines = getAABBLines(aabb);
    AABBLineCount = aabbLines.size();

    glBindVertexArray(aabbVAO);
    glBindBuffer(GL_ARRAY_BUFFER, aabbVBO);
    glBufferData(GL_ARRAY_BUFFER, aabbLines.size() * sizeof(glm::vec3), aabbLines.data(), GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
    glBindVertexArray(0);
}

void SDFSceneGL::setCSGTreeAABBUniforms(GLuint shaderID, const glm::mat4& viewProjMat) {
    glUniformMatrix4fv(glGetUniformLocation(shaderID, "uModel"), 1, GL_FALSE, glm::value_ptr(glm::mat4(1.0f)));
    glUniformMatrix4fv(glGetUniformLocation(shaderID, "uViewProj"), 1, GL_FALSE, glm::value_ptr(viewProjMat));
    glUniform3fv(glGetUniformLocation(shaderID, "color"), 1, glm::value_ptr(glm::vec3(1.0, 1.0, 0.0)));
}

void SDFSceneGL::renderCSGTreeAABB() const {
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glDisable(GL_DEPTH_TEST);

    glBindVertexArray(aabbVAO);
    glDrawArrays(GL_LINES, 0, AABBLineCount);
    glBindVertexArray(0);

    glEnable(GL_DEPTH_TEST);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void SDFSceneGL::setupMCVoxel(std::vector<GridCell> grid) {
    voxelModels.clear();
    voxelModels.reserve(grid.size());

    for (const GridCell& cell : grid) {
        glm::vec3 minCorner = cell.position[0];
        glm::vec3 maxCorner = cell.position[6];
        glm::vec3 scale = maxCorner - minCorner;
        glm::vec3 center = 0.5f * (minCorner + maxCorner);

        glm::mat4 model = glm::translate(glm::mat4(1.0f), center);
        model = glm::scale(model, scale);
        voxelModels.push_back(model);
    }

    glGenVertexArrays(1, &voxelVAO);
    glBindVertexArray(voxelVAO);
    // Create or update model matrix buffer
    glGenBuffers(1, &voxelModelVBO);
    glBindBuffer(GL_ARRAY_BUFFER, voxelModelVBO);
    glBufferData(GL_ARRAY_BUFFER, voxelModels.size() * sizeof(glm::mat4), voxelModels.data(), GL_STATIC_DRAW);

    // Setup instancing attributes
    glBindBuffer(GL_ARRAY_BUFFER, voxelModelVBO);
    std::size_t vec4Size = sizeof(glm::vec4);
    for (int i = 0; i < 4; ++i) {
        glEnableVertexAttribArray(1 + i);
        glVertexAttribPointer(1 + i, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(i * vec4Size));
        glVertexAttribDivisor(1 + i, 1);
    }
    glBindVertexArray(0);
}

void SDFSceneGL::setMCVoxelUniforms(GLuint shaderID, const glm::mat4& viewProjMat) {
    glUniformMatrix4fv(glGetUniformLocation(shaderID, "uViewProj"), 1, GL_FALSE, glm::value_ptr(viewProjMat));
    glUniform3fv(glGetUniformLocation(shaderID, "color"), 1, glm::value_ptr(glm::vec3(1.0, 0.0, 0.0)));
}

void SDFSceneGL::renderMCVoxel() const {
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glDisable(GL_DEPTH_TEST);

    glBindVertexArray(voxelVAO);
    glDrawArraysInstanced(GL_LINES, 0, 24, voxelModels.size());  // One draw call
    glBindVertexArray(0);

    glEnable(GL_DEPTH_TEST);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}