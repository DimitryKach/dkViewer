#include "Mesh.h"
#include <filesystem>

#define SAFE_DELETE(p) if (p) { delete p; p = NULL; }
#define ARRAY_SIZE_IN_ELEMENTS(a) (sizeof(a)/sizeof(a[0]))
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

    if (m_buffers[0] != 0) {
        glDeleteBuffers(ARRAY_SIZE_IN_ELEMENTS(m_buffers), m_buffers);
    }

    if (m_VAO != 0) {
        glDeleteVertexArrays(1, &m_VAO);
        m_VAO = 0;
    }
    m_materials.clear();
    m_meshes.clear();
    m_edges.clear();
}

void Mesh::Draw()
{
    glBindVertexArray(m_VAO);
    //glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glDrawArrays(GL_TRIANGLES, 0, 36);
}

uint32_t Mesh::GetNumVerts()
{
    return uint32_t(m_positions.size());
}

uint32_t Mesh::GetNumEdges()
{
    return uint32_t(m_edges.size());
}

bool Mesh::LoadFile(const std::string& Filename)
{
    // Release the previously loaded mesh (if it exists)
    Clear();

    // Create the VAO
    glGenVertexArrays(1, &m_VAO);
    glBindVertexArray(m_VAO);

    // Create the buffers for the vertices attributes
    glGenBuffers(ARRAY_SIZE_IN_ELEMENTS(m_buffers), m_buffers);

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
    m_meshes.resize(pScene->mNumMeshes);
    //m_Textures.resize(pScene->mNumMaterials);

    unsigned int NumVertices = 0;
    unsigned int NumIndices = 0;

    CountVerticesAndIndices(pScene, NumVertices, NumIndices);

    ReserveSpace(NumVertices, NumIndices);

    InitAllMeshes(pScene);

    materials_loaded = InitMaterials(pScene, Filename);

    PopulateBuffers();

    return GLCheckError();
}


void Mesh::CountVerticesAndIndices(const aiScene* pScene, unsigned int& NumVertices, unsigned int& NumIndices)
{
    for (unsigned int i = 0; i < m_meshes.size(); i++) {
        m_meshes[i].MaterialIndex = pScene->mMeshes[i]->mMaterialIndex;
        m_meshes[i].NumIndices = pScene->mMeshes[i]->mNumFaces * 3;
        m_meshes[i].BaseVertex = NumVertices;
        m_meshes[i].BaseIndex = NumIndices;

        NumVertices += pScene->mMeshes[i]->mNumVertices;
        NumIndices += m_meshes[i].NumIndices;
    }
}


void Mesh::ReserveSpace(unsigned int NumVertices, unsigned int NumIndices)
{
    m_positions.reserve(NumVertices);
    m_normals.reserve(NumVertices);
    m_texCoords.reserve(NumVertices);
    m_indices.reserve(NumIndices);
}


void Mesh::InitAllMeshes(const aiScene* pScene)
{
    for (unsigned int i = 0; i < m_meshes.size(); i++) {
        const aiMesh* paiMesh = pScene->mMeshes[i];
        InitSingleMesh(paiMesh);
    }
}


void Mesh::InitSingleMesh(const aiMesh* paiMesh)
{
    const aiVector3D Zero3D(0.0f, 0.0f, 0.0f);
    auto make_edge = [](uint32_t u, uint32_t v) noexcept { // Always have the larger index first
        if (u > v) std::swap(u, v);
        return Edge{ u, v };
    };

    auto isCorner = [](const aiVector3D pos) {
        if (abs(pos.x) < 0.00001f && (abs(abs(pos.y) - 2.1258f) < 0.00001f || abs(abs(pos.y) - 0.1258f) < 0.00001f) && abs(abs(pos.z) - 1.0f) < 0.00001f) return true;
        return false;
    };

    // Populate the vertex attribute vectors
    for (unsigned int i = 0; i < paiMesh->mNumVertices; i++) {
        const aiVector3D& pPos = paiMesh->mVertices[i];
        //std::cout << i << " " << pPos.x << " " << pPos.y << " " << pPos.z << std::endl;
        const aiVector3D& pNormal = paiMesh->mNormals[i];
        const aiVector3D& pTexCoord = paiMesh->HasTextureCoords(0) ? paiMesh->mTextureCoords[0][i] : Zero3D;

        m_positions.push_back(Eigen::Vector3f(pPos.x, pPos.y, pPos.z));
        m_normals.push_back(Eigen::Vector3f(pNormal.x, pNormal.y, pNormal.z));
        m_texCoords.push_back(Eigen::Vector2f(pTexCoord.x, pTexCoord.y));
    }

    // Populate the index buffer and edges
    m_numFaces = paiMesh->mNumFaces;
    for (unsigned int i = 0; i < m_numFaces; i++) {
        const aiFace& Face = paiMesh->mFaces[i];
        assert(Face.mNumIndices == 3);
        const uint32_t v0 = Face.mIndices[0], v1 = Face.mIndices[1], v2 = Face.mIndices[2];
        m_indices.push_back(v0);
        m_indices.push_back(v1);
        m_indices.push_back(v2);
        // Populate edges
        const std::array<std::pair<uint32_t, uint32_t>, 3> _edges{ { {v0,v1}, {v1,v2}, {v2,v0} } };
        for (auto& [_v1, _v2] : _edges)
        {
            Edge e = make_edge(_v1, _v2);
            m_edges.insert(e);
        }
    }
}

