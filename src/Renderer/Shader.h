#pragma once

#include <string>
#include <unordered_map>
#include <optional>
#include <iostream>
#include <variant>
#include <fstream>
#include <sstream>
#include <iostream>
#include <filesystem>

#include <glad/glad.h>
#include <glm/glm.hpp>

#include "..\Types.h"

using UniformVariant = std::variant<
    int,
    glm::ivec2,
    glm::ivec3,
    glm::ivec4,
    float,
	glm::vec2,
    glm::vec3,
    glm::vec4,
    glm::mat3,
    glm::mat4
>;

class Shader
{

public:
    Shader(const std::string& vertPath, const std::string& fragPath);
    ~Shader();

    void Bind() const;
    void Unbind() const;
    bool IsValid() const;

    void SetUniform1i(const std::string& name, int v);
    void SetUniform2i(const std::string& name, int v0, int v1);
    void SetUniform3i(const std::string& name, int v0, int v1, int v2);
    void SetUniform4i(const std::string& name, int v0, int v1, int v2, int v3);

    void SetUniform1f(const std::string& name, float v);
    void SetUniform2f(const std::string& name, float v0, float v1);
    void SetUniform3f(const std::string& name, float v0, float v1, float v2);
    void SetUniform4f(const std::string& name, float v0, float v1, float v2, float v3);

    void SetUniformMat3f(const std::string& name, const glm::mat3& matrix);
    void SetUniformMat4f(const std::string& name, const glm::mat4& matrix);

    inline uint GetRendererID() { return m_RendererID; }

    void Reload(const std::string& vertPath, const std::string& fragPath);

private:
    uint m_RendererID;

    std::unordered_map<std::string, int> m_UniformLocationCache;
    std::unordered_map<std::string, UniformVariant> m_UniformValueCache;

    uint CompileShader(uint type, const std::string& source);
    uint CreateShader(const std::string& vert, const std::string& frag);

    std::optional<std::string> ParseShader(const std::string& filepath);
    int GetUniformLocation(const std::string& name);
};