#pragma once

#include "Types.h"
#include "Camera.h"
#include "Lightsource.h"
#include "Resources/Entity.h"

class SceneData
{

public:
    Camera* activeCamera;
    
	// std::vector<Camera*> Camera;
	std::vector<Entity*> m_Entities;
	std::vector<Light*> m_Lights;
	DirectionalLight m_Sun;
};