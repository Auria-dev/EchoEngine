#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/vec3.hpp>

namespace Echo
{
    class Application
    {

    public:
        Application();
        ~Application();

        void Run();

    private:
        GLFWwindow* m_Window;
        glm::vec3 m_ClearColor = glm::vec3(0.1f, 0.1f, 0.1f);
    
    };
}