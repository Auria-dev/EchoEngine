#include "MeshResource.h"
#include <glad/glad.h>

#include <iostream>

MeshResource::MeshResource(const Mesh& mesh)
{
    m_SubMeshes = mesh.SubMeshes;

    m_VA = std::make_unique<VertexArray>();

    m_VB = std::make_unique<VertexBuffer>(
        mesh.Vertices.data(), 
        mesh.Vertices.size() * sizeof(Vertex)
    );

    VertexBufferLayout layout;
    layout.Push<float>(3); // Position
    layout.Push<float>(3); // Normal
    layout.Push<float>(2); // TexCoords
    layout.Push<float>(3); // Tangent
    layout.Push<float>(3); // Bitangent
    m_VA->AddBuffer(*m_VB, layout);

    m_IB = std::make_unique<IndexBuffer>(
        mesh.Indices.data(), 
        mesh.Indices.size()
    );
}

MeshResource::~MeshResource() {}

void MeshResource::Bind() const
{
    m_VA->Bind();
    m_IB->Bind();
}

void MeshResource::DrawSubMesh(int index)
{
    if (index < 0 || index >= m_SubMeshes.size()) return;
    
    const SubMesh& sm = m_SubMeshes[index];
    
    void* offset = (void*)(sm.BaseIndex * sizeof(unsigned int));
    
    glDrawElements(GL_TRIANGLES, sm.IndexCount, GL_UNSIGNED_INT, offset);
}