#pragma once

#include <string>

namespace Voxel
{
    class Shader
    {
    public:

        Shader(
            const std::string& vertexPath,
            const std::string& fragmentPath
        );

        ~Shader();

        Shader(const Shader&) = delete;
        Shader& operator=(const Shader&) = delete;

        void bind() const;
        void unbind() const;

        unsigned int getID() const;

    private:

        unsigned int compileShader(
            unsigned int type,
            const std::string& source
        );

        unsigned int createProgram(
            unsigned int vertexShader,
            unsigned int fragmentShader
        );

        std::string readFile(
            const std::string& path
        );

    private:

        unsigned int m_programID = 0;
    };
}