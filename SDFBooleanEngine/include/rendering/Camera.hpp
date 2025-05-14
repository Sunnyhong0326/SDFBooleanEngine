#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <GLFW/glfw3.h>

enum CameraMode {FPS, ORBIT};

class Camera {
public:
    float fov = 45.0f;
    float aspect = 800.0f / 600.0f;
    glm::vec3 position;
    glm::vec3 target;
    glm::vec3 front, up, right;
    glm::vec3 worldUp = glm::vec3(0.0f, 1.0f, 0.0f);

    float radius = 6.0f;
    float theta = glm::half_pi<float>(); // yaw angle (around Y axis)
    float phi = glm::half_pi<float>(); // pitch angle from Y axis

    float moveSpeed = 3.0f;
    float orbitSensitivity = 0.3f;
    float panSensitivity = 0.01f;

    float zoomSpeed = 1.0f;

    float minRadius = 0.5f;
    float maxRadius = 50.0f;

    Camera(glm::vec3 camPos, glm::vec3 targetPos = glm::vec3(0.0f), float dist = 6.0f);


    void processMouseDrag(float dx, float dy);

    void processKeyboard(bool* keys, float dt);

    void processScroll(float yoffset);

    void processPan(float dx, float dy);

    void updateAspectRatio(float);

    glm::mat4 getViewMatrix() const;

    glm::mat4 getInverseViewProj();

private:
    void updatePositionFromSpherical();

    void updateVectors();

};
