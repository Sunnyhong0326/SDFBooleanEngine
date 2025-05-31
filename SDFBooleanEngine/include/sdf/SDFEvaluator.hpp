#pragma once
#include <glm/glm.hpp>

struct AABB {
    glm::vec3 min;
    glm::vec3 max;
};

inline AABB boxAABB(const glm::vec3& center, const glm::vec3& half_size) {
    return { center - half_size, center + half_size };
}

inline AABB sphereAABB(const glm::vec3& center, float radius) {
    return { center - glm::vec3(radius), center + glm::vec3(radius) };
}

inline AABB torusAABB(glm::vec3 center, glm::vec2 t) {
    float R = t.x, r = t.y;
    glm::vec3 extent = glm::vec3(R + r);
    extent.y = r;
    return { center - extent, center + extent };
}

inline AABB cylinderAABB(glm::vec3 center, float height, float radius) {
    glm::vec3 extent = glm::vec3(radius, height, radius);
    return { center - extent, center + extent };
}

inline AABB coneAABB(glm::vec3 center, glm::vec2 sincos, float height) {
    // Radius at base = height * tan(£c) = height * (sin/cos) = height * sincos.x / sincos.y
    float radius = height * sincos.x / sincos.y;
    glm::vec3 extent = glm::vec3(radius, height, radius);

    return { center - extent, center + extent };
}

inline AABB capsuleAABB(glm::vec3 a, glm::vec3 b, float r) {
    glm::vec3 min = glm::min(a, b) - glm::vec3(r);
    glm::vec3 max = glm::max(a, b) + glm::vec3(r);
    return { min, max };
}


inline AABB unionAABB(const AABB& a, const AABB& b) {
    return { glm::min(a.min, b.min), glm::max(a.max, b.max) };
}

inline AABB intersectAABB(const AABB& a, const AABB& b) {
    return { glm::max(a.min, b.min), glm::min(a.max, b.max) };
}

inline AABB subtractAABB(const AABB& a, const AABB& b) {
    return a; // Conservative
}

inline AABB triPrismAABB(const glm::vec3& center, const glm::vec2& size) {
    float base_half = size.x * 0.9f;
    float tri_height_y = size.x * 0.9f / 0.866025f; 
    float z_half = size.y;

    glm::vec3 half_extent(base_half, tri_height_y, z_half);
    return { center - half_extent, center + half_extent };
}

inline std::vector<glm::vec3> getAABBLines(const AABB& box)
{
    glm::vec3 min = box.min;
    glm::vec3 max = box.max;

    glm::vec3 corners[8] = {
        {min.x, min.y, min.z},
        {max.x, min.y, min.z},
        {max.x, max.y, min.z},
        {min.x, max.y, min.z},
        {min.x, min.y, max.z},
        {max.x, min.y, max.z},
        {max.x, max.y, max.z},
        {min.x, max.y, max.z}
    };

    std::vector<glm::vec3> lines = {
        corners[0], corners[1],
        corners[1], corners[2],
        corners[2], corners[3],
        corners[3], corners[0],

        corners[4], corners[5],
        corners[5], corners[6],
        corners[6], corners[7],
        corners[7], corners[4],

        corners[0], corners[4],
        corners[1], corners[5],
        corners[2], corners[6],
        corners[3], corners[7],
    };

    return lines;
}