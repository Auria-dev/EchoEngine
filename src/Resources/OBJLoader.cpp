#include "OBJLoader.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <filesystem>

// temporary helper
struct VertexKey
{
    unsigned int v, vt, vn;

    bool operator==(const VertexKey& other) const
    {
        return v == other.v && vt == other.vt && vn == other.vn;
    }
};

struct VertexKeyHash
{
    std::size_t operator()(const VertexKey& k) const
    {
        return std::hash<unsigned int>()(k.v) ^ (std::hash<unsigned int>()(k.vt) << 1) ^ (std::hash<unsigned int>()(k.vn) << 2);
    }
};

static std::string ParseTexturePath(std::stringstream& ss, const std::string& baseDir)
{
    std::string token;
    std::string filename = "";
    
    while(ss >> token)
    {
        if (token == "-bm") { ss >> token; continue; }                   // Bump multiplier (1 arg)
        if (token == "-o")  { ss >> token >> token >> token; continue; } // Offset (3 args)
        if (token == "-s")  { ss >> token >> token >> token; continue; } // Scale (3 args)
        if (token == "-mm") { ss >> token >> token; continue; }          // Color gain/offset (2 args)
        if (token == "-blendu" || token == "-blendv" || token == "-imfchan" || token == "-texres") { 
            ss >> token; continue; 
        }
        
        if (token[0] == '-') continue;

        filename = token;
        break; 
    }

    if (filename.empty()) return "";

    std::replace(filename.begin(), filename.end(), '\\', '/');

    return baseDir + filename;
}

std::string OBJLoader::GetBaseDir(const std::string& filepath)
{
    std::filesystem::path p(filepath);
    return p.parent_path().string() + "/";
}

void OBJLoader::ParseVertexIndex(const std::string& token, std::vector<unsigned int>& vIdx, std::vector<unsigned int>& vtIdx, std::vector<unsigned int>& vnIdx)
{
    std::stringstream ss(token);
    std::string segment;
    std::vector<std::string> parts;

    while (std::getline(ss, segment, '/'))
    {
        parts.push_back(segment);
    }

    // position index
    if (parts.size() > 0 && !parts[0].empty()) vIdx.push_back(std::stoi(parts[0]));
    else                                       vIdx.push_back(0);

    // texcoord index
    if (parts.size() > 1 && !parts[1].empty()) vtIdx.push_back(std::stoi(parts[1]));
    else                                       vtIdx.push_back(0);

    // normal index
    if (parts.size() > 2 && !parts[2].empty()) vnIdx.push_back(std::stoi(parts[2]));
    else                                       vnIdx.push_back(0);
}

