#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <vector>
#include <string>
#include <Eigen/Geometry>
class Mesh;
class Camera;
class Shader;
class TextureManager;
class Scene
{
public:
    Scene();
    ~Scene();

    std::shared_ptr<Camera> camera;
    std::vector<std::shared_ptr<Mesh>> models;
    std::vector<std::shared_ptr<Shader>> shaders;
    std::shared_ptr<TextureManager> texMgr;
    std::shared_ptr<Mesh> LoadModel(const std::string& file_path);
    void DrawGrid();
    void SetupGrid();
    void Render(const Eigen::Matrix4f& viewMtx, const Eigen::Matrix4f& modelMtx);
    uint32_t GetNumVerts();
    uint32_t GetNumEdges();
    void ShowWireframe();
    void HideWireframe();
    const unsigned int SCR_WIDTH;
    const unsigned int SCR_HEIGHT;
    const char* title;
    float MIX_VALUE;
    float TIME_STATE_MULT;
private:
    GLuint m_gridVAO = 0;
    GLuint m_gridVBO = 0;
    std::vector<Eigen::Vector3f> m_gridVerts;
    bool m_doGrid = true;
    bool m_doWire = true;
};