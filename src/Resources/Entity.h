#pragma once

#include <memory>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

#include "Mesh.h"
#include "Material.h"

class Entity
{

public:
    std::shared_ptr<Mesh> meshAsset;
    std::vector<std::shared_ptr<Material>> materials;

    glm::mat4 transform;
	glm::vec3 position;
	glm::vec3 rotation;
	glm::vec3 scale;

	void SetPosition(const glm::vec3& pos);
	void SetRotation(const glm::vec3& rot);
	void SetScale(const glm::vec3& scl);

	void Translate(const glm::vec3& delta);
	void Rotate(const glm::vec3& delta);
	void Scale(const glm::vec3& factor);

private:
	void UpdateTransform();

};