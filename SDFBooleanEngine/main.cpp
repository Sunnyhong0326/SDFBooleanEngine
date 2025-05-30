#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "rendering/Shader.hpp"
#include "rendering/Camera.hpp"
#include "sdf/CSGTree.hpp"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <vector>
#include "core/MarchingCube.hpp"
#include "core/MeshUtils.hpp"
#include "sdf/SDFSceneGL.hpp"
#include <memory>

Camera cam;
bool keys[1024];
float lastX = 400, lastY = 300;
bool firstMouse = true;
bool leftMousePressed = false;
bool rightMousePressed = false;
bool imguiIO = false;
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
    if (!imguiIO && leftMousePressed)
        cam.processMouseDrag(dx, dy);
    if (!imguiIO && rightMousePressed) {
        cam.processPan(dx, dy);
    }
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    cam.processScroll((float)yoffset);  // Positive yoffset = zoom in
}


int main() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(800, 600, "CSG SDF Raymarching", NULL, NULL);
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

    std::unique_ptr<Shader> rayMarchShader = std::make_unique<Shader>("shaders/raymarch.vert", "shaders/raymarch.frag");
    std::unique_ptr<Shader> aabbShader = std::make_unique<Shader>("shaders/aabb.vert", "shaders/aabb.frag");

    std::unique_ptr<CSGTree> csgTree = std::make_unique<CSGTree>();
    csgTree->loadNodesFromJson("assets/gemini_robot.json");

    // Run marching cubes
    std::unique_ptr<MarchingCubes> marchingCubes = std::make_unique<MarchingCubes>();
    int rootIndex = csgTree->getNumNodes() - 1;
    AABB bbox = csgTree->computeAABB(rootIndex);
    auto grid = marchingCubes->sampleGrid(csgTree, rootIndex, bbox, glm::ivec3(128, 128, 128));
    auto tris = marchingCubes->run(grid, 0.0f);
    exportToPLY("output.ply", tris);


    std::unique_ptr<SDFSceneGL> sdfScene = std::make_unique<SDFSceneGL>();
    sdfScene->setupQuad();
    sdfScene->setupMCVoxel(grid);
    sdfScene->setupCSGTreeAABB(csgTree);
    sdfScene->uploadScene(csgTree->getNodes());

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    ImGui::StyleColorsDark(); // or Light()

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 430");

    float lastFrame = glfwGetTime();
    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("Scene");
        ImGui::End();

        float time = glfwGetTime();
        float dt = time - lastFrame;
        lastFrame = time;

        imguiIO = io.WantCaptureMouse;
        cam.processKeyboard(keys, dt);
        cam.update();

        // ray march shader
        rayMarchShader->use();

        std::vector<glm::vec3> lightDirs = {
            glm::normalize(glm::vec3(0.5f, 0.8f, -1.0f)),
            glm::normalize(glm::vec3(-0.3f, 0.6f, 0.7f))
        };
        glUniform1i(glGetUniformLocation(rayMarchShader->ID, "numLights"), lightDirs.size());
        glUniform3fv(glGetUniformLocation(rayMarchShader->ID, "lights"), lightDirs.size(), glm::value_ptr(lightDirs[0]));

        sdfScene->setRaymarchUniforms(rayMarchShader->ID, cam.position, cam.getInverseViewProj(), glm::vec2(windowWidth, windowHeight));
        sdfScene->bindScene(0);
        sdfScene->renderQuad();

        // aabb line shader
        aabbShader->use();
        sdfScene->setCSGTreeAABBUniforms(aabbShader->ID, cam.getViewProjMatrix());
        sdfScene->renderCSGTreeAABB();

        // voxel shader
        aabbShader->use();
        sdfScene->setMCVoxelUniforms(aabbShader->ID, cam.getViewProjMatrix());
        sdfScene->renderMCVoxel();

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
