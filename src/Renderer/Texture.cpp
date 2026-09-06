#include "../Renderer/Texture.hpp"

#include <stb_image.h>

#include <iostream>
#include <utility>

namespace Voxel
{
    Texture::Texture(
        const std::string& path,
        bool repeat
    )
    {
        load(
            path,
            repeat
        );
    }

    Texture::~Texture()
    {
        if (m_id != 0)
        {
            glDeleteTextures(
                1,
                &m_id
            );
        }
    }

    Texture::Texture(
        Texture&& other
    ) noexcept
        :
        m_id(other.m_id),
        m_width(other.m_width),
        m_height(other.m_height),
        m_channels(other.m_channels)
    {
        other.m_id = 0;
        other.m_width = 0;
        other.m_height = 0;
        other.m_channels = 0;
    }

    Texture& Texture::operator=(
        Texture&& other
        ) noexcept
    {
        if (this == &other)
            return *this;

        if (m_id != 0)
        {
            glDeleteTextures(
                1,
                &m_id
            );
        }

        m_id = other.m_id;
        m_width = other.m_width;
        m_height = other.m_height;
        m_channels = other.m_channels;

        other.m_id = 0;
        other.m_width = 0;
        other.m_height = 0;
        other.m_channels = 0;

        return *this;
    }

    bool Texture::load(
        const std::string& path,
        bool repeat
    )
    {
        if (m_id != 0)
        {
            glDeleteTextures(
                1,
                &m_id
            );

            m_id = 0;
        }

        stbi_set_flip_vertically_on_load(
            false
        );

        unsigned char* data =
            stbi_load(
                path.c_str(),
                &m_width,
                &m_height,
                &m_channels,
                4
            );

        if (!data)
        {
            std::cerr
                << "Failed to load texture: "
                << path
                << '\n';

            return false;
        }

        glGenTextures(
            1,
            &m_id
        );

        glBindTexture(
            GL_TEXTURE_2D,
            m_id
        );

        glTexParameteri(
            GL_TEXTURE_2D,
            GL_TEXTURE_MIN_FILTER,
            GL_NEAREST
        );

        glTexParameteri(
            GL_TEXTURE_2D,
            GL_TEXTURE_MAG_FILTER,
            GL_NEAREST
        );

        glTexParameteri(
            GL_TEXTURE_2D,
            GL_TEXTURE_WRAP_S,
            GL_CLAMP_TO_EDGE
        );

        glTexParameteri(
            GL_TEXTURE_2D,
            GL_TEXTURE_WRAP_T,
            GL_CLAMP_TO_EDGE
        );

        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_RGBA8,
            m_width,
            m_height,
            0,
            GL_RGBA,
            GL_UNSIGNED_BYTE,
            data
        );

       /* glGenerateMipmap(
            GL_TEXTURE_2D
        );*/

        glBindTexture(
            GL_TEXTURE_2D,
            0
        );

        stbi_image_free(data);

        std::cout
            << "Loaded texture: "
            << path
            << " ("
            << m_width
            << "x"
            << m_height
            << ")\n";

        return true;
    }

    void Texture::bind(
        unsigned int slot
    ) const
    {
        glActiveTexture(
            GL_TEXTURE0 + slot
        );

        glBindTexture(
            GL_TEXTURE_2D,
            m_id
        );
    }

    void Texture::unbind() const
    {
        glBindTexture(
            GL_TEXTURE_2D,
            0
        );
    }
}