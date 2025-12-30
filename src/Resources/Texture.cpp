#include "Texture.h"

#include <stb_image.h>

#include <iostream>

Texture::Texture(const std::string& filepath)
{
    Load(filepath);
}

Texture::Texture(int width, int height, int channels)
    : m_Width(width), m_Height(height), m_Channels(channels)
{
    m_LocalBuffer = (unsigned char*)malloc(width * height * channels);
}

Texture::~Texture()
{
    Free();
}

Texture::Texture(Texture&& other) noexcept
{
    m_LocalBuffer = other.m_LocalBuffer;
    m_Width = other.m_Width;
    m_Height = other.m_Height;
    m_Channels = other.m_Channels;

    other.m_LocalBuffer = nullptr;
    other.m_Width = 0;
    other.m_Height = 0;
    other.m_Channels = 0;
}

Texture& Texture::operator=(Texture&& other) noexcept
{
    if (this != &other)
    {
        Free();

        m_LocalBuffer = other.m_LocalBuffer;
        m_Width = other.m_Width;
        m_Height = other.m_Height;
        m_Channels = other.m_Channels;

        other.m_LocalBuffer = nullptr;
        other.m_Width = 0;
        other.m_Height = 0;
        other.m_Channels = 0;
    }

    return *this;
}

Texture::Texture(glm::vec4 color)
{
    m_Channels = 4;
    m_Width = 4;
    m_Height = 4;
    
    int size = m_Width * m_Height * m_Channels;

    m_LocalBuffer = (uchar*)malloc(size);

    uchar r = static_cast<uchar>(color.r * 255.0f);
    uchar g = static_cast<uchar>(color.g * 255.0f);
    uchar b = static_cast<uchar>(color.b * 255.0f);
    uchar a = static_cast<uchar>(color.a * 255.0f);

    for (int i = 0; i < m_Width * m_Height; ++i)
    {
        int index = i * m_Channels;
        m_LocalBuffer[index + 0] = r;
        m_LocalBuffer[index + 1] = g;
        m_LocalBuffer[index + 2] = b;
        m_LocalBuffer[index + 3] = a;
    }
}

void Texture::Load(const std::string& path)
{
    Free();
    stbi_set_flip_vertically_on_load(1); 

    m_LocalBuffer = stbi_load(path.c_str(), &m_Width, &m_Height, &m_Channels, 4);
    if (m_LocalBuffer)
    {
        m_Channels = 4; 
    }
    else
    {
        std::cerr << "Failed to load texture: " << path << "\n";
        std::cerr << "Error: " << stbi_failure_reason() << std::endl;
    }
}

void Texture::Free()
{
    if (m_LocalBuffer)
    {
        stbi_image_free(m_LocalBuffer);
        m_LocalBuffer = nullptr;
        m_Width = 0;
        m_Height = 0;
        m_Channels = 0;
    }
}