#pragma once

#include <glad/glad.h>

#include <string>

#include "Types.h"
#include "Resources/Material.h"

class RenderTexture
{

public:
    RenderTexture() = default;
    
    RenderTexture(const Texture& texture);
    
    ~RenderTexture();

    void Bind(uint slot = 0) const;
    void Unbind() const;

    inline int GetWidth() const { return m_Width; }
    inline int GetHeight() const { return m_Height; }
    inline uint GetID() const { return m_RendererID; }

private:
    uint m_RendererID = 0;
    int m_Width = 0, m_Height = 0;
    
    void CreateTexture(const Texture& texture);

};