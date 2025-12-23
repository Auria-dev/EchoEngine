#include "Shader.h"

#include <fstream>
#include <sstream>
#include <iostream>
#include <filesystem>

Shader::Shader(const std::string& vertPath, const std::string& fragPath) : m_RendererID(0)
{
    std::optional<std::string> vertexSource = ParseShader(vertPath);
    std::optional<std::string> fragmentSource = ParseShader(fragPath);

    if (!vertexSource || !fragmentSource) return;

    m_RendererID = CreateShader(*vertexSource, *fragmentSource);

    // tell what program id the shader now belongs to (print the name and id)
    std::cout << "Loaded shader program from '" << std::filesystem::path(vertPath).filename().string()
        << "' and '" << std::filesystem::path(fragPath).filename().string()
        << "' with ID: " << m_RendererID << std::endl;
}

Shader::~Shader()
{
    if (m_RendererID != 0) glDeleteProgram(m_RendererID);
}

void Shader::Bind() const
{
    if (m_RendererID != 0) glUseProgram(m_RendererID);
}

void Shader::Unbind() const
{
    glUseProgram(0);
}

bool Shader::IsValid() const
{
    return m_RendererID != 0;
}

std::optional<std::string> Shader::ParseShader(const std::string& filepath)
{
    std::ifstream stream(filepath);
    if (!stream.is_open())
    {
        std::cerr << "failed to open shader file: " << filepath << std::endl;
        return std::nullopt;
    }
    
    std::stringstream buffer;
    buffer << stream.rdbuf();
    stream.close();
    return buffer.str();
}

uint Shader::CompileShader(uint type, const std::string& source)
{
    uint id = glCreateShader(type);
    const char* src = source.c_str();
    glShaderSource(id, 1, &src, nullptr);
    glCompileShader(id);

    int result;
    glGetShaderiv(id, GL_COMPILE_STATUS, &result);
    if (result == GL_FALSE) {
        int length;
        glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);

        std::vector<char> message(length);
        glGetShaderInfoLog(id, length, &length, message.data());

        std::cerr << " failed to compile "
            << (type == GL_VERTEX_SHADER ? "vertex" : "fragment")
            << " shader!\n" << message.data() << std::endl;

        glDeleteShader(id);
        return 0;
    }
    return id;
}

uint Shader::CreateShader(const std::string& vertex_shader, const std::string& fragment_shader)
{
    uint program = glCreateProgram();
    uint vs = CompileShader(GL_VERTEX_SHADER, vertex_shader);
    uint fs = CompileShader(GL_FRAGMENT_SHADER, fragment_shader);

    if (vs == 0 || fs == 0)
    {
        if (vs != 0) glDeleteShader(vs);
        if (fs != 0) glDeleteShader(fs);
        glDeleteProgram(program);
        return 0;
    }

    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);

    int linkResult;
    glGetProgramiv(program, GL_LINK_STATUS, &linkResult);
    if (linkResult == GL_FALSE)
    {
        int length;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &length);
        std::vector<char> message(length);
        glGetProgramInfoLog(program, length, &length, message.data());

        std::cerr << "failed to link shader program!\n"
            << message.data() << std::endl;

        glDeleteProgram(program);
        glDeleteShader(vs);
        glDeleteShader(fs);
        return 0;
    }

    glDeleteShader(vs);
    glDeleteShader(fs);
    return program;
}

int Shader::GetUniformLocation(const std::string& name)
{
    if (m_UniformLocationCache.find(name) != m_UniformLocationCache.end())
        return m_UniformLocationCache[name];

    int location = glGetUniformLocation(m_RendererID, name.c_str());

    if (location == -1) {
        std::cerr << "Warning: uniform '" << name << "' doesn't exist in shader program ID " << m_RendererID << std::endl;
    }

    m_UniformLocationCache[name] = location;
    return location;
}

void Shader::SetUniform1i(const std::string& name, int v)
{
    glUniform1i(GetUniformLocation(name), v);
    m_UniformValueCache[name] = v;
}

void Shader::SetUniform2i(const std::string& name, int v0, int v1)
{
    glUniform2i(GetUniformLocation(name), v0, v1);
    m_UniformValueCache[name] = glm::ivec2(v0, v1);
}

