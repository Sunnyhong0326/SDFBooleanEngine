#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "rendering/Shader.hpp"
#include "rendering/Camera.hpp"
#include "sdf/SDFSceneGL.hpp"
#include "sdf/SDFBuilder.hpp"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include "sdf/SDFLoader.hpp"

Camera cam;
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
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(800, 600, "SDF Raymarching", NULL, NULL);
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
    glfwSetKeyCallback(window, keyCallback);
    glfwSetCursorPosCallback(window, mouseCallback);
    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    glfwSetScrollCallback(window, scroll_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "GLAD init failed!" << std::endl;
        return -1;
    }
    // SDFBuilder builder;

    // // House body
    // int body = builder.addBox(glm::vec3(0.0f, 0.5f, 0.0f), 1.0f, glm::vec3(0.8, 0.5, 0.3));

    // // Door
    // int door = builder.addBox(glm::vec3(0.0f, 0.0f, 1.01f), 0.25f, glm::vec3(0.3, 0.2, 0.1));
    // int houseWithDoor = builder.subtract(body, door, glm::vec3(0.8, 0.5, 0.3));

    // // Windows
    // int window1 = builder.addBox(glm::vec3(-0.4f, 0.5f, 1.01f), 0.15f, glm::vec3(0.2, 0.4, 1.0));
    // int window2 = builder.addBox(glm::vec3(0.4f, 0.5f, 1.01f), 0.15f, glm::vec3(0.2, 0.4, 1.0));
    // houseWithDoor = builder.subtract(houseWithDoor, window1, glm::vec3(0.8, 0.5, 0.3));
    // houseWithDoor = builder.subtract(houseWithDoor, window2, glm::vec3(0.8, 0.5, 0.3));

    // // Slanted Roof using triangle prism
    // int roof = builder.addTriPrism(glm::vec3(0.0f, 1.5f, 0.0f), 2.0f, 1.0f, glm::vec3(0.5, 0.1, 0.1));

    // // Chimney
    // int chimney = builder.addBox(glm::vec3(0.5f, 2.0f, 0.2f), 0.15f, glm::vec3(0.2, 0.1, 0.1));

    // // Steps
    // int step1 = builder.addBox(glm::vec3(0.0f, -0.6f, 1.2f), 0.3f, glm::vec3(0.5, 0.4, 0.3));
    // int step2 = builder.addBox(glm::vec3(0.0f, -0.4f, 1.1f), 0.25f, glm::vec3(0.5, 0.4, 0.3));
    // int steps = builder.unionOp(step1, step2, glm::vec3(0.5, 0.4, 0.3));

    // // Fence pillars
    // int fence = -1;
    // for (int i = -2; i <= 2; ++i) {
    //     glm::vec3 pos(i * 0.4f, 0.0f, 1.5f);
    //     int pillar = builder.addBox(pos, 0.05f, glm::vec3(0.4, 0.3, 0.2));
    //     if (fence == -1) fence = pillar;
    //     else fence = builder.unionOp(fence, pillar, glm::vec3(0.4, 0.3, 0.2));
    // }

    // // Final union
    // std::vector<int> components = { houseWithDoor, roof, chimney, steps, fence };
    // int fullHouse = components[0];
    // for (size_t i = 1; i < components.size(); ++i) {
    //     fullHouse = builder.unionOp(fullHouse, components[i], glm::vec3(1.0, 1.0, 1.0));
    // }

    // Build final scene
    //std::vector<SDFNode> nodes = builder.build();


    // Retrieve full node list to upload
    std::vector<SDFNode> sceneNodes = loadNodesFromJson("assets/house.json");
    std::cout << "Size " << sceneNodes.size() << std::endl;

    Shader shader("shaders/raymarch.vert", "shaders/raymarch.frag");
    SDFSceneGL sdfScene;
    sdfScene.setupQuad();
    sdfScene.uploadScene(sceneNodes);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    ImGui::StyleColorsDark(); // or Light()

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 430");

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

        cam.processKeyboard(keys, dt);
        std::vector<glm::vec3> lightDirs = {
            glm::normalize(glm::vec3(0.5f, 0.8f, -1.0f)),
            glm::normalize(glm::vec3(-0.3f, 0.6f, 0.7f))
        };

        glUniform1i(glGetUniformLocation(shader.ID, "numLights"), lightDirs.size());
        glUniform3fv(glGetUniformLocation(shader.ID, "lights"), lightDirs.size(), glm::value_ptr(lightDirs[0]));
        // Set OpenGL state
        glClearColor(0, 0, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT);
        shader.use();
        sdfScene.setUniforms(shader.ID, cam.position, cam.getInverseViewProj(), glm::vec2(windowWidth, windowHeight));
        sdfScene.bindScene(0);
        sdfScene.renderQuad();

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
