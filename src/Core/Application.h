#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/vec3.hpp>

#include "InputManager.h"
#include "Camera.h"
#include "Scene.h"
#include "Renderer/Renderer.h"

class Application
{

public:
    Application();
    ~Application();

    void Run();
    void Update();

    void MouseMoved(float x, float y);
    void MouseScrolled(float x, float y);
    void MouseButton(int button, int action, int mods);
    void Keyboard(int key, int scancode, int action, int mods);
    void Resized(int width, int height);

    static void OnWindowResized(GLFWwindow* window, int width, int height);
    static void OnMouseMoved(GLFWwindow* window, double x, double y);
    static void OnMouseWheelScrolled(GLFWwindow* window, double x, double y);
    static void OnMouseButton(GLFWwindow* window, int button, int action, int modifiers);
    static void OnKeyboard(GLFWwindow* window, int key, int scancode, int action, int modifiers);

private:
    GLFWwindow* m_Window; // TODO: abstract this probably
    int m_WPosX, m_WPosY, m_WWidth, m_WHeight;
    bool m_WFullscreen;

    float m_DeltaTime;
    float m_LastFrameTime;

    SceneData m_Scene;
    Renderer m_Renderer;
};