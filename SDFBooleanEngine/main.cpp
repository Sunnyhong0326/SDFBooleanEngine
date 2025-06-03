#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <memory>
#include <iostream>
#include <vector>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <glm/gtc/type_ptr.hpp>
#include "rendering/Shader.hpp"
#include "rendering/Camera.hpp"
#include "core/MarchingCube.hpp"
#include "core/MeshUtils.hpp"
#include "sdf/SDFSceneGL.hpp"
#include "sdf/CSGTree.hpp"
#include "ui/CSGAppState.hpp"
#include "ui/CSGAppUI.hpp"

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
    std::unique_ptr<Shader> meshShader = std::make_unique<Shader>("shaders/mesh.vert", "shaders/mesh.frag");

    std::unique_ptr<CSGTree> csgTree = std::make_unique<CSGTree>();
    std::unique_ptr<MarchingCubes> marchingCubes = std::make_unique<MarchingCubes>();
    std::unique_ptr<SDFSceneGL> sdfScene = std::make_unique<SDFSceneGL>();
    sdfScene->setupQuad();
    /*std::vector<Triangle> tris{};
    sdfScene->uploadMesh(tris);*/
    /*sdfScene->setupQuad();
    sdfScene->setupCSGTreeAABB(csgTree.get());
    sdfScene->uploadScene(csgTree->getNodes());*/

    CSGAppState appState;
    appState.csgTree = csgTree.get();
    appState.marchingCubes = marchingCubes.get();
    appState.sdfScene = sdfScene.get();
    CSGAppUI appUI(appState);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark(); 
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 430");

    float lastFrame = glfwGetTime();
    while (!glfwWindowShouldClose(window)) {
        // mimic the same background in mesh renderer
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        float time = glfwGetTime();
        float dt = time - lastFrame;
        lastFrame = time;

        imguiIO = io.WantCaptureMouse;
        cam.processKeyboard(keys, dt);
        cam.update();

        appUI.draw();

        if (appState.hasExtracted && appState.onMeshExtracted) {
            appState.onMeshExtracted();
            appState.onMeshExtracted = nullptr;
            appState.joinBackgroundThread();
        }

        if (!appState.useMeshRenderer) {
            rayMarchShader->use();
            sdfScene->setRaymarchUniforms(rayMarchShader->ID, cam.position, cam.getInverseViewProj(), glm::vec2(windowWidth, windowHeight));
            sdfScene->bindScene(0);
            sdfScene->renderQuad();
        }
        else {
            meshShader->use();
            sdfScene->setMeshUniforms(meshShader->ID, cam.getViewProjMatrix());
            sdfScene->renderMesh();
        }

        if (appState.showAABB) {
            aabbShader->use();
            sdfScene->setCSGTreeAABBUniforms(aabbShader->ID, cam.getViewProjMatrix());
            sdfScene->renderCSGTreeAABB();
        }

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    appState.joinBackgroundThread();
    glfwTerminate();
    return 0;
}
