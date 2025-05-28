#pragma once
#include <glad/glad.h>
#include <string>
#include <vector>
#include "sdf/SDFNode.hpp"
class Shader {
public:
    unsigned int ID;
    Shader(const char* vertexPath, const char* fragmentPath);
    void checkCompileErrors(GLuint shader, std::string type);
    void use();
};
