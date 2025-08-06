#pragma once
#include <vector>
#include <string>
#include <Eigen/Geometry>
class Mesh;
class Camera;
class Shader;
class Scene
{
public:
    Scene();
    ~Scene();

    std::shared_ptr<Camera> camera;
    std::vector<std::shared_ptr<Mesh>> models;
    std::vector<std::shared_ptr<Shader>> shaders;
    std::shared_ptr<Mesh> LoadModel(const std::string& file_path);
    std::shared_ptr<Shader> CreateShader(const char* vertexShaderPath, const char* fragShaderPath);
    void AddTextureToShader(unsigned int id, std::shared_ptr<Shader> shader);
    const unsigned int SCR_WIDTH;
    const unsigned int SCR_HEIGHT;
    const char* title;
    float MIX_VALUE;
    float TIME_STATE_MULT;
};