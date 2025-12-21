#define TINYOBJLOADER_IMPLEMENTATION
#include "Mesh.h"
#include <filesystem>
#include <map>
#include <unordered_map>

#define SAFE_DELETE(p) if (p) { delete p; p = NULL; }
#define ARRAY_SIZE_IN_ELEMENTS(a) (sizeof(a)/sizeof(a[0]))
#define ASSIMP_LOAD_FLAGS (aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_JoinIdenticalVertices)
#define GLCheckError() (glGetError() == GL_NO_ERROR)
#define POSITION_LOCATION  0
#define TEX_COORD_LOCATION 1
#define NORMAL_LOCATION    2

Mesh::Mesh()
{
    doRecompNormals = false;
    modelMtx = Eigen::Matrix4f::Identity();
}

Mesh::Mesh(std::shared_ptr<TextureManager> texMgr)
{
    m_texMgr = texMgr;
    doRecompNormals = false;
    modelMtx = Eigen::Matrix4f::Identity();
}

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

uint32_t Mesh::GetNumTriangles()
{
    return uint32_t(m_indices.size()/3);
}

int* Mesh::GetTriIndices(const int triId)
{
    int* triInds = new int[3];
    triInds[0] = m_indices[3 * triId];
    triInds[1] = m_indices[3 * triId + 1];
    triInds[2] = m_indices[3 * triId + 2];
    return triInds;
}

Eigen::Matrix4f Mesh::GetModelMtx()
{
    return modelMtx;
}

void Mesh::SetModelMtx(const Eigen::Matrix4f& mtx)
{
    modelMtx = mtx;
}

bool Mesh::LoadFile(const std::string& Filename)
{
    std::cout << "Using Assimp to load" << Filename << std::endl;
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
        assert(Face.mNumIndices > 1);
        for (uint16_t i = 0; i < Face.mNumIndices; i++)
        {
            uint32_t v0 = Face.mIndices[i];
            uint32_t v1 = (i == Face.mNumIndices - 1) ? Face.mIndices[0] : Face.mIndices[i + 1];
            m_indices.push_back(v0);
            Edge e = make_edge(v0, v1);
            m_edges.insert(e);
        }
    }
}

