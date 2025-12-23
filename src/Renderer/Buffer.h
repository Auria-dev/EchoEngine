#pragma once

#include <glad/glad.h>

#include <vector>

#include "..\Types.h"


struct VertexBufferElement 
{
    uint type, count;
    uchar normalized;

    static uint GetSizeOfType(uint type)
    {
        switch (type) {
			case GL_FLOAT:			return sizeof(GLfloat);
			case GL_INT:			return sizeof(GLint);
			case GL_UNSIGNED_INT:	return sizeof(GLuint);
			case GL_UNSIGNED_BYTE:	return sizeof(GLubyte);
		}
		return 0;
    }
};


class VertexBufferLayout
{

public:
	VertexBufferLayout() : m_Stride(0) {}

	template<typename T>
	void Push(uint count)
    {
		static_assert(false);
	}

	inline const std::vector<VertexBufferElement>& GetElements() const
    {
        return m_Elements;
    }
	
    inline uint GetStride() const 
    {
        return m_Stride;
    }

private:
	std::vector<VertexBufferElement> m_Elements;
	uint m_Stride;

};

template<>
inline void VertexBufferLayout::Push<float>(uint count)
{
	m_Elements.push_back({ GL_FLOAT, count, GL_FALSE });
	m_Stride += count * VertexBufferElement::GetSizeOfType(GL_FLOAT);
}

template<>
inline void VertexBufferLayout::Push<int>(uint count)
{
	m_Elements.push_back({ GL_INT, count, GL_FALSE });
	m_Stride += count * VertexBufferElement::GetSizeOfType(GL_INT);
}

template<>
inline void VertexBufferLayout::Push<uint>(uint count)
{
	m_Elements.push_back({ GL_UNSIGNED_INT, count, GL_FALSE });
	m_Stride += count * VertexBufferElement::GetSizeOfType(GL_UNSIGNED_INT);
}

template<>
inline void VertexBufferLayout::Push<uchar>(uint count)
{
	m_Elements.push_back({ GL_UNSIGNED_BYTE, count, GL_TRUE });
	m_Stride += count * VertexBufferElement::GetSizeOfType(GL_UNSIGNED_BYTE);
}


class VertexBuffer {

public:
    VertexBuffer(const void* data, uint size, uint usageHint = GL_STATIC_DRAW);
    VertexBuffer(uint size);
    ~VertexBuffer();

    void Bind() const;
    void Unbind() const;
    
    void SetData(const void* data, uint size, uint offset = 0);

    inline uint GetRendererID() const { return m_RendererID; }

private:
    uint m_RendererID;

};


class IndexBuffer {

public:
    IndexBuffer(const uint* data, uint count);
    ~IndexBuffer();

    void Bind() const;
    void Unbind() const;

    inline uint GetCount() const { return m_Count; }

private:
    uint m_RendererID;
    uint m_Count;

};


class VertexArray
{

public:
    VertexArray();
    ~VertexArray();

    void Bind() const;
    void Unbind() const;

    void AddBuffer(const VertexBuffer& vb, const VertexBufferLayout& layout);
    void SetIndexBuffer(const IndexBuffer& ib);

private:
    uint m_RendererID;

};