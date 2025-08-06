#pragma once
#include "assimp/scene.h"
#include <assimp/postprocess.h>
#include <assimp/Importer.hpp>
#include <string>
#include <vector>
#include <Eigen/Geometry>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

class Shader;

class Mesh
{
public:
	Mesh() {};
	~Mesh();

	void Clear();
	bool LoadFile(const std::string& file_path);
	bool InitFromScene(const aiScene* pScene, const std::string& Filename);
	void CountVerticesAndIndices(const aiScene* pScene, unsigned int& NumVertices, unsigned int& NumIndices);
	void ReserveSpace(unsigned int NumVertices, unsigned int NumIndices);
	void InitAllMeshes(const aiScene* pScene);
	void InitSingleMesh(const aiMesh* paiMesh);
	void SetShader(const std::shared_ptr <Shader> shader);
	std::shared_ptr <Shader> GetShader();
	void Cleanup();
	void PopulateBuffers();
	void Render();
	void Draw();
#define INVALID_MATERIAL 0xFFFFFFFF
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
	GLuint m_Buffers[NUM_BUFFERS] = { 0 };
	struct Vertex {
		Eigen::Vector3f* position;
		Eigen::Vector2f* uv;
		Eigen::Vector3f* normal;
	};
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
	std::vector<Vertex> vertices;
	std::vector<BasicMeshEntry> m_Meshes;
	//std::vector<Texture*> m_Textures;
	std::shared_ptr<Shader> m_shader;
	std::vector<Eigen::Vector3f> m_Positions;
	std::vector<Eigen::Vector3f> m_Normals;
	std::vector<Eigen::Vector2f> m_TexCoords;
	std::vector<unsigned int> m_Indices;
};