void Mesh::RecomputeNormals()
{
    std::fill(m_normals.begin(), m_normals.end(), Eigen::Vector3f::Zero());
    for (int i = 0; i < (m_indices.size() / 3); i++)
    {
        auto v1 = m_positions[m_indices[i * 3]];
        auto v2 = m_positions[m_indices[i * 3 + 1]];
        auto v3 = m_positions[m_indices[i * 3 + 2]];

        auto wNorm = (v2 - v1).cross(v3 - v1);
        m_normals[m_indices[i * 3]] += wNorm;
        m_normals[m_indices[i * 3 + 1]] += wNorm;
        m_normals[m_indices[i * 3 + 2]] += wNorm;
        //std::cout << "i=" << i << std::endl;
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

void Mesh::SetRecomputeNormals(bool recomp)
{
    doRecompNormals = recomp;
}

AABB Mesh::ComputeAABB()
{
    AABB meshAABB;
    float minX, maxX = m_positions[0](0);
    float minY, maxY = m_positions[0](1);
    float minZ, maxZ = m_positions[0](2);
    // Find the boundaries
    for (auto& pos : m_positions)
    {
        if (pos(0) < minX) minX = pos(0);
        if (pos(0) > maxX) maxX = pos(0);
        if (pos(1) < minY) minY = pos(1);
        if (pos(1) > maxY) maxY = pos(1);
        if (pos(2) < minZ) minZ = pos(2);
        if (pos(2) > maxZ) maxZ = pos(2);
    }
    meshAABB.width = maxX - minX;
    meshAABB.depth = maxY - minY;
    meshAABB.height = maxZ - minZ;

    return meshAABB;
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

Eigen::Vector3f Mesh::GetVertex(uint16_t id, bool worldSpace)
{
    Eigen::Vector3f out = m_positions[id];
    if (worldSpace)
        out = modelMtx.block<3,3>(0,0) * out + modelMtx.block<3, 1>(0, 3);
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
    if (doRecompNormals) RecomputeNormals();
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

bool Mesh::LoadFileTinyObj(const std::string& Filename, bool updateGPUBuffers)
{
    // TODO: We need a way to store non-duplicated world positions and indices ALONG
    //       with the buffer elements. We will need an array that stores GPU vertex
    //       indices as related to the 
    std::cout << "Using TinyObjLoader to load" << Filename << std::endl;
    // Release the previously loaded mesh (if it exists)
    Clear();

    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;

    std::filesystem::path p(Filename);
    std::string base_dir = p.parent_path().string() + "/";

    bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, Filename.c_str(), base_dir.c_str(), false);

    if (!warn.empty()) {
        std::cout << "TinyObj Warn: " << warn << std::endl;
    }

    if (!err.empty()) {
        std::cerr << "TinyObj Err: " << err << std::endl;
    }

    if (!ret) {
        return false;
    }

    // Process materials
    for (const auto& mat : materials) {
        BasicMaterialEntry material;
        if (!mat.diffuse_texname.empty()) {
            std::string fullPath = base_dir + mat.diffuse_texname;
            if (std::filesystem::exists(fullPath)) {
                material.texturePath = fullPath;
                unsigned int id;
                if (m_texMgr && m_texMgr->loadTexture(fullPath, id)) {
                    material.texID = id;
                }
            }
            else {
                std::cout << "Texture not found: " << fullPath << std::endl;
            }
        }
        m_materials.push_back(material);
    }

    if (m_materials.empty()) {
        m_materials.push_back(BasicMaterialEntry());
    }

    // Map unique vertex attributes to a single index
    struct IndexCombo {
        int v_idx, n_idx, t_idx;
        bool operator==(const IndexCombo& other) const {
            return v_idx == other.v_idx && n_idx == other.n_idx && t_idx == other.t_idx;
        }
    };

    // A unique vertex is defined as a unique hash of the position index, the normal index, and the texture index.
    // This is used by the unordered_map for the hash table.
    struct IndexComboHash {
        size_t operator()(const IndexCombo& k) const {
            return ((std::hash<int>()(k.v_idx) ^ (std::hash<int>()(k.n_idx) << 1)) >> 1) ^ (std::hash<int>()(k.t_idx) << 1);
        }
    };

    // This is the exploded index view for the vertices to be stored within the ARRAY buffer
    std::unordered_map<IndexCombo, unsigned int, IndexComboHash> uniqueVertices;

    // Quick creation of edges
    auto make_edge = [](uint32_t u, uint32_t v) noexcept {
        if (u > v) std::swap(u, v);
        return Edge{ u, v };
    };

    // Map<material_id, vector<indices>>
    std::map<int, std::vector<unsigned int>> materialToIndices;

    for (const auto& shape : shapes) {
        size_t index_offset = 0;
        // Here we go over the face data. We go through a list that contains the number of verts for each face
        for (size_t f = 0; f < shape.mesh.num_face_vertices.size(); f++) {
            // This is the current face's number of vertices. We will shift the index_offset by this much later
            int fv = shape.mesh.num_face_vertices[f];
            // Material IDs are per-face
            int mat_id = shape.mesh.material_ids[f];
            if (mat_id < 0) mat_id = 0;

            // Resolve all vertices for this face first. We will store NEW face indices that refer to the GLOBAL
            // indices we are creating with the uniqueVertices unordered_map.
            std::vector<unsigned int> faceIndices;
            faceIndices.reserve(fv);
            // We go through each vertex of the face now
            for (int v = 0; v < fv; v++) {
                // If I understand correctly, this is the unique TinyObj way of storing face indices so that we
                // have separate UV and normal data per vertex.
                tinyobj::index_t idx = shape.mesh.indices[index_offset + v];
                IndexCombo combo = { idx.vertex_index, idx.normal_index, idx.texcoord_index };

                if (uniqueVertices.find(combo) == uniqueVertices.end()) {
                    // Note a tricky moment - a vertex that may occupy the SAME position, might have a different
                    // normal and/or texture coord. Thus, it needs to be stored in the m_positions separately.
                    unsigned int newIdx = (unsigned int)m_positions.size();
                    uniqueVertices[combo] = newIdx;
                    faceIndices.push_back(newIdx);

                    // Push vertex data
                    if (idx.vertex_index >= 0) {
                        m_positions.push_back(Eigen::Vector3f(
                            attrib.vertices[3 * idx.vertex_index + 0],
                            attrib.vertices[3 * idx.vertex_index + 1],
                            attrib.vertices[3 * idx.vertex_index + 2]
                        ));
                    }
                    else {
                        m_positions.push_back(Eigen::Vector3f(0, 0, 0));
                    }

                    if (idx.normal_index >= 0) {
                        m_normals.push_back(Eigen::Vector3f(
                            attrib.normals[3 * idx.normal_index + 0],
                            attrib.normals[3 * idx.normal_index + 1],
                            attrib.normals[3 * idx.normal_index + 2]
                        ));
                    }
                    else {
                        m_normals.push_back(Eigen::Vector3f(0, 0, 0));
                    }

                    if (idx.texcoord_index >= 0) {
                        m_texCoords.push_back(Eigen::Vector2f(
                            attrib.texcoords[2 * idx.texcoord_index + 0],
                            attrib.texcoords[2 * idx.texcoord_index + 1]
                        ));
                    }
                    else {
                        m_texCoords.push_back(Eigen::Vector2f(0, 0));
                    }
                }
                else {
                    faceIndices.push_back(uniqueVertices[combo]);
                }
            }

            // Triangulate and add edges - we do this like a fan about the first vertex
            for (int v = 0; v < fv - 2; v++) {
                unsigned int i0 = faceIndices[0];
                unsigned int i1 = faceIndices[v + 1];
                unsigned int i2 = faceIndices[v + 2];

                materialToIndices[mat_id].push_back(i0);
                materialToIndices[mat_id].push_back(i1);
                materialToIndices[mat_id].push_back(i2);

                m_edges.insert(make_edge(i0, i1));
                m_edges.insert(make_edge(i1, i2));
                m_edges.insert(make_edge(i2, i0));
            }

            // Add second shear spring for quads
            if (fv == 4) {
                m_edges.insert(make_edge(faceIndices[1], faceIndices[3]));
            }

            index_offset += fv;
        }
    }

    // Now build m_meshes
    // TODO: I have questions about how this might handle multiple materials, but that's for later...
    for (auto& it : materialToIndices) {
        BasicMeshEntry entry;
        entry.MaterialIndex = it.first;
        entry.BaseVertex = 0;
        entry.BaseIndex = (unsigned int)m_indices.size();
        entry.NumIndices = (unsigned int)it.second.size();

        m_indices.insert(m_indices.end(), it.second.begin(), it.second.end());
        m_meshes.push_back(entry);
    }

    materials_loaded = true;
    if (updateGPUBuffers)
    {
        // Create the VAO
        glGenVertexArrays(1, &m_VAO);
        glBindVertexArray(m_VAO);

        // Create the buffers for the vertices attributes
        glGenBuffers(ARRAY_SIZE_IN_ELEMENTS(m_buffers), m_buffers);
        PopulateBuffers();
        glBindVertexArray(0);
        return GLCheckError();
    }

    return true;
}