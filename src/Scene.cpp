#include "utils.h"
#include "Scene.h"
#include "Mesh.h"
#include "Camera.h"
#include "Shader.h"

Scene::Scene() : SCR_WIDTH(2560), SCR_HEIGHT(1440), TIME_STATE_MULT(1.0f), title("DK Viewer")
{
    camera = std::make_shared<Camera>();
    camera->FOV = 45.0f;
    camera->ASPECT_RATIO = (float)SCR_WIDTH / (float)SCR_HEIGHT;
    camera->NEAR = 0.5f;
    camera->FAR = 10.0f;
    camera->updateProjMtx();
}

Scene::~Scene()
{
    glDeleteVertexArrays(1, &m_gridVAO);
    glDeleteBuffers(1, &m_gridVBO);
}

std::shared_ptr<Mesh> Scene::LoadModel(const std::string& file_path)
{
    std::shared_ptr<Mesh> newMesh = static_cast<bool>(texMgr) ? std::make_shared<Mesh>(texMgr) : std::make_shared<Mesh>();
    if (!newMesh->LoadFileTinyObj(file_path))
    {
        return nullptr;
    };
    models.push_back(newMesh);
    return newMesh;
}

void Scene::SetupGrid()
{
    m_doGrid = true;
    m_gridVerts.clear();
    int numPoints = 10;
    float scale = 100.0f;
    float step = scale / float(numPoints);
    for (int i = -numPoints; i <= numPoints; ++i) {
        // line parallel to X-axis at Z = i
        m_gridVerts.push_back({ -scale, 0.0f, step * (float)i});
        m_gridVerts.push_back({ scale, 0.0f, step * (float)i });
        // line parallel to Z-axis at X = i
        m_gridVerts.push_back({ step * (float)i, 0.0f, -scale });
        m_gridVerts.push_back({ step * (float)i, 0.0f,  scale });
    }
    glGenVertexArrays(1, &m_gridVAO);
    glGenBuffers(1, &m_gridVBO);
    glBindVertexArray(m_gridVAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_gridVBO);
    glBufferData(GL_ARRAY_BUFFER, m_gridVerts.size() * sizeof(Eigen::Vector3f),
        m_gridVerts.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glBindVertexArray(0);
}

void Scene::Render(const Eigen::Matrix4f& viewMtx, const Eigen::Matrix4f& modelMtx)
{
    if (m_doGrid) DrawGrid();
    Eigen::Matrix4f MV = viewMtx * modelMtx;
    Eigen::Matrix4f MVP = camera->projectionMtx * MV;
    Eigen::Matrix4f NormalMtx = MV.inverse().transpose();
    // Static light color - white
    Eigen::Vector3f lightColor = Eigen::Vector3f(1.0f, 1.0f, 1.0f);
    // For now we pin the light to the camera
    Eigen::Vector3f lightPos = camera->position;
    for (auto& mesh : models)
    {
        mesh->UpdatePositionBuffer();
        mesh->RecomputeNormals();
        mesh->m_shader->use();
        // Bind view and projection matrices
        mesh->m_shader->setMat4("MVP", MVP.data());
        mesh->m_shader->setMat4("MV", MV.data());
        mesh->m_shader->setMat4("NormalMtx", NormalMtx.data());
        mesh->m_shader->setMat4("ViewMtx", viewMtx.data());
        // Bind the light color and pos
        mesh->m_shader->setVec3("lightColor", lightColor.data());
        mesh->m_shader->setVec3("lightPos", lightPos.data());
        //shader.setMat4("transform", final.data());
        mesh->Render();
    }
}

uint32_t Scene::GetNumVerts()
{
    unsigned int numVerts = 0;
    for (auto& model : models)
    {
        numVerts += model->GetNumVerts();
    }
    return numVerts;
}

uint32_t Scene::GetNumEdges()
{
    unsigned int numEdges = 0;
    for (auto& model : models)
    {
        numEdges += model->GetNumEdges();
    }
    return numEdges;
}

void Scene::ShowWireframe()
{
    for (auto& shader : shaders)
    {
        m_doWire = true;
        shader->use();
        shader->setBool("doWire", m_doWire);
    }
}

void Scene::HideWireframe()
{
    for (auto& shader : shaders)
    {
        m_doWire = false;
        shader->use();
        shader->setBool("doWire", m_doWire);
    }
}

void Scene::DrawGrid()
{
    glBindVertexArray(m_gridVAO);
    glDrawArrays(GL_LINES, 0, m_gridVerts.size());
    glBindVertexArray(0);
}
