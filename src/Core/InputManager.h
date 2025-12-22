#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/vec2.hpp>

#include <string>
#include <vector>
#include <unordered_map>
#include <array>

enum class InputType
{
    Key,
    MouseButton
    // TODO: controller support
};

struct InputBinding
{
    InputType type;
    int code;
};

class InputManager
{

public:
    static InputManager& GetInstance();

    InputManager(const InputManager&) = delete;
    InputManager(InputManager&&) = delete;
    InputManager& operator=(const InputManager&) = delete;
    InputManager& operator=(InputManager&&) = delete;

    void EndFrame();

    void ProcessKeyEvent(int key, int scancode, int action, int mods);
    void ProcessMouseMove(double x, double y);
    void ProcessMouseScroll(double x, double y);
    void ProcessMouseButton(int button, int action, int mods);

	// for single press
    bool IsKeyDown(int key) const;
    bool IsKeyPressed(int key) const;
    bool IsKeyReleased(int key) const;
    bool IsMouseButtonDown(int button) const;
    bool IsMouseButtonPressed(int button) const;
    bool IsMouseButtonReleased(int button) const;

    glm::vec2 GetMousePosition() const;
    glm::vec2 GetMouseDelta() const;
    glm::vec2 GetScrollDelta() const;

	// for repeated press
    bool IsActionDown(const std::string& action) const;
    bool IsActionPressed(const std::string& action) const;
    bool IsActionReleased(const std::string& action) const;

    void BindAction(const std::string& action, InputType type, int code);

private:
    InputManager();
    ~InputManager() = default;

    std::array<bool, GLFW_KEY_LAST + 1> m_currentKeyStates;
    std::array<bool, GLFW_KEY_LAST + 1> m_previousKeyStates;

    std::array<bool, GLFW_MOUSE_BUTTON_LAST + 1> m_currentMouseStates;
    std::array<bool, GLFW_MOUSE_BUTTON_LAST + 1> m_previousMouseStates;

    glm::vec2 m_currentMousePos;
    glm::vec2 m_previousMousePos;
    glm::vec2 m_scrollDelta;

    bool m_firstMouse;

    std::unordered_map<std::string, std::vector<InputBinding>> m_actionBindings;

};