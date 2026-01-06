#pragma once

#include <string>
#include <vector>
#include <memory>
#include "Mesh.h"
#include "Material.h"

struct LoadResult
{
    std::shared_ptr<Mesh> mesh;
    std::vector<std::shared_ptr<Material>> materials;
};

class OBJLoader
{

public:
    static LoadResult Load(const std::string& filepath);

private:
    static std::string GetBaseDir(const std::string& filepath);
    
    static void ParseMTL(const std::string& filepath, std::vector<std::shared_ptr<Material>>& materials, std::unordered_map<std::string, int>& matMap);

    static void ParseVertexIndex(
        const std::string& token, 
        std::vector<unsigned int>& vertexIndices, 
        std::vector<unsigned int>& uvIndices, 
        std::vector<unsigned int>& normalIndices
    );

};