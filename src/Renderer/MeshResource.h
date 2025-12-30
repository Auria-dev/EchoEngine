#pragma once

#include <memory>

#include "Buffer.h"
#include "Resources/Mesh.h"
#include "RenderTexture.h"

class MeshResource
{

public:
    MeshResource(const Mesh& mesh);
    ~MeshResource();

    void Bind() const;
    
    void DrawSubMesh(int subMeshIndex);

private:
    std::unique_ptr<VertexArray> m_VA;
    std::unique_ptr<VertexBuffer> m_VB;
    std::unique_ptr<IndexBuffer> m_IB;
    
    std::vector<SubMesh> m_SubMeshes;

};

struct RenderEntity
{
    std::unique_ptr<MeshResource> MeshRes;
    
    struct GPUMaterial {
        std::unique_ptr<RenderTexture> Diffuse;
        std::unique_ptr<RenderTexture> Normal;
        std::unique_ptr<RenderTexture> ARM;
    };
    std::vector<GPUMaterial> Materials;
};