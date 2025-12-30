#include "Mesh.h"

Mesh::Mesh() {}
Mesh::~Mesh() {}

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
    }

    for (auto& v : Vertices)
    {
        if (glm::length(v.Tangent) < 0.0001f)
        {
            v.Tangent = glm::vec3(1.0f, 0.0f, 0.0f);
            v.Bitangent = glm::vec3(0.0f, 1.0f, 0.0f);
            continue;
        }

        // T_new = Normalize(T - N * (N dot T))
        v.Tangent = glm::normalize(v.Tangent - v.Normal * glm::dot(v.Normal, v.Tangent));
        v.Bitangent = glm::cross(v.Normal, v.Tangent);
    }
}