#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/vec3.hpp>

#include "InputManager.h"

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

    static void OnWindowResized(GLFWwindow* window, int width, int height);
    static void OnMouseMoved(GLFWwindow* window, double x, double y);
    static void OnMouseWheelScrolled(GLFWwindow* window, double x, double y);
    static void OnMouseButton(GLFWwindow* window, int button, int action, int modifiers);
    static void OnKeyboard(GLFWwindow* window, int key, int scancode, int action, int modifiers);

private:
    GLFWwindow* m_Window;
    float m_DeltaTime;
    float m_LastFrameTime;

};