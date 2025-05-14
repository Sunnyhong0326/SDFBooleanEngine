#include "rendering/Camera.hpp"
#include <GLFW/glfw3.h>

Camera::Camera(glm::vec3 camPos, glm::vec3 targetPos, float dist) {
    position = glm::vec3(0, 0, 6);
    target = glm::vec3(0.0f);
    radius = 6;
    updatePositionFromSpherical();
}

void Camera::processMouseDrag(float dx, float dy) {
    theta += glm::radians(dx * orbitSensitivity);
    phi -= glm::radians(dy * orbitSensitivity);

    // Clamp phi to avoid flipping
    phi = glm::clamp(phi, 0.01f, glm::pi<float>() - 0.01f);

    updatePositionFromSpherical();
}

void Camera::processKeyboard(bool* keys, float dt) {
    glm::vec3 camRight = glm::normalize(glm::cross(front, worldUp));
    glm::vec3 camUp = glm::normalize(glm::cross(camRight, front));

    glm::vec3 moveDir(0.0f);
    float v = moveSpeed * dt;
    if (keys[GLFW_KEY_W]) moveDir += front * v;
    if (keys[GLFW_KEY_S]) moveDir -= front * v;
    if (keys[GLFW_KEY_E]) moveDir += camUp * v;
    if (keys[GLFW_KEY_Q]) moveDir -= camUp * v;
    if (keys[GLFW_KEY_A]) moveDir -= camRight * v;
    if (keys[GLFW_KEY_D]) moveDir += camRight * v;

    position += moveDir;
    target += moveDir;

    updateVectors();
}

void Camera::processScroll(float yoffset) {
    radius -= yoffset * zoomSpeed;
    radius = glm::clamp(radius, minRadius, maxRadius);
    updatePositionFromSpherical();
}

void Camera::processPan(float dx, float dy) {
    // Scale mouse movement by radius to get consistent pan speed
    float panSpeed = panSensitivity * radius * 0.1f;

    glm::vec3 panRight = glm::normalize(glm::cross(front, worldUp));
    glm::vec3 panUp = glm::normalize(glm::cross(panRight, front));

    glm::vec3 movement = -dx * panRight * panSpeed + dy * panUp * panSpeed;

    position += movement;
    target += movement;

    updateVectors();
}

void Camera::updatePositionFromSpherical() {
    position.x = target.x + radius * sin(phi) * cos(theta);
    position.y = target.y + radius * cos(phi);
    position.z = target.z + radius * sin(phi) * sin(theta);

    updateVectors();
}

void Camera::updateVectors() {
    front = glm::normalize(target - position);
    right = glm::normalize(glm::cross(front, worldUp));
    up = glm::normalize(glm::cross(right, front));
}

void Camera::updateAspectRatio(float aspectRatio) {
    aspect = aspectRatio;
}

glm::mat4 Camera::getViewMatrix() const {
    return glm::lookAt(position, target, up);
}

glm::mat4 Camera::getInverseViewProj() {
    glm::mat4 view = getViewMatrix();
    glm::mat4 proj = glm::perspective(glm::radians(fov), aspect, 0.1f, 100.0f);
    return glm::inverse(proj * view);
}