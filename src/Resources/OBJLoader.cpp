#include "OBJLoader.h"

#include <chrono>
#include <iostream>
#include <fstream>
#include <vector>
#include <unordered_map>
#include <filesystem>
#include <charconv>
#include <cstring>

struct VertexKey
{
    int v, vt, vn;

    bool operator==(const VertexKey& other) const
    {
        return v == other.v && vt == other.vt && vn == other.vn;
    }
};

struct VertexKeyHash
{
    std::size_t operator()(const VertexKey& k) const
    {
        std::size_t seed = 0;
        seed ^= std::hash<int>()(k.v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        seed ^= std::hash<int>()(k.vt) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        seed ^= std::hash<int>()(k.vn) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        return seed;
    }
};

static int ParseInt(const char*& p, const char* end)
{
    while (p < end && (*p == ' ' || *p == '/')) p++;
    if (p >= end) return 0;

    int val = 0;
    int sign = 1;
    
    if (*p == '-')
    {
        sign = -1;
        p++;
    }

    while (p < end && *p >= '0' && *p <= '9')
    {
        val = val * 10 + (*p - '0');
        p++;
    }
    return val * sign;
}

static float ParseFloat(const char*& p, const char* end)
{
    while (p < end && *p == ' ') p++;
    if (p >= end) return 0.0f;

    float val = 0.0f;
    auto res = std::from_chars(p, end, val);
    
    p = res.ptr; 
    return val;
}

static void SkipLine(const char*& p, const char* end)
{
    while (p < end && *p != '\n') p++;
    if (p < end) p++;
}

static void SkipSpace(const char*& p, const char* end)
{
    while (p < end && (*p == ' ' || *p == '\t')) p++;
}

static std::string ParseStringToken(const char*& p, const char* end)
{
    SkipSpace(p, end);
    const char* start = p;
    while (p < end && *p != ' ' && *p != '\t' && *p != '\n' && *p != '\r') p++;
    return std::string(start, p - start);
}

static std::string ParseTexturePath(const char*& p, const char* end, const std::string& baseDir)
{
    SkipSpace(p, end);
    const char* lineEnd = p;
    while (lineEnd < end && *lineEnd != '\n' && *lineEnd != '\r') lineEnd++;
    
    std::string line(p, lineEnd - p);
    p = lineEnd;

    std::stringstream ss(line);
    std::string token, filename;
    while(ss >> token) {
        if(token[0] == '-')
        {
             if (token == "-bm") ss >> token;
             else if (token == "-o" || token == "-s") { ss >> token >> token; }
             continue;
        }
        filename = token;
    }
    
    if (filename.empty()) return "";
    std::replace(filename.begin(), filename.end(), '\\', '/');
    return baseDir + filename;
}

void OBJLoader::ParseMTL(const std::string& filepath, std::vector<std::shared_ptr<Material>>& materials, std::unordered_map<std::string, int>& matMap)
{
    std::ifstream file(filepath);
    if (!file.is_open()) return;

    std::string baseDir = GetBaseDir(filepath);
    std::string line;
    std::shared_ptr<Material> activeMat = nullptr;

    while (std::getline(file, line))
    {
        if (line.empty()) continue;
        
        const char* p = line.c_str();
        const char* end = p + line.size();
        
        SkipSpace(p, end);
        if (p == end || *p == '#') continue;

        if (strncmp(p, "newmtl", 6) == 0)
        {
            p += 6;
            std::string name = ParseStringToken(p, end);
            if (matMap.find(name) == matMap.end())
            {
                matMap[name] = (int)materials.size();
                materials.push_back(std::make_shared<Material>());
            }
            activeMat = materials[matMap[name]];
            activeMat->SetNormal(Texture(glm::vec4(0.5f, 0.5f, 1.0f, 1.0f)));
            activeMat->SetRough(Texture(glm::vec4(1.0f, 1.0f, 0.0f, 1.0f)));
        }
        else if (activeMat) {
            if (strncmp(p, "map_Kd", 6) == 0)
            {
                p += 6;
                std::string path = ParseTexturePath(p, end, baseDir);
                if (!path.empty()) activeMat->SetDiffuse(Texture(path));
            }
            else if (strncmp(p, "map_Bump", 8) == 0 || strncmp(p, "map_bump", 8) == 0)
            {
                p += 8;
                std::string path = ParseTexturePath(p, end, baseDir);
                if (!path.empty()) activeMat->SetNormal(Texture(path));
            }
            else if (strncmp(p, "map_Ns", 6) == 0 || strncmp(p, "map_Pr", 6) == 0)
            {
                p += 6;
                std::string path = ParseTexturePath(p, end, baseDir);
                if (!path.empty()) activeMat->SetRough(Texture(path));
            }
            else if (strncmp(p, "Kd", 2) == 0)
            {
                p += 2;
                if (!activeMat->DiffuseTexture)
                {
                    float r = ParseFloat(p, end);
                    float g = ParseFloat(p, end);
                    float b = ParseFloat(p, end);
                    activeMat->SetDiffuse(Texture(glm::vec4(r, g, b, 1.0f)));
                }
            }
        }
    }
}

std::string OBJLoader::GetBaseDir(const std::string& filepath)
{
    std::filesystem::path p(filepath);
    return p.parent_path().string() + "/";
}

void OBJLoader::ParseVertexIndex(const std::string& token, std::vector<unsigned int>& vIdx, std::vector<unsigned int>& vtIdx, std::vector<unsigned int>& vnIdx) {}

LoadResult OBJLoader::Load(const std::string& filepath)
{
    std::cout << " [OBJ DEBUG] Loading (Optimized): " << filepath << std::endl;
    auto start_time = std::chrono::steady_clock::now();

    LoadResult result;
    result.mesh = std::make_shared<Mesh>();
    result.mesh->Filepath = filepath;

    std::ifstream file(filepath, std::ios::binary | std::ios::ate);
    if (!file.is_open())
    {
        std::cerr << "OBJLoader Error: Could not open file: " << filepath << std::endl;
        return result;
    }
    
    std::streamsize fileSize = file.tellg();
    file.seekg(0, std::ios::beg);

    if (fileSize == 0) return result;

    std::vector<char> buffer(fileSize);
    if (!file.read(buffer.data(), fileSize))
    {
        std::cerr << "OBJLoader Error: Failed to read file." << std::endl;
        return result;
    }
    file.close();

    size_t estimatedVerts = fileSize / 64; 
    std::vector<glm::vec3> tempPositions; tempPositions.reserve(estimatedVerts);
    std::vector<glm::vec2> tempUVs;       tempUVs.reserve(estimatedVerts);
    std::vector<glm::vec3> tempNormals;   tempNormals.reserve(estimatedVerts);

    result.mesh->Vertices.reserve(estimatedVerts);
    result.mesh->Indices.reserve(estimatedVerts * 3);

    std::unordered_map<std::string, int> materialMap;
    std::string currentMtlName = "";
    std::string baseDir = GetBaseDir(filepath);

    std::unordered_map<VertexKey, unsigned int, VertexKeyHash> uniqueVertices;
    uniqueVertices.reserve(estimatedVerts);

    SubMesh currentSubMesh;
    currentSubMesh.BaseIndex = 0;
    currentSubMesh.IndexCount = 0;
    currentSubMesh.MaterialIndex = 0;
    bool firstSubMesh = true;

    const char* p = buffer.data();
    const char* end = p + fileSize;

    while (p < end)
    {
        while (p < end && (*p == ' ' || *p == '\t')) p++;
        if (p >= end) break;

        char c = *p;
        
        if (c == '#') SkipLine(p, end);
        else if (c == 'v') 
        {
            p++;
            if (*p == ' ') // Position
            {
                float x = ParseFloat(p, end);
                float y = ParseFloat(p, end);
                float z = ParseFloat(p, end);
                tempPositions.emplace_back(x, y, z);
            }
            else if (*p == 't') // Texture
            {
                p++;
                float u = ParseFloat(p, end);
                float v = ParseFloat(p, end);
                tempUVs.emplace_back(u, v);
            }
            else if (*p == 'n') // Normal
            {
                p++;
                float x = ParseFloat(p, end);
                float y = ParseFloat(p, end);
                float z = ParseFloat(p, end);
                tempNormals.emplace_back(x, y, z);
            }
            else 
            {
                SkipLine(p, end);
            }
        }
        else if (c == 'f') // Face
        {
            p++;
            unsigned int faceVIndices[16]; 
            int faceVertCount = 0;

            while (p < end && *p != '\n' && *p != '\r')
            {
                SkipSpace(p, end);
                if (*p == '\n' || *p == '\r') break;

                int v = 0, vt = 0, vn = 0;

                v = ParseInt(p, end);

                if (p < end && *p == '/')
                {
                    p++;
                    if (*p != '/')
                    {
                        vt = ParseInt(p, end);
                    }
                    if (p < end && *p == '/')
                    {
                        p++;
                        vn = ParseInt(p, end);
                    }
                }

                if (v < 0)  v  = (int)tempPositions.size() + v + 1;
                if (vt < 0) vt = (int)tempUVs.size() + vt + 1;
                if (vn < 0) vn = (int)tempNormals.size() + vn + 1;

                VertexKey key = { v, vt, vn };

                auto it = uniqueVertices.find(key);
                unsigned int index;

                if (it != uniqueVertices.end())
                {
                    index = it->second;
                }
                else
                {
                    Vertex newVertex;
                    
                    if (v  > 0 && v  <= (int)tempPositions.size()) newVertex.Position = tempPositions[v - 1];
                    else                                           newVertex.Position = glm::vec3(0.0f);

                    if (vt > 0 && vt <= (int)tempUVs.size())       newVertex.TexCoords = glm::vec3(tempUVs[vt - 1], 0.0f);
                    else                                           newVertex.TexCoords = glm::vec3(0.0f);

                    if (vn > 0 && vn <= (int)tempNormals.size())   newVertex.Normal = tempNormals[vn - 1];
                    else                                           newVertex.Normal = glm::vec3(0.0f, 1.0f, 0.0f);

                    index = (unsigned int)result.mesh->Vertices.size();
                    result.mesh->Vertices.push_back(newVertex);
                    uniqueVertices[key] = index;
                }

                if (faceVertCount < 16)
                {
                    faceVIndices[faceVertCount++] = index;
                }
            }

            for (int i = 1; i < faceVertCount - 1; i++)
            {
                result.mesh->Indices.push_back(faceVIndices[0]);
                result.mesh->Indices.push_back(faceVIndices[i]);
                result.mesh->Indices.push_back(faceVIndices[i + 1]);
                currentSubMesh.IndexCount += 3;
            }
        }
        else if (c == 'u') // usemtl
        {
            if (strncmp(p, "usemtl", 6) == 0)
            {
                p += 6;
                std::string matName = ParseStringToken(p, end);

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
            else SkipLine(p, end);
        }
        else if (c == 'm') // mtllib
        {
            if (strncmp(p, "mtllib", 6) == 0)
            {
                p += 6;
                std::string mtlName = ParseStringToken(p, end);
                ParseMTL(baseDir + mtlName, result.materials, materialMap);
            }
            else SkipLine(p, end);
        }
        else
        {
            SkipLine(p, end);
        }
    }

    result.mesh->SubMeshes.push_back(currentSubMesh);

    result.mesh->RecalculateNormals();
    result.mesh->RecalculateTangents();

    auto end_time = std::chrono::steady_clock::now();
    long long ms = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();

    std::cout << "==================================================" << std::endl;
    std::cout << " [OBJ DEBUG] Done! Took " << ms / 60000 << "m " << (ms / 1000) % 60 << "s " << ms % 1000 << "ms" << std::endl; 
    std::cout << "  Vertices:  " << result.mesh->Vertices.size() << std::endl;
    std::cout << "  Indices:   " << result.mesh->Indices.size() << std::endl;
    std::cout << "==================================================" << std::endl;

    return result;
}