void Mesh::RecomputeNormals()
{
    std::fill(m_normals.begin(), m_normals.end(), Eigen::Vector3f::Zero());
    for (uint16_t i = 0; i < (m_indices.size()/3); i++)
    {
        auto v1 = m_positions[m_indices[i * 3]];
        auto v2 = m_positions[m_indices[i * 3 + 1]];
        auto v3 = m_positions[m_indices[i * 3 + 2]];

        auto wNorm = (v2 - v1).cross(v3 - v1);
        m_normals[m_indices[i * 3]] += wNorm;
        m_normals[m_indices[i * 3 + 1]] += wNorm;
        m_normals[m_indices[i * 3 + 2]] += wNorm;
    }
    // Normalize the normals
    for (auto& norm : m_normals)
    {
        norm.normalize();
    }
    glBindBuffer(GL_ARRAY_BUFFER, m_buffers[NORMAL_VB]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(m_normals[0]) * m_normals.size(), nullptr, GL_DYNAMIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(m_normals[0]) * m_normals.size(), &m_normals[0]);
}

void Mesh::SetVertex(const Eigen::Vector3f& pos, uint16_t id)
{
    m_positions[id] = pos;
}

bool Mesh::InitMaterials(const aiScene* pScene, const std::string& Filename)
{
    if (!m_texMgr)
    {
        std::cout << "Material loading can only be done with an attached TextureManager." << std::endl;
        return false;
    }
    // Extract the directory part from the file name
    std::filesystem::path _p(Filename);
    auto Dir = _p.parent_path();

    bool Ret = true;

    // Initialize the materials
    for (unsigned int i = 0; i < pScene->mNumMaterials; i++) {
        const aiMaterial* pMaterial = pScene->mMaterials[i];
        BasicMaterialEntry material;

        //m_Textures[i] = NULL;

        if (pMaterial->GetTextureCount(aiTextureType_DIFFUSE) > 0) {
            aiString Path;

            if (pMaterial->GetTexture(aiTextureType_DIFFUSE, 0, &Path, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS) {
                std::string p(Path.data);

                if (p.substr(0, 2) == ".\\") {
                    p = p.substr(2, p.size() - 2);
                }

                std::string FullPath = (Dir / p).string();
                if (!std::filesystem::exists(FullPath))
                {
                    std::cout << "The provided texture " << FullPath << " doesn't exist" << std::endl;
                    m_materials.push_back(material);
                    continue;
                }
                material.texturePath = FullPath;
                unsigned int id;
                if (!m_texMgr->loadTexture(FullPath, id))
                {
                    return false;
                }
                material.texID = id;
            }
        }
        m_materials.push_back(material);
    }

    return Ret;
}

Eigen::Vector3f Mesh::GetVertex(uint16_t id)
{
    Eigen::Vector3f out = m_positions[id];
    return out;
}

void Mesh::PopulateBuffers()
{
    glBindBuffer(GL_ARRAY_BUFFER, m_buffers[POS_VB]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(m_positions[0]) * m_positions.size(), &m_positions[0], GL_STATIC_DRAW);
    glEnableVertexAttribArray(POSITION_LOCATION);
    glVertexAttribPointer(POSITION_LOCATION, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glBindBuffer(GL_ARRAY_BUFFER, m_buffers[TEXCOORD_VB]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(m_texCoords[0]) * m_texCoords.size(), &m_texCoords[0], GL_STATIC_DRAW);
    glEnableVertexAttribArray(TEX_COORD_LOCATION);
    glVertexAttribPointer(TEX_COORD_LOCATION, 2, GL_FLOAT, GL_FALSE, 0, 0);

    glBindBuffer(GL_ARRAY_BUFFER, m_buffers[NORMAL_VB]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(m_normals[0]) * m_normals.size(), &m_normals[0], GL_STATIC_DRAW);
    glEnableVertexAttribArray(NORMAL_LOCATION);
    glVertexAttribPointer(NORMAL_LOCATION, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_buffers[INDEX_BUFFER]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(m_indices[0]) * m_indices.size(), &m_indices[0], GL_STATIC_DRAW);
}

void Mesh::UpdatePositionBuffer()
{
    glBindBuffer(GL_ARRAY_BUFFER, m_buffers[POS_VB]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(m_positions[0]) * m_positions.size(), nullptr, GL_DYNAMIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(m_positions[0]) * m_positions.size(), &m_positions[0]);
}

void Mesh::Render()
{
    glBindVertexArray(m_VAO);

    for (unsigned int i = 0; i < m_meshes.size(); i++) {
        unsigned int MaterialIndex = m_meshes[i].MaterialIndex;

        assert(MaterialIndex < m_materials.size());

        BasicMaterialEntry* material = &m_materials[MaterialIndex];
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, material->texID);

        glDrawElementsBaseVertex(GL_TRIANGLES,
            m_meshes[i].NumIndices,
            GL_UNSIGNED_INT,
            (void*)(sizeof(unsigned int) * m_meshes[i].BaseIndex),
            m_meshes[i].BaseVertex);
    }

    // Make sure the VAO is not changed from the outside
    glBindVertexArray(0);
}