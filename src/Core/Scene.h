#pragma once

#include "Types.h"
#include "Camera.h"
#include "Resources/Entity.h"

class SceneData
{

public:
    Camera* activeCamera;
    
	// std::vector<Camera*> Camera;
	std::vector<Entity*> Entities;

};