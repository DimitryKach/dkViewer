#include "Mesh.h"
#include <filesystem>

#define SAFE_DELETE(p) if (p) { delete p; p = NULL; }
#define ARRAY_SIZE_IN_ELEMENTS(a) (sizeof(m_Buffers)/sizeof(m_Buffers[0]))
#define ASSIMP_LOAD_FLAGS (aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_JoinIdenticalVertices)
#define GLCheckError() (glGetError() == GL_NO_ERROR)
#define POSITION_LOCATION  0
#define TEX_COORD_LOCATION 1
#define NORMAL_LOCATION    2

Mesh::~Mesh()
{
    Clear();
}

void Mesh::Clear()
{
    /*for (unsigned int i = 0; i < m_Textures.size(); i++) {
        SAFE_DELETE(m_Textures[i]);
    }*/

    if (m_Buffers[0] != 0) {
        glDeleteBuffers(ARRAY_SIZE_IN_ELEMENTS(m_Buffers), m_Buffers);
    }

    if (m_VAO != 0) {
        glDeleteVertexArrays(1, &m_VAO);
        m_VAO = 0;
    }
    m_Materials.clear();
    m_Meshes.clear();
}

void Mesh::Draw()
{
    glBindVertexArray(m_VAO);
    //glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glDrawArrays(GL_TRIANGLES, 0, 36);
}

bool Mesh::LoadFile(const std::string& Filename)
{
    // Release the previously loaded mesh (if it exists)
    Clear();

    // Create the VAO
    glGenVertexArrays(1, &m_VAO);
    glBindVertexArray(m_VAO);

    // Create the buffers for the vertices attributes
    glGenBuffers(ARRAY_SIZE_IN_ELEMENTS(m_Buffers), m_Buffers);

    bool Ret = false;
    Assimp::Importer Importer;

    const aiScene* pScene = Importer.ReadFile(Filename.c_str(), ASSIMP_LOAD_FLAGS);

    if (pScene) {
        Ret = InitFromScene(pScene, Filename);
    }
    else {
        printf("Error parsing '%s': '%s'\n", Filename.c_str(), Importer.GetErrorString());
    }

    // Make sure the VAO is not changed from the outside
    glBindVertexArray(0);

    return Ret;
}

bool Mesh::InitFromScene(const aiScene* pScene, const std::string& Filename)
{
    m_Meshes.resize(pScene->mNumMeshes);
    //m_Textures.resize(pScene->mNumMaterials);

    unsigned int NumVertices = 0;
    unsigned int NumIndices = 0;

    CountVerticesAndIndices(pScene, NumVertices, NumIndices);

    ReserveSpace(NumVertices, NumIndices);

    InitAllMeshes(pScene);

    InitMaterials(pScene, Filename);

    PopulateBuffers();

    return GLCheckError();
}


void Mesh::CountVerticesAndIndices(const aiScene* pScene, unsigned int& NumVertices, unsigned int& NumIndices)
{
    for (unsigned int i = 0; i < m_Meshes.size(); i++) {
        m_Meshes[i].MaterialIndex = pScene->mMeshes[i]->mMaterialIndex;
        m_Meshes[i].NumIndices = pScene->mMeshes[i]->mNumFaces * 3;
        m_Meshes[i].BaseVertex = NumVertices;
        m_Meshes[i].BaseIndex = NumIndices;

        NumVertices += pScene->mMeshes[i]->mNumVertices;
        NumIndices += m_Meshes[i].NumIndices;
    }
}


void Mesh::ReserveSpace(unsigned int NumVertices, unsigned int NumIndices)
{
    m_Positions.reserve(NumVertices);
    m_Normals.reserve(NumVertices);
    m_TexCoords.reserve(NumVertices);
    m_Indices.reserve(NumIndices);
}


void Mesh::InitAllMeshes(const aiScene* pScene)
{
    for (unsigned int i = 0; i < m_Meshes.size(); i++) {
        const aiMesh* paiMesh = pScene->mMeshes[i];
        InitSingleMesh(paiMesh);
    }
}