LoadResult OBJLoader::Load(const std::string& filepath)
{
    LoadResult result;
    result.mesh = std::make_shared<Mesh>();
    result.mesh->Filepath = filepath;

    std::ifstream file(filepath);
    if (!file.is_open())
    {
        std::cerr << "OBJLoader Error: Could not open file: " << filepath << std::endl;
        return result;
    }

    std::string baseDir = GetBaseDir(filepath);

    std::vector<glm::vec3> tempPositions;
    std::vector<glm::vec2> tempUVs;
    std::vector<glm::vec3> tempNormals;

    std::vector<unsigned int> vIndices;
    std::vector<unsigned int> vtIndices;
    std::vector<unsigned int> vnIndices;

    std::unordered_map<std::string, int> materialMap;
    std::string currentMtlName = "";
    
    std::unordered_map<VertexKey, unsigned int, VertexKeyHash> uniqueVertices;
    
    std::string line;
    SubMesh currentSubMesh;
    currentSubMesh.BaseIndex = 0;
    currentSubMesh.IndexCount = 0;
    currentSubMesh.MaterialIndex = 0;

    bool firstSubMesh = true;

    while (std::getline(file, line))
    {
        std::stringstream ss(line);
        std::string prefix;
        ss >> prefix;

        if (prefix == "v") // vertex position
        {
            glm::vec3 pos;
            ss >> pos.x >> pos.y >> pos.z;
            tempPositions.push_back(pos);
        }
        else if (prefix == "vt") // texture coordinate
        {
            glm::vec2 uv;
            ss >> uv.x >> uv.y;
            tempUVs.push_back(uv);
        }
        else if (prefix == "vn") // normal
        {
            glm::vec3 norm;
            ss >> norm.x >> norm.y >> norm.z;
            tempNormals.push_back(norm);
        }
        else if (prefix == "usemtl") // material
        {
            std::string matName;
            ss >> matName;

            if (!firstSubMesh)
            {
                result.mesh->SubMeshes.push_back(currentSubMesh);
                
                currentSubMesh.BaseIndex += currentSubMesh.IndexCount;
                currentSubMesh.IndexCount = 0;
            }

            if (materialMap.find(matName) == materialMap.end())
            {
                materialMap[matName] = (int)result.materials.size();
                result.materials.push_back(std::make_shared<Material>()); 
            }
            
            currentSubMesh.MaterialIndex = materialMap[matName];
            currentSubMesh.NodeName = matName;
            firstSubMesh = false;
        }
        else if (prefix == "f") // face
        {
            std::string token;
            std::vector<std::string> faceTokens;
            while (ss >> token) faceTokens.push_back(token);

            // triangulate
            for (size_t i = 1; i < faceTokens.size() - 1; i++)
            {
                ParseVertexIndex(faceTokens[0], vIndices, vtIndices, vnIndices);
                ParseVertexIndex(faceTokens[i], vIndices, vtIndices, vnIndices);
                ParseVertexIndex(faceTokens[i+1], vIndices, vtIndices, vnIndices);
                currentSubMesh.IndexCount += 3;
            }
        }
        else if (prefix == "mtllib") {
            std::string mtlFilename; ss >> mtlFilename;
            std::string mtlPath = baseDir + mtlFilename;

            std::ifstream mtlFile(mtlPath);
            if (mtlFile.is_open()) {
                std::string mtlLine;
                std::shared_ptr<Material> activeMat = nullptr;

                while (std::getline(mtlFile, mtlLine)) {
                    if (mtlLine.empty()) continue;
                    std::stringstream mss(mtlLine);
                    std::string mPrefix; mss >> mPrefix;

                    if (mPrefix == "newmtl") {
                        std::string mName; mss >> mName;
                        if (materialMap.find(mName) == materialMap.end()) {
                            materialMap[mName] = (int)result.materials.size();
                            result.materials.push_back(std::make_shared<Material>());
                        }
                        activeMat = result.materials[materialMap[mName]];
                    }
                    else if (mPrefix == "map_Kd") { // diffuse
                        if (activeMat) {
                            std::string path = ParseTexturePath(mss, baseDir);
                            if(!path.empty()) activeMat->SetDiffuse(Texture(path));
                        }
                    }
                    else if (mPrefix == "map_Bump" || mPrefix == "map_bump") { // normal/bump
                        if (activeMat) {
                            std::string path = ParseTexturePath(mss, baseDir);
                            if(!path.empty()) activeMat->SetNormal(Texture(path));
                        }
                    }
                    else if (mPrefix == "map_Ns" || mPrefix == "map_Pr") { // roughness
                        // Note: OBJ Ns is technically Specular Highlight, but often used for Roughness in PBR
                        if (activeMat) {
                            std::string path = ParseTexturePath(mss, baseDir);
                            if(!path.empty()) activeMat->SetRough(Texture(path));
                        }
                    }
                    else if (mPrefix == "map_d") { } // alpha/opacity 
                }
            } else {
                std::cerr << "OBJLoader Warning: Could not find MTL file " << mtlPath << std::endl;
            }
        }
    }

    result.mesh->SubMeshes.push_back(currentSubMesh);

    for (size_t i = 0; i < vIndices.size(); ++i)
    {
        unsigned int v = vIndices[i];
        unsigned int vt = vtIndices[i];
        unsigned int vn = vnIndices[i];

        VertexKey key = { v, vt, vn };

        if (uniqueVertices.count(key) == 0)
        {
            Vertex newVertex;
            
            if (v > 0) newVertex.Position = tempPositions[v - 1];
            
            if (vt > 0) newVertex.TexCoords = glm::vec3(tempUVs[vt - 1], 0.0f);
            else newVertex.TexCoords = glm::vec3(0.0f);

            if (vn > 0) newVertex.Normal = tempNormals[vn - 1];
            else newVertex.Normal = glm::vec3(0.0f, 1.0f, 0.0f);

            uniqueVertices[key] = (unsigned int)result.mesh->Vertices.size();
            result.mesh->Vertices.push_back(newVertex);
        }

        result.mesh->Indices.push_back(uniqueVertices[key]);
        currentSubMesh.IndexCount++;
    }
        
    result.mesh->RecalculateNormals(); 
    result.mesh->RecalculateTangents();

    std::cout << "==================================================" << std::endl;
    std::cout << " [OBJ DEBUG] Loaded: " << filepath << std::endl;
    std::cout << "--------------------------------------------------" << std::endl;
    std::cout << "  Vertices:  " << result.mesh->Vertices.size() << std::endl;
    std::cout << "  Indices:   " << result.mesh->Indices.size() << std::endl;
    std::cout << "  Materials: " << result.materials.size() << std::endl;
    std::cout << "  SubMeshes: " << result.mesh->SubMeshes.size() << std::endl;
    std::cout << "--------------------------------------------------" << std::endl;

    uint totalIndices = 0;
    for (size_t i = 0; i < result.mesh->SubMeshes.size(); i++)
    {
        const SubMesh& sm = result.mesh->SubMeshes[i];
        std::string matName = sm.NodeName.empty() ? "None" : sm.NodeName;
        
        std::cout << "  [SubMesh " << i << "]" << std::endl;
        std::cout << "    > Material:   " << matName << " (Index: " << sm.MaterialIndex << ")" << std::endl;
        std::cout << "    > IndexCount: " << sm.IndexCount << std::endl;
        std::cout << "    > BaseIndex:  " << sm.BaseIndex << std::endl;
        
        totalIndices += sm.IndexCount;
    }

    std::cout << "--------------------------------------------------" << std::endl;
    
    if (totalIndices != result.mesh->Indices.size())
    {
        std::cerr << " [CRITICAL ERROR] SubMesh count mismatch!" << std::endl;
        std::cerr << " Expected: " << result.mesh->Indices.size() << std::endl;
        std::cerr << " Actual:   " << totalIndices << std::endl;
    }
    else
    {
        std::cout << " [OK] Mesh Integrity Verified." << std::endl;
    }
    std::cout << "==================================================" << std::endl;

    return result;
}