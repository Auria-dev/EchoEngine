#pragma once

#include <unordered_map>
#include <memory>

#include "Resources/Entity.h"
#include "MeshResource.h"
#include "RenderTexture.h"
#include "Shader.h"

class Renderer
{

public:
    Renderer() = default;
    
    void DrawEntity(const Entity& entity, Shader& shader);
    void ClearCache();

private:
    std::unordered_map<const Mesh*, std::unique_ptr<MeshResource>> m_MeshCache;
    std::unordered_map<const Texture*, std::unique_ptr<RenderTexture>> m_TextureCache;
    RenderTexture* GetGPUTexture(const Texture* cpuTexture);

};