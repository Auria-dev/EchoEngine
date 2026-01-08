#pragma once

#include "Types.h"

#include <glm/glm.hpp>
#include <vector>
#include <string>

// #define MAX_BONE_INFLUENCE 8 

struct Vertex
{
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec2 TexCoords;
    glm::vec3 Tangent;
    glm::vec3 Bitangent;
    // int BoneIDs[MAX_BONE_INFLUENCE];
    // float Weights[MAX_BONE_INFLUENCE];
};

struct SubMesh
{
    uint BaseIndex = 0;
    uint IndexCount = 0;
    uint MaterialIndex = 0;

	std::string NodeName; 
    
    float LocalRadius;
    glm::vec3 LocalCenter;
};

class Mesh
{

public:
    Mesh();
    ~Mesh();

    std::vector<Vertex> Vertices;
    std::vector<unsigned int> Indices;

    std::vector<SubMesh> SubMeshes;

    std::string Name;
    std::string Filepath;

    void RecalculateNormals();
    void RecalculateTangents();
    
    void CalculateSubMeshBoundsAndCenter(SubMesh& sm, const Mesh& mesh);
};