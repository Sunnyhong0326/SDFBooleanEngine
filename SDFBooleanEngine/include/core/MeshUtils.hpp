#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include "core/MarchingCube.hpp"

inline void exportToPLY(const std::string& filename, const std::vector<Triangle>& triangles) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Failed to open PLY file for writing: " << filename << std::endl;
        return;
    }

    size_t vertexCount = triangles.size() * 3;
    size_t faceCount = triangles.size();

    file << "ply\nformat ascii 1.0\n";
    file << "element vertex " << vertexCount << "\n";
    file << "property float x\nproperty float y\nproperty float z\n";
    file << "element face " << faceCount << "\n";
    file << "property list uchar int vertex_indices\n";
    file << "end_header\n";

    for (const auto& tri : triangles) {
        file << tri.v0.x << " " << tri.v0.y << " " << tri.v0.z << "\n";
        file << tri.v1.x << " " << tri.v1.y << " " << tri.v1.z << "\n";
        file << tri.v2.x << " " << tri.v2.y << " " << tri.v2.z << "\n";
    }

    for (size_t i = 0; i < faceCount; ++i) {
        file << "3 " << (i * 3 + 0) << " " << (i * 3 + 1) << " " << (i * 3 + 2) << "\n";
    }

    file.close();
    std::cout << "Exported PLY file: " << filename << std::endl;
}
