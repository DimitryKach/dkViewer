#include "TextureManager.h"
#include "glad/glad.h"
#include "GLFW/glfw3.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <iostream>

struct TextureData {
    unsigned char* data;
    int width;
    int height;
    int nChannels;
};

bool TextureManager::loadTexture(const std::string& texture_path, unsigned int& id)
{
    stbi_set_flip_vertically_on_load(1);
    TextureData texture;
    texture.data = stbi_load(texture_path.c_str(), &texture.width, &texture.height, &texture.nChannels, 0);
    if (!texture.data)
    {
        std::cout << "Texture " << texture_path << " failed to load" << std::endl;
        return false;
    }
    std::cout << "Texture " << texture_path << " loaded succesfully" << std::endl;
    glGenTextures(1, &id);
    // Bind this texture to modify it
    glBindTexture(GL_TEXTURE_2D, id);
    if (texture.nChannels == 3)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texture.width, texture.height, 0, GL_RGB, GL_UNSIGNED_BYTE, texture.data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else if (texture.nChannels == 4)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texture.width, texture.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, texture.data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    glBindTexture(GL_TEXTURE_2D, 0);
    stbi_image_free(texture.data);
    m_allTextureIDs.push_back(id);
    return true;
}

TextureManager::~TextureManager()
{
    for (unsigned int& texID : m_allTextureIDs)
    {
        glDeleteTextures(1, &texID);
    }
}