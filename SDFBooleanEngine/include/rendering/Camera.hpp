#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Camera {
public:
    glm::vec3 position;
    float yaw, pitch;
    float speed = 3.0f;
    float sensitivity = 0.1f;
    float fov = 45.0f;
    float aspect = 800.0f / 600.0f;

    Camera(glm::vec3 pos);
    glm::mat4 getViewMatrix();
    glm::mat4 getInverseViewProj();
    void processKeyboard(bool* keys, float dt);
    void processMouse(float xoffset, float yoffset);
    glm::vec3 getRayDir(float xNDC, float yNDC);
private:
    glm::vec3 front, up, right, worldUp;
    void updateVectors();
};
