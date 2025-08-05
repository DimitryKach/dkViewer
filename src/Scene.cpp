#include "Scene.h"
#include "Camera.h"

Scene::Scene() : SCR_WIDTH(1000), SCR_HEIGHT(600), title("LearnOpenGL") {
    MIX_VALUE = 0.2f;
    FOV = 90.0f;
    ASPECT_RATIO = (float)SCR_WIDTH / (float)SCR_HEIGHT;
    NEAR = 0.5f;
    FAR = 10.0f;
    TIME_STATE_MULT = 1.0;
    projectionMtx = CalcProjectionMtx();
}

bool Scene::LoadModel(const std::string& file_path)
{
    std::shared_ptr<BaseMesh> newMesh = std::make_shared<BaseMesh>();
    models.emplace_back(newMesh);
    newMesh->LoadFile(file_path);
    return false;
}

Eigen::Matrix4f Scene::CalcProjectionMtx()
{
    float tanHalfFOV = tanf(ToRadian(FOV / 2.0f));
    float d = 1.0f / tanHalfFOV;
    float A = (-FAR - NEAR) / (FAR - NEAR);
    float B = -(2 * FAR * NEAR) / (FAR - NEAR);

    Eigen::Matrix4f pm;
    pm <<
        d / ASPECT_RATIO, 0.0f, 0.0f, 0.0f,
        0.0f, d, 0.0f, 0.0f,
        0.0f, 0.0f, A, B,
        0.0f, 0.0f, -1.0f, 0.0f;

    return pm;
}