#pragma once

#include <glad/glad.h>

#include <string>

namespace Voxel
{
    class Texture
    {
    public:

        Texture() = default;

        explicit Texture(
            const std::string& path,
            bool repeat = true
        );

        ~Texture();

        Texture(const Texture&) = delete;
        Texture& operator=(const Texture&) = delete;

        Texture(Texture&& other) noexcept;
        Texture& operator=(Texture&& other) noexcept;

        bool load(
            const std::string& path,
            bool repeat = true
        );

        void bind(
            unsigned int slot = 0
        ) const;

        void unbind() const;

        GLuint getID() const
        {
            return m_id;
        }

        int getWidth() const
        {
            return m_width;
        }

        int getHeight() const
        {
            return m_height;
        }

        bool isValid() const
        {
            return m_id != 0;
        }

    private:

        GLuint m_id = 0;

        int m_width = 0;
        int m_height = 0;
        int m_channels = 0;
    };
}