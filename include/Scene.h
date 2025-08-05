#include "utils.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <algorithm>
#include <vector>
#include <string>
#include "Mesh.h"
class Camera;
class Scene
{
public:
    Scene();
    ~Scene() = default;

    std::shared_ptr<Camera> camera;
    std::vector<std::shared_ptr<BaseMesh>> models;
    bool LoadModel(const std::string& file_path);
    void SetupGeometry(unsigned int& VBO, unsigned int& VAO, unsigned int& EBO);
    Eigen::Matrix4f CalcProjectionMtx();
    const unsigned int SCR_WIDTH;
    const unsigned int SCR_HEIGHT;
    const char* title;
    float MIX_VALUE;
    float FOV;
    float ASPECT_RATIO;
    float NEAR;
    float FAR;
    float TIME_STATE_MULT;
    Eigen::Matrix4f projectionMtx;
};