#pragma once
#include "assimp/scene.h"
#include <assimp/postprocess.h>
#include <assimp/Importer.hpp>
#include <string>
#include <vector>
#include <Eigen/Geometry>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "TextureManager.h"
#include "Shader.h"
#include <unordered_set>

class Mesh
{
public:
	Mesh() {};
	Mesh(std::shared_ptr<TextureManager> texMgr)
	{
		m_texMgr = texMgr;
	}
	~Mesh();

#define INVALID_MATERIAL 0xFFFFFFFF
	struct BasicMeshEntry {
		BasicMeshEntry()
		{
			NumIndices = 0;
			BaseVertex = 0;
			BaseIndex = 0;
			MaterialIndex = INVALID_MATERIAL;
		}
		unsigned int NumIndices;
		unsigned int BaseVertex;
		unsigned int BaseIndex;
		unsigned int MaterialIndex;
	};
	struct BasicMaterialEntry {
		unsigned int texID;
		std::string texturePath;
	};
	struct Edge { uint32_t a, b; };
	void Clear();
	bool LoadFile(const std::string& file_path);
	bool InitFromScene(const aiScene* pScene, const std::string& Filename);
	void CountVerticesAndIndices(const aiScene* pScene, unsigned int& NumVertices, unsigned int& NumIndices);
	void ReserveSpace(unsigned int NumVertices, unsigned int NumIndices);
	void InitAllMeshes(const aiScene* pScene);
	void InitSingleMesh(const aiMesh* paiMesh);
	bool InitMaterials(const aiScene* pScene, const std::string& Filename);
	std::shared_ptr<Shader> m_shader;
	void Cleanup();
	void PopulateBuffers();
	void Render();
	void Draw();
	std::vector<BasicMeshEntry> m_meshes;
	std::vector<BasicMaterialEntry> m_materials;
	bool materials_loaded = false;
private:
	enum BUFFER_TYPE {
		INDEX_BUFFER = 0,
		POS_VB = 1,
		TEXCOORD_VB = 2,
		NORMAL_VB = 3,
		WVP_MAT_VB = 4,
		WORLD_MAT_VB = 5,
		NUM_BUFFERS = 6
	};
	Eigen::Matrix4f m_worldTransform;
	GLuint m_VAO = 0;
	GLuint m_buffers[NUM_BUFFERS] = { 0 };
	struct Vertex {
		Eigen::Vector3f* position;
		Eigen::Vector2f* uv;
		Eigen::Vector3f* normal;
	};
	std::vector<Vertex> vertices;
	//std::vector<Texture*> m_Textures;
	std::vector<Eigen::Vector3f> m_positions;
	std::vector<Eigen::Vector3f> m_normals;
	std::vector<Eigen::Vector2f> m_texCoords;
	unsigned int m_numFaces;
	std::vector<unsigned int> m_indices;
	std::shared_ptr<TextureManager> m_texMgr;
	// Cool thing I learned - EdgeHash is a hashing functor (defining operator() makes it a functor). We then pass an Edge, which has two
	// 32-bit integers for the vertex IDs, and we hash them by first ordering each edge by having vId1 < vId2, and then putting vId1 into
	// the high of the 64 bit hash, and vId2 into the low. This allows us to hash the unordered_set.
	struct EdgeHash { size_t operator()(const Edge& e) const noexcept { return (uint64_t)e.a << 32 | e.b; } }; // the
	struct EdgeEq { bool operator()(const Edge& e1, const Edge& e2) const noexcept { return e1.a == e2.a && e1.b == e2.b; } };
	std::unordered_set<Edge, EdgeHash, EdgeEq> m_edges;
};