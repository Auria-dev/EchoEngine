#pragma once

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
    unsigned int BaseIndex = 0;
    unsigned int IndexCount = 0;
    unsigned int MaterialIndex = 0;

	std::string NodeName; 
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

    void RecalculateTangents();

};