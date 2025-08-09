#include <glad/glad.h>
#include <GLFW/glfw3.h>

// Dear ImGui core
#include "imgui.h"
// ImGui backends
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
// STD libs
#include <iostream>
#include <sstream>
#include <algorithm>
#include <vector>
#include <memory>
#include <string>
#include <filesystem>
// Eigen
#include <Eigen/Geometry>
// Local
#include "utils.h"
#include "Shader.h"
#include "TextureManager.h"
#include "stb_image.h"
#include "Scene.h"
#include "Camera.h"
#include "Mesh.h"

static const std::string g_assets_folder = ASSETS_DIR;

void framebuffer_size_clbk(GLFWwindow* window, int width, int height);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

Scene* MyScene = NULL;

void setupScene(Scene* scene)
{
    MyScene->SetupGrid();
    // Create a TextureManager to automatically load texture when materials are loaded in models
    MyScene->texMgr = std::make_shared<TextureManager>();
    //////////////////// MESH SETUP /////////////////////////
    auto modelPath = std::filesystem::path(g_assets_folder) / "plane.obj";
    auto Mesh = MyScene->LoadModel(modelPath.string().c_str());

    ////////////////// SHADER SETUP  ///////////////////////
    auto vertShaderPath = std::filesystem::path(g_assets_folder) / "VertexShader.vert";
    auto fragShaderPath = std::filesystem::path(g_assets_folder) / "Frag_BasicLighting.frag";
    auto geomShaderPath = std::filesystem::path(g_assets_folder) / "GeoWireframe.gs";

    /*std::shared_ptr<Shader> shader = std::make_shared<Shader>(
        vertShaderPath.string().c_str(),
        fragShaderPath.string().c_str());*/

    std::shared_ptr<Shader> shader = std::make_shared<Shader>(
        vertShaderPath.string().c_str(),
        fragShaderPath.string().c_str(),
        geomShaderPath.string().c_str());

    if (Mesh)
    {
        Mesh->m_shader = shader;
    }
    Eigen::Vector3f wireColor(1.0f, 0.0f, 0.0f);
    shader->use();
    shader->setVec3("wireColor", wireColor.data());
    shader->setInt("doWire", 1);
    /*shader->setInt("texture1", 0);
    shader->setBool("hasTexture", true);*/
}

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    
    MyScene = new Scene(); // This creates a default camera

    GLFWwindow* window = glfwCreateWindow(MyScene->SCR_WIDTH, MyScene->SCR_HEIGHT, MyScene->title, NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create the GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;   // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;       // Enable Docking
    // io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;  // Enable Multi-Viewport

    // 5. Setup ImGui style
    ImGui::StyleColorsDark();
    // If viewports are enabled, tweak style:
    // ImGuiStyle& style = ImGui::GetStyle();
    // if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
    //     style.WindowRounding = 0.0f;
    //     style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    // }

    // 6. Initialize ImGui GLFW + OpenGL3 backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    glfwSetFramebufferSizeCallback(window, framebuffer_size_clbk);

    //glfwSetInputMode(window, GLFW_STICKY_KEYS, GLFW_TRUE);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    glfwSetKeyCallback(window, [](GLFWwindow* w, int key, int scancode, int action, int mods) {
        ImGui_ImplGlfw_KeyCallback(w, key, scancode, action, mods);
        key_callback(w, key, scancode, action, mods);
    });

    glfwSetCursorPosCallback(window, [](GLFWwindow* w, double x, double y) {
        ImGui_ImplGlfw_CursorPosCallback(w, x, y);
        mouse_callback(w, x, y);
    });

    glfwSetMouseButtonCallback(window, [](GLFWwindow* w, int button, int action, int mods) {
        ImGui_ImplGlfw_MouseButtonCallback(w, button, action, mods);
        mouse_button_callback(w, button, action, mods);
    });

    glfwSetScrollCallback(window, [](GLFWwindow* w, double xoffset, double yoffset) {
        ImGui_ImplGlfw_ScrollCallback(w, xoffset, yoffset);
        scroll_callback(w, xoffset, yoffset);
    });
    //////////////////  SCENE SETUP //////////////////////
    setupScene(MyScene);

    // this is the render loop, that is also the frame processing

    //glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glFrontFace(GL_CW);
    glCullFace(GL_BACK);
    float last_rot = 0.0f;

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        // Start ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // 9. Your own simple window
        {
            static float f = 0.0f;
            static int counter = 0;
            ImGui::Begin("Hello, ImGui!");
            // Near plane slider
            if (ImGui::SliderFloat("Near Plane", &MyScene->camera->NEAR, 0.01f, MyScene->camera->FAR - 0.1f, "%.3f")) {
                MyScene->camera->updateProjMtx();
            }
            // Far plane slider
            if (ImGui::SliderFloat("Far Plane", &MyScene->camera->FAR,MyScene->camera->NEAR + 0.1f, 1000.0f, "%.3f")) {
                MyScene->camera->updateProjMtx();
            }
            if (ImGui::SliderFloat("FOV", &MyScene->camera->FOV, 0.1f, 180.0f, "%.3f")) {
                MyScene->camera->updateProjMtx();
            }
            /*ImGui::Text("This is a basic ImGui window.");
            ImGui::SliderFloat("float", &f, 0.0f, 1.0f);
            if (ImGui::Button("Button")) counter++;
            ImGui::SameLine();
            ImGui::Text("Counter = %d", counter);
            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
                1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);*/
            ImGui::End();
        }

        // Rendering
        ImGui::Render();
        // compute transforms
        float time = (float)glfwGetTime();
        Eigen::Matrix4f viewMtx = MyScene->camera->getMtx();
        Eigen::Matrix4f modelMtx = Eigen::Matrix4f::Identity();
        //modelMtx.topLeftCorner<3, 3>() *= 0.01f;

        // now we do the rendering
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        MyScene->Render(viewMtx, modelMtx);

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData()); 

        // update buffer
        glfwSwapBuffers(window);
    }
    
    glfwTerminate();
    return 0;
}

void framebuffer_size_clbk(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    if (key == GLFW_KEY_P && action == GLFW_PRESS)
        MyScene->TIME_STATE_MULT = 1.0f;
    if (key == GLFW_KEY_SPACE && action == GLFW_PRESS)
        MyScene->TIME_STATE_MULT = 0.0f;
    if (key == GLFW_KEY_LEFT_BRACKET && action == GLFW_PRESS)
    {
        MyScene->camera->FOV = std::max(0.0f, MyScene->camera->FOV - 2.0f);
        MyScene->camera->updateProjMtx();
    }
    if (key == GLFW_KEY_RIGHT_BRACKET && action == GLFW_PRESS)
    {
        MyScene->camera->FOV += std::min(180.0f, MyScene->camera->FOV + 2.0f);
        MyScene->camera->updateProjMtx();
    }
    MyScene->camera->handleKeyInputs(key);
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow) || ImGui::IsAnyItemActive()) {
        // Mouse is over *some* ImGui window (including popups, tooltips, docks)
        return;
    }
    if (MyScene->camera->active)
    {
        MyScene->camera->handleMouseMotion(xpos, ypos);
    }
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    bool state = MyScene->camera->active;
    MyScene->camera->handleMouseButtonInputs(button, action);
    if (state != MyScene->camera->active) // This suggests the states switched, so we need to store the current mouse position
    {
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);
        MyScene->camera->updateLastPos((float)xpos, (float)ypos);
    }
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    MyScene->camera->handleMouseScroll(yoffset);
}
