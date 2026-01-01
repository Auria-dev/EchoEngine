#pragma once

#include "Types.h"

class FullscreenQuad
{

public:
	FullscreenQuad();
	~FullscreenQuad();

	void Init();
	void Draw() const;
private:
	uint m_VAO = 0;
	uint m_VBO = 0;
	uint m_EBO = 0;

};