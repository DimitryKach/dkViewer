#include <glad/glad.h> // include glad to get all the required OpenGL headers
#pragma once

#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

class Shader
{
public:
	Shader(const char* vertexShaderPath, const char* fragShaderPath)
	{
		std::string vertexCode, fragCode;
		std::ifstream vertFile, fragFile;

		vertFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
		fragFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

		try
		{
			vertFile.open(vertexShaderPath);
			fragFile.open(fragShaderPath);
			std::stringstream vertStream, fragStream;
			vertStream << vertFile.rdbuf();
			fragStream << fragFile.rdbuf();
			vertFile.close();
			fragFile.close();
			vertexCode = vertStream.str();
			fragCode   = fragStream.str();
		}
		catch (std::ifstream::failure& e)
		{
			std::cout << "ERROR::SHADER::FILE_NOT_SUCCESSFULLY_READ: " << e.what() << std::endl;
		}
		const char* vShaderCode = vertexCode.c_str();
		const char* fShaderCode = fragCode.c_str();

        // compiling the vertex shader
        // vertex shader
        unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertexShader, 1, &vShaderCode, NULL);
        glCompileShader(vertexShader);
        // check for shader compile errors
        errorHandler(vertexShader, "VERTEX");
        // fragment shader
        unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragmentShader, 1, &fShaderCode, NULL);
        glCompileShader(fragmentShader);
        // check for shader compile errors
        errorHandler(vertexShader, "FRAGMENT");
        // link shaders
        ID = glCreateProgram();
        glAttachShader(ID, vertexShader);
        glAttachShader(ID, fragmentShader);
        glLinkProgram(ID);
        // check for linking errors
        errorHandler(vertexShader, "PROGRAM");
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);

	}
    ~Shader(){};
    unsigned int ID;
    void use() {
        glUseProgram(ID);
        for (unsigned int i = 0; i < m_textureIDs.size(); i++)
        {
            setInt(("texture" + std::to_string(i + 1)), m_textureIDs[i]);
        }
    }
    void del() {
        glDeleteProgram(ID);
    }
    void addTexture(unsigned int id) {
        m_textureIDs.push_back(id);
    }

    void setBool(const std::string& name, bool value) const
    {
        glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value);
    }
    void setInt(const std::string& name, int value) const
    {
        glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
    }
    void setFloat(const std::string& name, float value) const
    {
        glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
    }
    void setMat4(const std::string& name, const float *value) const
    {
        glUniformMatrix4fv( glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, value);
    }
    void setVec3(const std::string& name, const float* value) const
    {
        glUniform3fv(glGetUniformLocation(ID, name.c_str()), 1, value);
    }

    void errorHandler(unsigned int shader, std::string type)
    {
        int success;
        char infoLog[1024];
        if (type != "PROGRAM")
        {
            glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
            if (!success)
            {
                glGetShaderInfoLog(shader, 1024, NULL, infoLog);
                std::cout << "ERROR::SHADER::" << type << "::COMPILATION_FAILED\n" << infoLog << std::endl;
            }
        }
        else
        {
            glGetProgramiv(ID, GL_LINK_STATUS, &success);
            if (!success) {
                glGetProgramInfoLog(ID, 1024, NULL, infoLog);
                std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
            }
        }
    }
    private:
        std::vector<unsigned int> m_textureIDs;
};