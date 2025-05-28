#pragma once
#include <vector>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include "sdf/SDFNode.hpp"

class SDFSceneGL {
public:
    SDFSceneGL();
    ~SDFSceneGL();

    void uploadScene(const std::vector<SDFNode>& nodes);
    void bindScene(GLuint ssboBinding = 0) const;

    void setupQuad();
    void renderQuad() const;

    void setUniforms(GLuint shaderID, const glm::vec3& camPos, const glm::mat4& invViewProj, const glm::vec2& iResolution);

private:
    GLuint ssbo = 0;
    GLuint quadVAO = 0, quadVBO = 0;
};