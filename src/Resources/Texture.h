#pragma once

#include <string>
#include <immintrin.h>

#include <glm/glm.hpp>

#include "../Types.h"

class Texture
{

public:
    Texture() = default;
    Texture(const std::string& filepath);
    Texture(int width, int height, int channels);
    Texture(int width, int height, int channels, uchar* buffer);
    ~Texture();

    // prevent copying
    Texture(const Texture&) = delete;
    Texture& operator = (const Texture&) = delete;

    // allow std::move()
    Texture(Texture&& other) noexcept;
    Texture& operator = (Texture&& other) noexcept;

    Texture(glm::vec4 color);

    void Load(const std::string& path);
    void Free();

    uchar* GetData() const { return m_LocalBuffer; }
    int GetWidth() const { return m_Width; }
    int GetHeight() const { return m_Height; }
    int GetChannels() const { return m_Channels; }

private:
    uchar* m_LocalBuffer = nullptr;
    int m_Width = 0, m_Height = 0, m_Channels = 0;

};