#include "RenderTexture.h"
#include <iostream>

RenderTexture::RenderTexture(const Texture& texture)
    : m_Width(texture.GetWidth()), m_Height(texture.GetHeight())
{
    CreateTexture(texture);
}

RenderTexture::~RenderTexture()
{
    glDeleteTextures(1, &m_RendererID);
}

void RenderTexture::CreateTexture(const Texture& texture)
{
    glGenTextures(1, &m_RendererID);
    glBindTexture(GL_TEXTURE_2D, m_RendererID);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    GLenum internalFormat = GL_RGBA8;
    GLenum dataFormat = GL_RGBA;

    if (texture.GetChannels() == 1)
    {
        internalFormat = GL_R8;
        dataFormat     = GL_RED;
    }
    else if (texture.GetChannels() == 3)
    {
        internalFormat = GL_RGB8;
        dataFormat     = GL_RGB;
    }
    else if (texture.GetChannels() == 4)
    {
        internalFormat = GL_RGBA8;
        dataFormat     = GL_RGBA;
    }

    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, m_Width, m_Height, 0, dataFormat, GL_UNSIGNED_BYTE, texture.GetData());
    
    glGenerateMipmap(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void RenderTexture::Bind(uint slot) const
{
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_2D, m_RendererID);
}

void RenderTexture::Unbind() const
{
    glBindTexture(GL_TEXTURE_2D, 0);
}