#include "sdf/SDFSceneGL.hpp"
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

void SDFSceneGL::renderQuad() const {
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}

void SDFSceneGL::setUniforms(GLuint shaderID, const glm::vec3& camPos, const glm::mat4& invViewProj, const glm::vec2& iResolution) {
    glUseProgram(shaderID);
    glUniform3fv(glGetUniformLocation(shaderID, "camPos"), 1, &camPos[0]);
    glUniformMatrix4fv(glGetUniformLocation(shaderID, "invViewProj"), 1, GL_FALSE, &invViewProj[0][0]);
    glUniform2f(glGetUniformLocation(shaderID, "iResolution"), iResolution.x, iResolution.y);
}
