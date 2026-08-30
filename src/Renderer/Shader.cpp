#include "../Renderer/Shader.hpp"

#include <glad/glad.h>

#include <fstream>
#include <sstream>
#include <stdexcept>

namespace Voxel
{
    Shader::Shader(
        const std::string& vertexPath,
        const std::string& fragmentPath
    )
    {
        const std::string vertexSource =
            readFile(vertexPath);

        const std::string fragmentSource =
            readFile(fragmentPath);

        const unsigned int vertexShader =
            compileShader(
                GL_VERTEX_SHADER,
                vertexSource
            );

        const unsigned int fragmentShader =
            compileShader(
                GL_FRAGMENT_SHADER,
                fragmentSource
            );

        m_programID =
            createProgram(
                vertexShader,
                fragmentShader
            );

        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
    }

    Shader::~Shader()
    {
        if (m_programID != 0)
        {
            glDeleteProgram(m_programID);
        }
    }

    void Shader::bind() const
    {
        glUseProgram(m_programID);
    }

    void Shader::unbind() const
    {
        glUseProgram(0);
    }

    unsigned int Shader::getID() const
    {
        return m_programID;
    }

    std::string Shader::readFile(
        const std::string& path
    )
    {
        std::ifstream file(path);

        if (!file.is_open())
        {
            throw std::runtime_error(
                "Failed to open shader: " + path
            );
        }

        std::stringstream buffer;

        buffer << file.rdbuf();

        return buffer.str();
    }

    unsigned int Shader::compileShader(
        unsigned int type,
        const std::string& source
    )
    {
        const char* sourceCode =
            source.c_str();

        const unsigned int shader =
            glCreateShader(type);

        glShaderSource(
            shader,
            1,
            &sourceCode,
            nullptr
        );

        glCompileShader(shader);

        int success = 0;

        glGetShaderiv(
            shader,
            GL_COMPILE_STATUS,
            &success
        );

        if (!success)
        {
            char infoLog[1024];

            glGetShaderInfoLog(
                shader,
                sizeof(infoLog),
                nullptr,
                infoLog
            );

            glDeleteShader(shader);

            throw std::runtime_error(
                "Shader compilation failed:\n" +
                std::string(infoLog)
            );
        }

        return shader;
    }

    unsigned int Shader::createProgram(
        unsigned int vertexShader,
        unsigned int fragmentShader
    )
    {
        const unsigned int program =
            glCreateProgram();

        glAttachShader(
            program,
            vertexShader
        );

        glAttachShader(
            program,
            fragmentShader
        );

        glLinkProgram(program);

        int success = 0;

        glGetProgramiv(
            program,
            GL_LINK_STATUS,
            &success
        );

        if (!success)
        {
            char infoLog[1024];

            glGetProgramInfoLog(
                program,
                sizeof(infoLog),
                nullptr,
                infoLog
            );

            glDeleteProgram(program);

            throw std::runtime_error(
                "Shader linking failed:\n" +
                std::string(infoLog)
            );
        }

        return program;
    }
}