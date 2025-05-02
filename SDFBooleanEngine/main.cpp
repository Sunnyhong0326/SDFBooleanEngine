#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "rendering/Shader.hpp"
#include "rendering/Camera.hpp"

Camera cam(glm::vec3(0, 0, 3));
bool keys[1024];
float lastX = 400, lastY = 300;
bool firstMouse = true;

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}
// Input callbacks
void key_callback(GLFWwindow* w, int key, int scancode, int action, int mods) {
    if (key >= 0 && key < 1024) {
        keys[key] = action != GLFW_RELEASE;
    }
}

void mouse_callback(GLFWwindow* w, double xpos, double ypos) {
    if (firstMouse) { lastX = xpos; lastY = ypos; firstMouse = false; }
    float dx = xpos - lastX;
    float dy = ypos - lastY;
    lastX = xpos;
    lastY = ypos;
    cam.processMouse(dx, dy);
}

int main() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(800, 600, "SDF Raymarching", NULL, NULL);
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetKeyCallback(window, key_callback);
    glfwSetCursorPosCallback(window, mouse_callback);

    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

    Shader shader("shaders/raymarch.vert", "shaders/raymarch.frag");

    float quad[] = {
        -1.0f, -1.0f,
         1.0f, -1.0f,
        -1.0f,  1.0f,
         1.0f,  1.0f,
    };

    GLuint VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quad), quad, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    float lastFrame = glfwGetTime();
    while (!glfwWindowShouldClose(window)) {
        float time = glfwGetTime();
        float dt = time - lastFrame;
        lastFrame = time;

        cam.processKeyboard(keys, dt);

        shader.use();
        glUniform3fv(glGetUniformLocation(shader.ID, "camPos"), 1, &cam.position[0]);
        glm::mat4 invVP = cam.getInverseViewProj();
        glUniformMatrix4fv(glGetUniformLocation(shader.ID, "invViewProj"), 1, GL_FALSE, &invVP[0][0]);
        glUniform2f(glGetUniformLocation(shader.ID, "iResolution"), 800.0f, 600.0f);

        // Send camera basis for ray direction in fragment shader
        /*glUniform3fv(glGetUniformLocation(shader.ID, "camFront"), 1, &cam.getRayDir(0, 0)[0]);
        glUniform3fv(glGetUniformLocation(shader.ID, "camRight"), 1, &cam.getRayDir(1, 0)[0]);
        glUniform3fv(glGetUniformLocation(shader.ID, "camUp"), 1, &cam.getRayDir(0, 1)[0]);*/
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
