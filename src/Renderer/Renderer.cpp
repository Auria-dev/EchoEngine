#include "Renderer.h"
#include <iostream>

void Renderer::DrawEntity(const Entity& entity, Shader& shader)
{
    if (!entity.meshAsset) return;

    shader.Bind();
    shader.SetUniformMat4f("uModel", entity.transform);

    if (m_MeshCache.find(entity.meshAsset.get()) == m_MeshCache.end())
    {
        m_MeshCache[entity.meshAsset.get()] = std::make_unique<MeshResource>(*entity.meshAsset);
    }
    
    MeshResource* mesh = m_MeshCache[entity.meshAsset.get()].get();

    mesh->Bind();

    const auto& subMeshes = entity.meshAsset->SubMeshes;

    for (int i = 0; i < subMeshes.size(); ++i)
    {
        const SubMesh& subMesh = subMeshes[i];
        
        if (subMesh.MaterialIndex < entity.materials.size())
        {
            auto cpuMat = entity.materials[subMesh.MaterialIndex];

            if (cpuMat->DiffuseTexture)
            {
                RenderTexture* tex = GetGPUTexture(cpuMat->DiffuseTexture.get());
                tex->Bind(0); 
            }
            
            if (cpuMat->NormalTexture)
            {
                RenderTexture* tex = GetGPUTexture(cpuMat->NormalTexture.get());
                tex->Bind(1);
            }

            if (cpuMat->ARMTexture)
            {
                RenderTexture* tex = GetGPUTexture(cpuMat->ARMTexture.get());
                tex->Bind(2);
            }
        }

        mesh->DrawSubMesh(i);
    }
}

RenderTexture* Renderer::GetGPUTexture(const Texture* cpuTexture)
{
    if (m_TextureCache.find(cpuTexture) == m_TextureCache.end())
    {
        m_TextureCache[cpuTexture] = std::make_unique<RenderTexture>(*cpuTexture);
    }

    return m_TextureCache[cpuTexture].get();
}

void Renderer::ClearCache()
{
    m_MeshCache.clear();
    m_TextureCache.clear();
}