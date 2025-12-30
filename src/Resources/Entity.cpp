#include "Entity.h"

void Entity::UpdateTransform()
{
    // Scale -> Rotate -> Translate
    glm::mat4 trans = glm::mat4(1.0f);
    trans = glm::translate(trans, position);
    trans = glm::rotate(trans, glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
    trans = glm::rotate(trans, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
    trans = glm::rotate(trans, glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
    trans = glm::scale(trans, scale);
    transform = trans;
}

void Entity::SetPosition(const glm::vec3& pos)
{
    position = pos;
    UpdateTransform();
}

void Entity::SetRotation(const glm::vec3& rot)
{
    rotation = rot;
    UpdateTransform();
}

void Entity::SetScale(const glm::vec3& scl)
{
    scale = scl;
    UpdateTransform();
}

void Entity::Translate(const glm::vec3& delta)
{
    position += delta;
    UpdateTransform();
}

void Entity::Rotate(const glm::vec3& delta)
{
    rotation += delta;
    UpdateTransform();
}

void Entity::Scale(const glm::vec3& factor)
{
    scale *= factor;
    UpdateTransform();
}