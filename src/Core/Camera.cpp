#include "Camera.h"

#include <iostream>

Camera::Camera(glm::vec3 position, float yaw, float pitch, float speed, float sensitivity, float fov) : 
    m_Position(position),
    m_WorldUp(glm::vec3(0.0f, 1.0f, 0.0f)),
    m_Yaw(yaw),
    m_Pitch(pitch),
    m_Front(glm::vec3(0.0f, 0.0f, -1.0f)),
    m_MovementSpeed(speed),
    m_MouseSensitivity(sensitivity),
    m_FOV(fov)
{
    UpdateCameraVectors();
}

glm::mat4 Camera::GetViewMatrix() const
{
    return glm::lookAt(m_Position, m_Position + m_Front, m_Up);
}

void Camera::SetProjectionMatrix(float aspectRatio, float nearPlane, float farPlane)
{
    m_ProjectionMatrix = glm::perspective(glm::radians(m_FOV), aspectRatio, nearPlane, farPlane);
}

void Camera::ProcessKeyboard(CameraMovement direction, float dt)
{
    float velocity = m_MovementSpeed * dt;
    if (direction == CameraMovement::FORWARD)  m_Position += glm::normalize(glm::vec3(m_Front.x, 0.0f, m_Front.z)) * velocity;
    if (direction == CameraMovement::BACKWARD) m_Position -= glm::normalize(glm::vec3(m_Front.x, 0.0f, m_Front.z)) * velocity;
    if (direction == CameraMovement::LEFT)     m_Position -= glm::normalize(glm::vec3(m_Right.x, 0.0f, m_Right.z)) * velocity;
    if (direction == CameraMovement::RIGHT)    m_Position += glm::normalize(glm::vec3(m_Right.x, 0.0f, m_Right.z)) * velocity;
    if (direction == CameraMovement::UP)       m_Position += m_WorldUp * velocity;
    if (direction == CameraMovement::DOWN)     m_Position -= m_WorldUp * velocity;
}

void Camera::ProcessMouseMovement(float xoffset, float yoffset, bool constrainPitch)
{
    xoffset *= m_MouseSensitivity;
    yoffset *= m_MouseSensitivity;

    m_Yaw   += xoffset;
    m_Pitch += yoffset;

    if (constrainPitch) {
        if (m_Pitch >  89.0f) m_Pitch =  89.0f;
        if (m_Pitch < -89.0f) m_Pitch = -89.0f;
    }

	if (m_Yaw > 360.0f) m_Yaw -= 360.0f;
	if (m_Yaw < 0.0f)   m_Yaw += 360.0f;

    UpdateCameraVectors();
}

void Camera::ProcessMouseScroll(float yoffset)
{
    m_FOV -= yoffset;

    if (m_FOV <  1.0f) m_FOV = 1.0f;
    if (m_FOV > 90.0f) m_FOV = 90.0f;
}

void Camera::UpdateCameraVectors() {
    glm::vec3 front;
    front.x = cos(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch));
    front.y = sin(glm::radians(m_Pitch));
    front.z = sin(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch));

    m_Front = glm::normalize(front);
    m_Right = glm::normalize(glm::cross(m_Front, m_WorldUp));
    m_Up    = glm::normalize(glm::cross(m_Right, m_Front));
}