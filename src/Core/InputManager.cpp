#include "InputManager.h"

InputManager& InputManager::GetInstance()
{
    static InputManager instance;
    return instance;
}

InputManager::InputManager()
    : m_currentKeyStates{ false }, m_previousKeyStates{ false },
    m_currentMouseStates{ false }, m_previousMouseStates{ false },
    m_currentMousePos(0.0f), m_previousMousePos(0.0f),
    m_scrollDelta(0.0f), m_firstMouse(true) {
}

void InputManager::EndFrame()
{
    m_previousKeyStates = m_currentKeyStates;
    m_previousMouseStates = m_currentMouseStates;
    m_previousMousePos = m_currentMousePos;

    m_scrollDelta = { 0.0f, 0.0f };
}

void InputManager::ProcessKeyEvent(int key, int scancode, int action, int mods)
{
    if (key >= 0 && key <= GLFW_KEY_LAST) {
        if (action == GLFW_PRESS) {
            m_currentKeyStates[key] = true;
        }
        else if (action == GLFW_RELEASE) {
            m_currentKeyStates[key] = false;
        }
    }
}

void InputManager::ProcessMouseMove(double x, double y)
{
    m_currentMousePos = { (float)x, (float)y };

    if (m_firstMouse)
    {
        m_previousMousePos = m_currentMousePos;
        m_firstMouse = false;
    }
}

void InputManager::ProcessMouseScroll(double x, double y)
{
    m_scrollDelta += glm::vec2((float)x, (float)y);
}

void InputManager::ProcessMouseButton(int button, int action, int mods)
{
    if (button >= 0 && button <= GLFW_MOUSE_BUTTON_LAST)
    {
        if (action == GLFW_PRESS)        m_currentMouseStates[button] = true;
        else if (action == GLFW_RELEASE) m_currentMouseStates[button] = false;
    }
}

bool InputManager::IsKeyDown(int key) const
{
    if (key < 0 || key > GLFW_KEY_LAST) return false;
    return m_currentKeyStates[key];
}

bool InputManager::IsKeyPressed(int key) const
{
    if (key < 0 || key > GLFW_KEY_LAST) return false;
    return m_currentKeyStates[key] && !m_previousKeyStates[key];
}

bool InputManager::IsKeyReleased(int key) const
{
    if (key < 0 || key > GLFW_KEY_LAST) return false;
    return !m_currentKeyStates[key] && m_previousKeyStates[key];
}

bool InputManager::IsMouseButtonDown(int button) const
{
    if (button < 0 || button > GLFW_MOUSE_BUTTON_LAST) return false;
    return m_currentMouseStates[button];
}

bool InputManager::IsMouseButtonPressed(int button) const
{
    if (button < 0 || button > GLFW_MOUSE_BUTTON_LAST) return false;
    return m_currentMouseStates[button] && !m_previousMouseStates[button];
}

bool InputManager::IsMouseButtonReleased(int button) const
{
    if (button < 0 || button > GLFW_MOUSE_BUTTON_LAST) return false;
    return !m_currentMouseStates[button] && m_previousMouseStates[button];
}

glm::vec2 InputManager::GetMousePosition() const
{
    return m_currentMousePos;
}

glm::vec2 InputManager::GetMouseDelta() const
{
    return m_currentMousePos - m_previousMousePos;
}

glm::vec2 InputManager::GetScrollDelta() const
{
    return m_scrollDelta;
}

void InputManager::BindAction(const std::string& action, InputType type, int code)
{
    m_actionBindings[action].push_back({ type, code });
}

bool InputManager::IsActionDown(const std::string& action) const
{
    auto it = m_actionBindings.find(action);
    if (it == m_actionBindings.end()) return false;

    for (const auto& binding : it->second)
    {

        if (binding.type == InputType::Key && IsKeyDown(binding.code))                      return true;
        else if (binding.type == InputType::MouseButton && IsMouseButtonDown(binding.code)) return true;
    }

    return false;
}

bool InputManager::IsActionPressed(const std::string& action) const
{
    auto it = m_actionBindings.find(action);
    if (it == m_actionBindings.end()) return false;

    for (const auto& binding : it->second) 
    {
        if (binding.type == InputType::Key && IsKeyPressed(binding.code))                      return true;
        else if (binding.type == InputType::MouseButton && IsMouseButtonPressed(binding.code)) return true;
    }
    
    return false;
}

bool InputManager::IsActionReleased(const std::string& action) const
{
    auto it = m_actionBindings.find(action);
    if (it == m_actionBindings.end()) return false;

    for (const auto& binding : it->second)
    {
        if (binding.type == InputType::Key && IsKeyReleased(binding.code))                      return true;
        else if (binding.type == InputType::MouseButton && IsMouseButtonReleased(binding.code)) return true;
    }
    
    return false;
}