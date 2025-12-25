#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

enum class CameraMovement {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT,
    UP,
    DOWN
};

class Camera
{

public:
    Camera(glm::vec3 position = { 0.0f, 0.0f, 0.0f },
        float yaw = 90.0f,
        float pitch = 0.0f,
        float speed = 2.5f,
        float sensitivity = 0.035f,
        float fov = 70.0f
    );

    float m_MovementSpeed;
    float m_MouseSensitivity;

    glm::mat4 GetViewMatrix() const;

    void SetProjectionMatrix(float aspectRatio, float nearPlane, float farPlane);
    glm::mat4 GetProjectionMatrix() const { return m_ProjectionMatrix; };
    
    glm::vec3 GetPosition() const { return m_Position; }
    glm::vec3 GetFront() const { return m_Front; }

	float GetYaw()   const { return m_Yaw; }
	void SetYaw(float yaw) { m_Yaw = yaw; UpdateCameraVectors(); }
	
    float GetPitch() const { return m_Pitch; }
	void SetPitch(float pitch) { m_Pitch = pitch; UpdateCameraVectors(); }
    
    float GetNear()  const { return m_Near; }
	void SetNear(float near) { m_Near = near; }

    float GetFar()  const { return m_Far; }
	void SetFar(float far) { m_Far = far; }
    
    float GetFOV()   const { return m_FOV; }
	void SetFOV(float fov) { m_FOV = fov; }
	
    void SetPosition(const glm::vec3& position) { m_Position = position; }

    void ProcessKeyboard(CameraMovement direction, float dt);
    void ProcessMouseMovement(float xoffset, float yoffset, bool constrainPitch = true);
    void ProcessMouseScroll(float yoffset);

private:
    glm::vec3 m_Position;
    glm::vec3 m_Front;
    glm::vec3 m_Up;
    glm::vec3 m_Right;
    glm::vec3 m_WorldUp;

    glm::mat4 m_ProjectionMatrix;

    // euler angles
    float m_Yaw;
    float m_Pitch;

    float m_FOV;

    float m_Near = 0.1f;
    float m_Far = 1000.0f;

    void UpdateCameraVectors();
};