#include "Mesh.h"

void Mesh::CalculateSubMeshBoundsAndCenter(SubMesh& sm, const Mesh& mesh)
{
    glm::vec3 min(FLT_MAX), max(-FLT_MAX);

    for (uint i = 0; i < sm.IndexCount; ++i)
    {
        uint index = mesh.Indices[sm.BaseIndex + i];
        const glm::vec3& p = mesh.Vertices[index].Position;
        min = glm::min(min, p);
        max = glm::max(max, p);
    }

    sm.LocalCenter = (min + max) * 0.5f;
    sm.LocalRadius = glm::length(max - sm.LocalCenter);
}

Mesh::Mesh() {}
Mesh::~Mesh() {}

void Mesh::RecalculateNormals()
{
    for (auto& v : Vertices)
    {
        v.Normal = glm::vec3(0.0f);
    }

    for (size_t i = 0; i < Indices.size(); i += 3)
    {
        Vertex& v0 = Vertices[Indices[i]];
        Vertex& v1 = Vertices[Indices[i + 1]];
        Vertex& v2 = Vertices[Indices[i + 2]];

        glm::vec3 edge1 = v1.Position - v0.Position;
        glm::vec3 edge2 = v2.Position - v0.Position;
        
        glm::vec3 normal = glm::cross(edge1, edge2);

        v0.Normal += normal;
        v1.Normal += normal;
        v2.Normal += normal;
    }

    for (auto& v : Vertices)
    {
        v.Normal = glm::normalize(v.Normal);
    }
}

void Mesh::RecalculateTangents()
{
    for (auto& v : Vertices)
    {
        v.Tangent = glm::vec3(0.0f);
        v.Bitangent = glm::vec3(0.0f);
    }

    for (size_t i = 0; i < Indices.size(); i += 3)
    {
        Vertex& v0 = Vertices[Indices[i]];
        Vertex& v1 = Vertices[Indices[i + 1]];
        Vertex& v2 = Vertices[Indices[i + 2]];

        glm::vec3 edge1 = v1.Position - v0.Position;
        glm::vec3 edge2 = v2.Position - v0.Position;

        glm::vec2 deltaUV1 = glm::vec2(v1.TexCoords) - glm::vec2(v0.TexCoords);
        glm::vec2 deltaUV2 = glm::vec2(v2.TexCoords) - glm::vec2(v0.TexCoords);

        // f = 1 / (du1 * dv2 - du2 * dv1)
        float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

        if (std::isinf(f) || std::isnan(f)) 
            continue;

        glm::vec3 tangent;
        tangent.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
        tangent.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
        tangent.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);

        glm::vec3 bitangent;
        bitangent.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
        bitangent.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
        bitangent.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);

        v0.Tangent += tangent;
        v1.Tangent += tangent;
        v2.Tangent += tangent;

        v0.Bitangent += bitangent;
        v1.Bitangent += bitangent;
        v2.Bitangent += bitangent;
    }

    for (auto& v : Vertices)
    {
        if (glm::length(v.Tangent) < 0.0001f)
        {
            v.Tangent = glm::vec3(1.0f, 0.0f, 0.0f);
            v.Bitangent = glm::vec3(0.0f, 1.0f, 0.0f);
            continue;
        }

        glm::vec3 T = glm::normalize(v.Tangent - v.Normal * glm::dot(v.Normal, v.Tangent));
        glm::vec3 B = glm::cross(v.Normal, T);

        float handedness = (glm::dot(B, v.Bitangent) < 0.0f) ? -1.0f : 1.0f;

        v.Tangent = T;
        v.Bitangent = B * handedness;
    }
}