void Mesh::InitSingleMesh(const aiMesh* paiMesh)
{
    const aiVector3D Zero3D(0.0f, 0.0f, 0.0f);

    // Populate the vertex attribute vectors
    for (unsigned int i = 0; i < paiMesh->mNumVertices; i++) {
        const aiVector3D& pPos = paiMesh->mVertices[i];
        const aiVector3D& pNormal = paiMesh->mNormals[i];
        const aiVector3D& pTexCoord = paiMesh->HasTextureCoords(0) ? paiMesh->mTextureCoords[0][i] : Zero3D;

        m_Positions.push_back(Eigen::Vector3f(pPos.x, pPos.y, pPos.z));
        m_Normals.push_back(Eigen::Vector3f(pNormal.x, pNormal.y, pNormal.z));
        m_TexCoords.push_back(Eigen::Vector2f(pTexCoord.x, pTexCoord.y));
    }

    // Populate the index buffer
    for (unsigned int i = 0; i < paiMesh->mNumFaces; i++) {
        const aiFace& Face = paiMesh->mFaces[i];
        assert(Face.mNumIndices == 3);
        m_Indices.push_back(Face.mIndices[0]);
        m_Indices.push_back(Face.mIndices[1]);
        m_Indices.push_back(Face.mIndices[2]);
    }
}

bool Mesh::InitMaterials(const aiScene* pScene, const std::string& Filename)
{
    // Extract the directory part from the file name
    std::filesystem::path _p(Filename);
    auto Dir = _p.parent_path();

    bool Ret = true;

    // Initialize the materials
    for (unsigned int i = 0; i < pScene->mNumMaterials; i++) {

        const aiMaterial* pMaterial = pScene->mMaterials[i];
        BasicMaterialEntry material;
        m_Materials.push_back(material);

        //m_Textures[i] = NULL;

        if (pMaterial->GetTextureCount(aiTextureType_DIFFUSE) > 0) {
            aiString Path;

            if (pMaterial->GetTexture(aiTextureType_DIFFUSE, 0, &Path, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS) {
                std::string p(Path.data);

                if (p.substr(0, 2) == ".\\") {
                    p = p.substr(2, p.size() - 2);
                }

                std::string FullPath = (Dir / p).string();
                std::string state = std::filesystem::exists(FullPath) ? " exists" : " doesn't exist";
                std::cout << "Path " << FullPath << state << std::endl;
                material.texturePath = FullPath;
                if (m_TexMgr)
                {
                    unsigned int id;
                    if (!m_TexMgr->loadTexture(FullPath, id))
                    {
                        return false;
                    }
                    material.texID = id;
                }
                else
                {
                    std::cout << "No Texture Manager assigned - skipping the loading of texture " << FullPath << std::endl;
                }
            }
        }
    }

    return Ret;
}

void Mesh::PopulateBuffers()
{
    glBindBuffer(GL_ARRAY_BUFFER, m_Buffers[POS_VB]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(m_Positions[0]) * m_Positions.size(), &m_Positions[0], GL_STATIC_DRAW);
    glEnableVertexAttribArray(POSITION_LOCATION);
    glVertexAttribPointer(POSITION_LOCATION, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glBindBuffer(GL_ARRAY_BUFFER, m_Buffers[TEXCOORD_VB]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(m_TexCoords[0]) * m_TexCoords.size(), &m_TexCoords[0], GL_STATIC_DRAW);
    glEnableVertexAttribArray(TEX_COORD_LOCATION);
    glVertexAttribPointer(TEX_COORD_LOCATION, 2, GL_FLOAT, GL_FALSE, 0, 0);

    glBindBuffer(GL_ARRAY_BUFFER, m_Buffers[NORMAL_VB]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(m_Normals[0]) * m_Normals.size(), &m_Normals[0], GL_STATIC_DRAW);
    glEnableVertexAttribArray(NORMAL_LOCATION);
    glVertexAttribPointer(NORMAL_LOCATION, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_Buffers[INDEX_BUFFER]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(m_Indices[0]) * m_Indices.size(), &m_Indices[0], GL_STATIC_DRAW);
}

void Mesh::Render()
{
    glBindVertexArray(m_VAO);

    for (unsigned int i = 0; i < m_Meshes.size(); i++) {
        unsigned int MaterialIndex = m_Meshes[i].MaterialIndex;

        assert(MaterialIndex < m_Materials.size());

        BasicMaterialEntry* material = &m_Materials[MaterialIndex];
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, material->texID);

        glDrawElementsBaseVertex(GL_TRIANGLES,
            m_Meshes[i].NumIndices,
            GL_UNSIGNED_INT,
            (void*)(sizeof(unsigned int) * m_Meshes[i].BaseIndex),
            m_Meshes[i].BaseVertex);
    }

    // Make sure the VAO is not changed from the outside
    glBindVertexArray(0);
}