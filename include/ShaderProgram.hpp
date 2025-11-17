#pragma once

#include "utils.hpp"

#include <glad/gl.h>
#include <glm/gtc/type_ptr.hpp>
#define GLFW_INCLUDE_NONE
#include <iostream>
#include <glm/glm.hpp>

class ShaderProgram {
public:
    void init(const std::string &vertexShaderPath,
        const std::string &fragmentShaderPath)
    {
        int success;
        char infoLog[512];

        // Vertex shader
        const std::string vertexShaderString
            = parseFileToString(vertexShaderPath);
        const char *vertexShaderSource = vertexShaderString.c_str();

        const unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
        glCompileShader(vertexShader);

        glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);

        if (!success) {
            glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
            std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n"
                      << infoLog << std::endl;
        }

        // Fragment shader
        const std::string fragmentShaderString
            = parseFileToString(fragmentShaderPath);
        const char *fragmentShaderSource = fragmentShaderString.c_str();

        const unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
        glCompileShader(fragmentShader);

        glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);

        if (!success) {
            glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
            std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n"
                      << infoLog << std::endl;
        }

        // Shader Program
        m_shaderProgram = glCreateProgram();

        glAttachShader(m_shaderProgram, vertexShader);
        glAttachShader(m_shaderProgram, fragmentShader);
        glLinkProgram(m_shaderProgram);

        glGetProgramiv(m_shaderProgram, GL_LINK_STATUS, &success);

        if (!success) {
            glGetProgramInfoLog(m_shaderProgram, 512, NULL, infoLog);
            std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n"
                      << infoLog << std::endl;
        }

        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
    }

    void use() const { glUseProgram(m_shaderProgram); }

    void setVec3(const std::string &uniformName, const glm::vec3 vec3) const
    {
        glUniform3f(glGetUniformLocation(m_shaderProgram, uniformName.c_str()),
            vec3.x, vec3.y, vec3.z);
    }

    void setVec2(const std::string &uniformName, const glm::vec2 vec2) const
    {
        glUniform2f(glGetUniformLocation(m_shaderProgram, uniformName.c_str()),
            vec2.x, vec2.y);
    }

    void setMat4(const std::string &uniformName, glm::mat4 mat4) const
    {
        glUniformMatrix4fv(
            glGetUniformLocation(m_shaderProgram, uniformName.c_str()), 1,
            GL_FALSE, glm::value_ptr(mat4));
    }

    void setBool(const std::string &uniformName, const bool value) const
    {
        glUniform1i(
            glGetUniformLocation(m_shaderProgram, uniformName.c_str()), value);
    }

    void setInt(const std::string &uniformName, const int value) const
    {
        glUniform1i(
            glGetUniformLocation(m_shaderProgram, uniformName.c_str()), value);
    }

    void setFloat(const std::string &uniformName, const float value) const
    {
        glUniform1f(
            glGetUniformLocation(m_shaderProgram, uniformName.c_str()), value);
    }

private:
    unsigned int m_shaderProgram;
};
