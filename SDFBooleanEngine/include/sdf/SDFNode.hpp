#pragma once
#include <glm/glm.hpp>

enum SDFType { 
    SPHERE = 0, 
    BOX = 1, 
    PLANE = 2, 
    TRI_PRISM = 3,
    
    UNION = 10, 
    INTERSECT = 11, 
    SUBTRACT = 12
};

struct SDFNode {
    int type;
    int left;
    int right;
    int pad;

    glm::vec4 param1;    // xyz = center or size

    glm::vec4 param2;    // x = radius

    glm::vec3 color; // rgb = color
    float pad0;
};
