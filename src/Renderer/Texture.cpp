#include "Texture.h"

#include <stb_image.h>
#include <iostream>
#include <filesystem>

Texture::Texture() : m_RendererID(0), m_Filepath(""), m_LocalBuffer(nullptr), m_Width(0), m_Height(0), m_BPP(0) {
	glGenTextures(1, &m_RendererID);
	glBindTexture(GL_TEXTURE_2D, m_RendererID);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_Width, m_Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, m_LocalBuffer);
	glGenerateMipmap(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);
}

Texture::Texture(const std::string& filepath) : m_RendererID(0), m_Filepath(filepath), m_LocalBuffer(nullptr), m_Width(0), m_Height(0), m_BPP(0) {
	stbi_set_flip_vertically_on_load(1);

	m_LocalBuffer = stbi_load(filepath.c_str(), &m_Width, &m_Height, &m_BPP, 4);
	if (!m_LocalBuffer) {
		std::cerr << "Failed to load texture: " << filepath << std::endl;
		return;
	}

	glGenTextures(1, &m_RendererID);
	std::cout << "Generated Texture ID: " << m_RendererID << " for texture: " << std::filesystem::path(filepath).filename().string() << std::endl;

	glBindTexture(GL_TEXTURE_2D, m_RendererID);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_Width, m_Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, m_LocalBuffer);
	glGenerateMipmap(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);

	if (m_LocalBuffer) {
		stbi_image_free(m_LocalBuffer);
		m_LocalBuffer = nullptr;
	}
}

Texture::Texture(const std::string& filepath, const std::string& type) : Texture(filepath) {
	m_Type = type;
}

Texture::~Texture() {
	glDeleteTextures(1, &m_RendererID);
}

void Texture::Bind(uint slot) const {
	glActiveTexture(GL_TEXTURE0 + slot);
	glBindTexture(GL_TEXTURE_2D, m_RendererID);
}

void Texture::Unbind() const {
	glBindTexture(GL_TEXTURE_2D, 0);
}

bool Texture::IsValid() const {
	return m_RendererID != 0; // assuming m_RendererID is 0 if loading failed
}