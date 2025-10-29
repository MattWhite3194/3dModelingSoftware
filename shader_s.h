#pragma once

#include <glad/glad.h>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <stb/stb_image.h>
#include <glm/gtc/type_ptr.hpp>

class Shader
{
public:
    unsigned int ID;
    // constructor generates the shader on the fly
    // ------------------------------------------------------------------------
    Shader(const char* vertexPath, const char* fragmentPath, const char* geometryPath = nullptr) {
        ID = glCreateProgram();
        compileShader(vertexPath, GL_VERTEX_SHADER);
        compileShader(fragmentPath, GL_FRAGMENT_SHADER);
        if (geometryPath) {
            compileShader(geometryPath, GL_GEOMETRY_SHADER);
        }
    }

    void use()
    {
        glUseProgram(ID);
    }
    // utility uniform functions
    // ------------------------------------------------------------------------
    void setBool(const std::string& name, bool value) const
    {
        glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value);
    }
    // ------------------------------------------------------------------------
    void setInt(const std::string& name, int value) const
    {
        glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
    }
    // ------------------------------------------------------------------------
    void setFloat(const std::string& name, float value) const
    {
        glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
    }
    void setVec2(const std::string& name, glm::vec2 vector2)
    {
        glUniform2fv(glGetUniformLocation(ID, name.c_str()), 1, glm::value_ptr(vector2));
    }
    // ------------------------------------------------------------------------
    void setVec3(const std::string& name, glm::vec3 vector3)
    {
        glUniform3fv(glGetUniformLocation(ID, name.c_str()), 1, glm::value_ptr(vector3));
    }
    void setVec4(const std::string& name, glm::vec4 vector4)
    {
        glUniform4fv(glGetUniformLocation(ID, name.c_str()), 1, glm::value_ptr(vector4));
    }
    void setMat4(const std::string& name, glm::mat4 matrix4) {
        glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, glm::value_ptr(matrix4));
    }
private:
    // utility function for checking shader compilation/linking errors.
    // ------------------------------------------------------------------------
    void checkCompileErrors(unsigned int shader, std::string type) {
        int success;
        char infoLog[1024];
        if (type != "PROGRAM")
        {
            glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
            if (!success)
            {
                glGetShaderInfoLog(shader, 1024, NULL, infoLog);
                std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
            }
        }
        else
        {
            glGetProgramiv(shader, GL_LINK_STATUS, &success);
            if (!success)
            {
                glGetProgramInfoLog(shader, 1024, NULL, infoLog);
                std::cout << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
            }
        }
    }
    void compileShader(const char* shaderPath, GLenum shaderType) {
        std::string shaderCode;
        std::ifstream shaderFile;
        shaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
        try
        {
            // open files
            shaderFile.open(shaderPath);
            std::stringstream vShaderStream;
            // read file's buffer contents into streams
            vShaderStream << shaderFile.rdbuf();
            // close file handlers
            shaderFile.close();
            // convert stream into string
            shaderCode = vShaderStream.str();
        }
        catch (std::ifstream::failure& e)
        {
            std::cout << "ERROR::SHADER::FILE_NOT_SUCCESSFULLY_READ: " << e.what() << std::endl;
        }
        const char* shaderCodeRaw = shaderCode.c_str();
        // 2. compile shaders
        unsigned int shader;
        // vertex shader
        shader = glCreateShader(shaderType);
        glShaderSource(shader, 1, &shaderCodeRaw, NULL);
        glCompileShader(shader);
        checkCompileErrors(shader, "VERTEX");
        // shader Program
        glAttachShader(ID, shader);
        glLinkProgram(ID);
        checkCompileErrors(ID, "PROGRAM");
        // delete the shaders as they're linked into our program now and no longer necessary
        glDeleteShader(shader);
    }
};