void Shader::SetUniform3i(const std::string& name, int v0, int v1, int v2)
{
    glUniform3i(GetUniformLocation(name), v0, v1, v2);
    m_UniformValueCache[name] = glm::ivec3(v0, v1, v2);
}

void Shader::SetUniform4i(const std::string& name, int v0, int v1, int v2, int v3)
{
    glUniform4i(GetUniformLocation(name), v0, v1, v2, v3);
    m_UniformValueCache[name] = glm::ivec4(v0, v1, v2, v3);
}

void Shader::SetUniform1f(const std::string& name, float v)
{
    glUniform1f(GetUniformLocation(name), v);
    m_UniformValueCache[name] = v;
}

void Shader::SetUniform2f(const std::string& name, float v0, float v1)
{
    glUniform2f(GetUniformLocation(name), v0, v1);
    m_UniformValueCache[name] = glm::vec2(v0, v1);
}

void Shader::SetUniform3f(const std::string& name, float v0, float v1, float v2)
{
    glUniform3f(GetUniformLocation(name), v0, v1, v2);
    m_UniformValueCache[name] = glm::vec3(v0, v1, v2);
}

void Shader::SetUniform4f(const std::string& name, float v0, float v1, float v2, float v3)
{
    glUniform4f(GetUniformLocation(name), v0, v1, v2, v3);
    m_UniformValueCache[name] = glm::vec4(v0, v1, v2, v3);
}

void Shader::SetUniformMat3f(const std::string& name, const glm::mat3& matrix)
{
    glUniformMatrix3fv(GetUniformLocation(name), 1, GL_FALSE, &matrix[0][0]);
    m_UniformValueCache[name] = matrix;
}

void Shader::SetUniformMat4f(const std::string& name, const glm::mat4& matrix)
{
    glUniformMatrix4fv(GetUniformLocation(name), 1, GL_FALSE, &matrix[0][0]);
    m_UniformValueCache[name] = matrix;
}

void Shader::Reload(const std::string& vertexFilepath, const std::string& fragmentFilepath)
{
    std::cout << "Reloading shader: " << vertexFilepath << " and " << fragmentFilepath << std::endl;

    if (m_RendererID != 0)
    {
        glDeleteProgram(m_RendererID);
        m_RendererID = 0;
    }

    m_UniformLocationCache.clear();

    std::optional<std::string> vertexSource = ParseShader(vertexFilepath);
    std::optional<std::string> fragmentSource = ParseShader(fragmentFilepath);
    if (!vertexSource || !fragmentSource) return;

    m_RendererID = CreateShader(*vertexSource, *fragmentSource);

    if (m_RendererID != 0)
    {
        Bind();

        for (const auto& pair : m_UniformValueCache)
        {
            const std::string& name = pair.first;
            const UniformVariant& value = pair.second;
            int location = GetUniformLocation(name);

            if (location == -1) continue;

            std::visit([&](auto&& arg) {
                using T = std::decay_t<decltype(arg)>;
                if constexpr (std::is_same_v<T, int>) {
                    glUniform1i(location, arg);
                }
                else if constexpr (std::is_same_v<T, glm::ivec2>) {
                    glUniform2i(location, arg.x, arg.y);
                }
                else if constexpr (std::is_same_v<T, glm::ivec3>) {
                    glUniform3i(location, arg.x, arg.y, arg.z);
                }
                else if constexpr (std::is_same_v<T, glm::ivec4>) {
                    glUniform4i(location, arg.x, arg.y, arg.z, arg.w);
                }
                else if constexpr (std::is_same_v<T, float>) {
                    glUniform1f(location, arg);
                }
                else if constexpr (std::is_same_v<T, glm::vec2>) {
                    glUniform2f(location, arg.x, arg.y);
                }
                else if constexpr (std::is_same_v<T, glm::vec3>) {
                    glUniform3f(location, arg.x, arg.y, arg.z);
                }
                else if constexpr (std::is_same_v<T, glm::vec4>) {
                    glUniform4f(location, arg.x, arg.y, arg.z, arg.w);
                }
                else if constexpr (std::is_same_v<T, glm::mat3>) {
                    glUniformMatrix3fv(location, 1, GL_FALSE, &arg[0][0]);
                }
                else if constexpr (std::is_same_v<T, glm::mat4>) {
                    glUniformMatrix4fv(location, 1, GL_FALSE, &arg[0][0]);
                }
            }, value);
        }
    }
}