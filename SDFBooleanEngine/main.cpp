#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "rendering/Shader.hpp"
#include "rendering/Camera.hpp"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

Camera cam(glm::vec3(0, 0, -6));
bool keys[1024];
float lastX = 400, lastY = 300;
bool firstMouse = true;
bool leftMousePressed = false;
bool rightMousePressed = false;
float windowWidth = 800.f, windowHeight=600.f;

void framebufferSizeCallback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
    windowWidth = static_cast<float> (width);
    windowHeight = static_cast<float> (height);
    float aspect = static_cast<float>(width) / static_cast<float>(height);
    cam.updateAspectRatio(aspect);
}

void keyCallback(GLFWwindow* w, int key, int scancode, int action, int mods) {
    if (key >= 0 && key < 1024) {
        keys[key] = action != GLFW_RELEASE;
    }
}

void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    if (action == GLFW_PRESS) {
        if (button == GLFW_MOUSE_BUTTON_LEFT)
            leftMousePressed = true;
        if (button == GLFW_MOUSE_BUTTON_RIGHT)
            rightMousePressed = true;
        firstMouse = true; // reset to avoid jump
    }
    else if (action == GLFW_RELEASE) {
        if (button == GLFW_MOUSE_BUTTON_LEFT)
            leftMousePressed = false;
        if (button == GLFW_MOUSE_BUTTON_RIGHT)
            rightMousePressed = false;
    }
}

void mouseCallback(GLFWwindow* w, double xpos, double ypos) {
    if (firstMouse) { 
        lastX = xpos; 
        lastY = ypos; 
        firstMouse = false;
        return;
    }
    float dx = xpos - lastX;
    float dy = ypos - lastY;
    lastX = xpos;
    lastY = ypos;
    if (leftMousePressed)
        cam.processMouseDrag(dx, dy);
    if (rightMousePressed) {
        cam.processPan(dx, dy);
    }

    //cam.processMouse(dx, dy, leftMousePressed, rightMousePressed);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    cam.processScroll((float)yoffset);  // Positive yoffset = zoom in
}


int main() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(800, 600, "SDF Raymarching", NULL, NULL);
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
    glfwSetKeyCallback(window, keyCallback);
    glfwSetCursorPosCallback(window, mouseCallback);
    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    glfwSetScrollCallback(window, scroll_callback);

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

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    ImGui::StyleColorsDark(); // or Light()

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    float lastFrame = glfwGetTime();
    while (!glfwWindowShouldClose(window)) {
        // Start new ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Draw UI
        ImGui::Begin("Scene");
        ImGui::End();

        float time = glfwGetTime();
        float dt = time - lastFrame;
        lastFrame = time;
        //cam.changeMode(mode);
        cam.processKeyboard(keys, dt);

        shader.use();
        glUniform3fv(glGetUniformLocation(shader.ID, "camPos"), 1, &cam.position[0]);
        glm::mat4 invVP = cam.getInverseViewProj();
        glUniformMatrix4fv(glGetUniformLocation(shader.ID, "invViewProj"), 1, GL_FALSE, &invVP[0][0]);
        glUniform2f(glGetUniformLocation(shader.ID, "iResolution"), windowWidth, windowHeight);

        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        // Render ImGui on top
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwTerminate();
    return 0;
}
