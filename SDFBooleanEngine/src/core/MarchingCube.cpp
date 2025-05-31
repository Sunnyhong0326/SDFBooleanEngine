/*
    Tables and conventions from
    http://paulbourke.net/geometry/polygonise/
*/

#include <iostream>
#include "core/MarchingCube.hpp"

std::vector<GridCell> MarchingCubes::sampleGrid(CSGTree* tree, int rootIdx, const AABB& box, glm::ivec3 resolution) {
    std::vector<GridCell> grid;
    glm::vec3 min = box.min - glm::vec3(0.5f, 0.5f, 0.5f);
    glm::vec3 max = box.max + glm::vec3(0.5f, 0.5f, 0.5f);
    glm::vec3 size = max - min;
    glm::vec3 step = size / glm::vec3(resolution - glm::ivec3(1));

    const glm::ivec3 offsets[8] = {
        {0, 0, 0}, {1, 0, 0}, {1, 1, 0}, {0, 1, 0},
        {0, 0, 1}, {1, 0, 1}, {1, 1, 1}, {0, 1, 1}
    };

    for (int z = 0; z < resolution.z - 1; ++z)
        for (int y = 0; y < resolution.y - 1; ++y)
            for (int x = 0; x < resolution.x - 1; ++x) {
                GridCell cell;
                for (int i = 0; i < 8; ++i) {
                    glm::ivec3 corner = glm::ivec3(x, y, z) + offsets[i];
                    glm::vec3 pos = min + glm::vec3(corner) * step;
                    cell.position[i] = pos;
                    cell.value[i] = tree->evalSDF(rootIdx, pos);
                }
                grid.push_back(cell);
            }
    return grid;
}
glm::vec3 MarchingCubes::interpolateVertex(float iso, const glm::vec3& p1, const glm::vec3& p2, float val1, float val2) {
    float t = (iso - val1) / (val2 - val1);
    return p1 + t * (p2 - p1);
}


std::vector<Triangle> MarchingCubes::run(const std::vector<GridCell>& grid, float isoLevel) {
    std::vector<Triangle> triangles;

    static const int edgeConnection[12][2] = {
        {0, 1}, {1, 2}, {2, 3}, {0, 3},
        {4, 5}, {5, 6}, {6, 7}, {4, 7},
        {0, 4}, {1, 5}, {2, 6}, {3, 7}
    };

    for (const auto& cell : grid) {
        int cubeIndex = 0;
        for (int i = 0; i < 8; ++i) {
            if (cell.value[i] < isoLevel)
                cubeIndex |= (1 << i);
        }

        int edgeMask = edgeTable[cubeIndex];
        glm::vec3 vertList[12];

        for (int i = 0; i < 12; ++i) {
            if (edgeMask & (1 << i)) {
                int a = edgeConnection[i][0];
                int b = edgeConnection[i][1];
                vertList[i] = interpolateVertex(isoLevel, cell.position[a], cell.position[b], cell.value[a], cell.value[b]);
            }
        }

        for (int i = 0; triangleTable[cubeIndex][i] != -1; i += 3) {
            Triangle tri;

            tri.v0 = vertList[triangleTable[cubeIndex][i]];
            tri.v1 = vertList[triangleTable[cubeIndex][i + 2]];
            tri.v2 = vertList[triangleTable[cubeIndex][i + 1]];

            glm::vec3 edge1 = tri.v1 - tri.v0;
            glm::vec3 edge2 = tri.v2 - tri.v0;
            glm::vec3 faceNormal = glm::normalize(glm::cross(edge1, edge2));
            tri.normal = faceNormal;

            triangles.push_back(tri);
        }
    }
    return triangles;
}