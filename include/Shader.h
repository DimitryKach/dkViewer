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

    Shader(const char* vertexShaderPath, const char* fragShaderPath, const char* geomShaderPath)
    {
        std::string vertexCode, fragCode, geomCode;
        std::ifstream vertFile, fragFile, geomFile;

        vertFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
        fragFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
        geomFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

        try
        {
            vertFile.open(vertexShaderPath);
            fragFile.open(fragShaderPath);
            geomFile.open(geomShaderPath);
            std::stringstream vertStream, fragStream, geomStream;
            vertStream << vertFile.rdbuf();
            fragStream << fragFile.rdbuf();
            geomStream << geomFile.rdbuf();
            vertFile.close();
            fragFile.close();
            geomFile.close();
            vertexCode = vertStream.str();
            fragCode = fragStream.str();
            geomCode = geomStream.str();
        }
        catch (std::ifstream::failure& e)
        {
            std::cout << "ERROR::SHADER::FILE_NOT_SUCCESSFULLY_READ: " << e.what() << std::endl;
        }
        const char* vShaderCode = vertexCode.c_str();
        const char* fShaderCode = fragCode.c_str();
        const char* gShaderCode = geomCode.c_str();

        // compiling the vertex shader
        // vertex shader
        unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertexShader, 1, &vShaderCode, NULL);
        glCompileShader(vertexShader);
        // check for shader compile errors
        errorHandler(vertexShader, "VERTEX");
        // geometry shader
        unsigned int geometryShader = glCreateShader(GL_GEOMETRY_SHADER);
        glShaderSource(geometryShader, 1, &gShaderCode, NULL);
        glCompileShader(geometryShader);
        // check for shader compile errors
        errorHandler(geometryShader, "GEOMETRY");
        // fragment shader
        unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragmentShader, 1, &fShaderCode, NULL);
        glCompileShader(fragmentShader);
        // check for shader compile errors
        errorHandler(fragmentShader, "FRAGMENT");
        // link shaders
        ID = glCreateProgram();
        glAttachShader(ID, vertexShader);
        glAttachShader(ID, geometryShader);
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
    }
    void del() {
        glDeleteProgram(ID);
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
};