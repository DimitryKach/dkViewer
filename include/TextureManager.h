#pragma once
#include <unordered_set>
#include <stack>
#include <string>

class TextureManager
{
public:
	TextureManager() {};
	~TextureManager();

	bool loadTexture(const std::string& texture_path, unsigned int& id);

private:
	std::vector<unsigned int> m_allTextureIDs;
};