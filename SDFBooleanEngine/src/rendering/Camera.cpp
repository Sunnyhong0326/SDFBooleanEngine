#include "rendering/Camera.hpp"
//#include <glm/gtc/constants.hpp>
#include <GLFW/glfw3.h>

Camera::Camera(glm::vec3 pos) : position(pos), yaw(-90.0f), pitch(0.0f), worldUp(0.0f, 1.0f, 0.0f) {
    updateVectors();
}

glm::mat4 Camera::getViewMatrix() {
    return glm::lookAt(position, position + front, worldUp);
}

glm::mat4 Camera::getInverseViewProj() {
    glm::mat4 view = getViewMatrix();
    glm::mat4 proj = glm::perspective(glm::radians(fov), aspect, 0.1f, 100.0f);
    return glm::inverse(proj * view);
}

void Camera::processKeyboard(bool* keys, float dt) {
    float v = speed * dt;
    if (keys[GLFW_KEY_W]) position += front * v;
    if (keys[GLFW_KEY_S]) position -= front * v;
    if (keys[GLFW_KEY_A]) position -= right * v;
    if (keys[GLFW_KEY_D]) position += right * v;
}

void Camera::processMouse(float xoffset, float yoffset) {
    yaw += xoffset * sensitivity;
    pitch -= yoffset * sensitivity;
    pitch = glm::clamp(pitch, -89.0f, 89.0f);
    updateVectors();
}

void Camera::updateVectors() {
    front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    front.y = sin(glm::radians(pitch));
    front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    front = glm::normalize(front);
    right = glm::normalize(glm::cross(front, worldUp));
    up = glm::normalize(glm::cross(right, front));
}

glm::vec3 Camera::getRayDir(float xNDC, float yNDC) {
    glm::vec3 ray = glm::normalize(front + xNDC * right + yNDC * up);
    return ray;
}
