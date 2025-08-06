#include "utils.h"
#include "Scene.h"
#include "Mesh.h"
#include "Camera.h"
#include "Shader.h"

Scene::Scene() : SCR_WIDTH(1000), SCR_HEIGHT(600), TIME_STATE_MULT(1.0f), title("LearnOpenGL")
{
    camera = std::make_shared<Camera>();
    camera->FOV = 90.0f;
    camera->ASPECT_RATIO = (float)SCR_WIDTH / (float)SCR_HEIGHT;
    camera->NEAR = 0.5f;
    camera->FAR = 10.0f;
    camera->updateProjMtx();
}

Scene::~Scene()
{
}

std::shared_ptr<Mesh> Scene::LoadModel(const std::string& file_path)
{
    std::shared_ptr<Mesh> newMesh = static_cast<bool>(texMgr) ? std::make_shared<Mesh>(texMgr) : std::make_shared<Mesh>();
    if (!newMesh->LoadFile(file_path))
    {
        return nullptr;
    };
    models.push_back(newMesh);
    return newMesh;
}
