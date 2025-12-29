#pragma once

#include <glad/glad.h>

#include <string>

#include "..\Types.h"

class Texture {
private:
	uint m_RendererID;
	std::string m_Filepath;
	uchar* m_LocalBuffer;
	int m_Width, m_Height, m_BPP;
	std::string m_Type;
public:
	Texture();
	Texture(const std::string& filepath);
	Texture(const std::string& filepath, const std::string& type);
	~Texture();

	void Bind(uint slot = 0) const;
	void Unbind() const;

	inline int GetWidth() const { return m_Width; }
	inline int GetHeight() const { return m_Height; }
	inline std::string GetType() const { return m_Type; }
	inline std::string GetFilepath() const { return m_Filepath; }

	void SetType(const std::string& type) { m_Type = type; }
	void SetFilepath(const std::string& filepath) { m_Filepath = filepath; }
	bool IsValid() const;
};