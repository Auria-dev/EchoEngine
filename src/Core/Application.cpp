#include "Application.h"

#include <iostream>
#include <iomanip>

#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Renderer/Buffer.h"
#include "Renderer/Shader.h"
#include "Renderer/Texture.h"

Application::Application()
{
    if (!glfwInit()) return;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    m_WPosX = 100;
    m_WPosY = 100;
    m_WWidth = 1280;
    m_WHeight = 720;
    m_WFullscreen = false;
    m_Window = glfwCreateWindow(m_WWidth, m_WHeight, "EchoEngine", nullptr, nullptr);
    if (!m_Window)
    {
        glfwTerminate();
        return;
    }
    glfwWindowHint(GLFW_POSITION_X, m_WPosX);
    glfwWindowHint(GLFW_POSITION_Y, m_WPosY);
    
    glfwMakeContextCurrent(m_Window);
    glfwSwapInterval(1);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to init Glad\n";
        return;
    }

	glfwSetWindowUserPointer(m_Window, this);
	glfwSetWindowSizeCallback(m_Window, Application::OnWindowResized);

	glfwSetInputMode(m_Window, GLFW_STICKY_KEYS, GLFW_TRUE);
	glfwSetCursorPosCallback(m_Window, Application::OnMouseMoved);
	glfwSetScrollCallback(m_Window, Application::OnMouseWheelScrolled);
	glfwSetKeyCallback(m_Window, Application::OnKeyboard);
	glfwSetMouseButtonCallback(m_Window, Application::OnMouseButton);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable; 
    // io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(m_Window, true);
    ImGui_ImplOpenGL3_Init("#version 460");

    InputManager::GetInstance().BindAction("Quit", InputType::Key, GLFW_KEY_ESCAPE);
    InputManager::GetInstance().BindAction("ToggleCursor", InputType::Key, GLFW_KEY_ENTER);
    InputManager::GetInstance().BindAction("Fullscreen", InputType::Key, GLFW_KEY_F11);
	InputManager::GetInstance().BindAction("MoveForward",  InputType::Key, GLFW_KEY_W);
	InputManager::GetInstance().BindAction("MoveBackward", InputType::Key, GLFW_KEY_S);
	InputManager::GetInstance().BindAction("MoveLeft",	   InputType::Key, GLFW_KEY_A);
	InputManager::GetInstance().BindAction("MoveRight",    InputType::Key, GLFW_KEY_D);
	InputManager::GetInstance().BindAction("MoveUp",       InputType::Key, GLFW_KEY_SPACE);
	InputManager::GetInstance().BindAction("MoveDown",     InputType::Key, GLFW_KEY_LEFT_SHIFT);
}

Application::~Application()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(m_Window);
    glfwTerminate();
}

void Application::Run()
{
    float vertices[] = {
        1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
        1.0f, 1.0f, 0.0f, 1.0f, 1.0f,
        0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 0.0f, 0.0f, 0.0f
    };

    unsigned int indices[] = {
        0, 1, 2,
        2, 3, 0
    };

    int textureSlot = 0;
    Texture debugTex("assets/textures/WallDebug.png");
    debugTex.Bind();

    VertexArray va;
    VertexBuffer vb(vertices, sizeof(vertices));

    VertexBufferLayout layout;
    layout.Push<float>(3); // Position
    layout.Push<float>(2); // TexCoords

    va.AddBuffer(vb, layout);
    IndexBuffer ib(indices, 6);

    va.SetIndexBuffer(ib);
    Shader shader("assets/shaders/Basic.vert", "assets/shaders/Basic.frag");
    shader.Bind();
    shader.SetUniform1i("uAlbedo", textureSlot);
    shader.SetUniform4f("uColor", 0.8f, 0.3f, 0.8f, 1.0f);
    m_ActiveCamera = Camera();

    m_ActiveCamera.SetProjectionMatrix((float)m_WWidth / (float)m_WHeight, m_ActiveCamera.GetNear(), m_ActiveCamera.GetFar());

    glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 5.0f));
	glm::mat4 view = m_ActiveCamera.GetViewMatrix();
	glm::mat4 projection = m_ActiveCamera.GetProjectionMatrix();

    while (!glfwWindowShouldClose(m_Window))
    {
        float currentFrame = (float)glfwGetTime();
        m_DeltaTime = currentFrame - m_LastFrameTime;
        m_LastFrameTime = currentFrame;

        glfwPollEvents();
        Update();

        {
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            
            ImGui::NewFrame();
            ImGuiDockNodeFlags dockFlags = ImGuiDockNodeFlags_PassthruCentralNode;
            ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport(), dockFlags);
            ImGui::Begin("Performance");
            float fps = 1.0/m_DeltaTime;
            ImGui::Text("%.3f ms (%.1f FPS)", m_DeltaTime, fps);
            static float fpsHistory[100] = { 0 };
            static int fpsHistoryIndex = 0;
            fpsHistory[fpsHistoryIndex] = fps;
            fpsHistoryIndex = (fpsHistoryIndex + 1) % 100;
            ImGui::PlotLines("FPS", fpsHistory, IM_ARRAYSIZE(fpsHistory), fpsHistoryIndex, nullptr, 0.0f, 100.0f, ImVec2(0, 80));
            glm::vec3 camPos = m_ActiveCamera.GetPosition();
            ImGui::Text("Position: (%.2f, %.2f, %.2f)", camPos.x, camPos.y, camPos.z);
            ImGui::Text("Pitch: %.2f, Yaw: %.2f", m_ActiveCamera.GetPitch(), m_ActiveCamera.GetYaw());

            ImGui::End();
            ImGui::Render();
        }

        int w, h;
        glfwGetFramebufferSize(m_Window, &w, &h);
        glViewport(0, 0, w, h);

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        shader.Bind();
        shader.SetUniformMat4f("uView", m_ActiveCamera.GetViewMatrix());
        shader.SetUniformMat4f("uProjection", m_ActiveCamera.GetProjectionMatrix());
        shader.SetUniformMat4f("uModel", model);

        va.Bind();
        glDrawElements(GL_TRIANGLES, ib.GetCount(), GL_UNSIGNED_INT, nullptr);

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        
        if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            GLFWwindow* backup_current_context = glfwGetCurrentContext();
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            glfwMakeContextCurrent(backup_current_context);
        }

        InputManager::GetInstance().EndFrame();
        glfwSwapBuffers(m_Window);
    }
}

void Application::Update()
{
    if (InputManager::GetInstance().IsActionPressed("Quit")) glfwSetWindowShouldClose(m_Window, true);

    if (InputManager::GetInstance().IsActionPressed("ToggleCursor")) {
		int currentMode = glfwGetInputMode(m_Window, GLFW_CURSOR);
		int newMode = (currentMode == GLFW_CURSOR_DISABLED) ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED;
		glfwSetInputMode(m_Window, GLFW_CURSOR, newMode);

        if (glfwGetInputMode(m_Window, GLFW_CURSOR) == GLFW_CURSOR_DISABLED)
        {
            ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NoMouse;
        } else
        {
            ImGui::GetIO().ConfigFlags &= ~ImGuiConfigFlags_NoMouse;
        }
	}

    if (InputManager::GetInstance().IsActionPressed("Fullscreen")) {
        m_WFullscreen = !m_WFullscreen;

        if (!m_WFullscreen)
        {
            glfwWindowHint(GLFW_POSITION_X, m_WPosX);
            glfwWindowHint(GLFW_POSITION_Y, m_WPosY);

            GLFWmonitor* monitor = glfwGetWindowMonitor(m_Window) ? glfwGetWindowMonitor(m_Window) : glfwGetPrimaryMonitor();
            glfwWindowHint(GLFW_REFRESH_RATE, GLFW_DONT_CARE);
            glfwSetWindowMonitor(m_Window, NULL, m_WPosX, m_WPosY, m_WWidth, m_WHeight, GLFW_DONT_CARE);
        } else
        {
            GLFWmonitor* monitor = glfwGetWindowMonitor(m_Window) ? glfwGetWindowMonitor(m_Window) : glfwGetPrimaryMonitor();
            const GLFWvidmode* mode = glfwGetVideoMode(monitor);
            glfwWindowHint(GLFW_RED_BITS, mode->redBits);
            glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
            glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
            glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);
            glfwSetWindowMonitor(m_Window, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
        }
    }
    
    if (InputManager::GetInstance().IsActionDown("MoveForward"))  m_ActiveCamera.ProcessKeyboard(CameraMovement::FORWARD,  m_DeltaTime);
	if (InputManager::GetInstance().IsActionDown("MoveBackward")) m_ActiveCamera.ProcessKeyboard(CameraMovement::BACKWARD, m_DeltaTime);
	if (InputManager::GetInstance().IsActionDown("MoveLeft"))     m_ActiveCamera.ProcessKeyboard(CameraMovement::LEFT,     m_DeltaTime);
	if (InputManager::GetInstance().IsActionDown("MoveRight"))    m_ActiveCamera.ProcessKeyboard(CameraMovement::RIGHT,    m_DeltaTime);
	if (InputManager::GetInstance().IsActionDown("MoveUp"))       m_ActiveCamera.ProcessKeyboard(CameraMovement::UP,       m_DeltaTime);
	if (InputManager::GetInstance().IsActionDown("MoveDown"))     m_ActiveCamera.ProcessKeyboard(CameraMovement::DOWN,     m_DeltaTime);

	glm::vec2 mouseDelta = InputManager::GetInstance().GetMouseDelta();
	if (glfwGetInputMode(m_Window, GLFW_CURSOR) == GLFW_CURSOR_DISABLED)
    {
		if (mouseDelta.x != 0.0f || mouseDelta.y != 0.0f)
        {
            m_ActiveCamera.ProcessMouseMovement(mouseDelta.x, -mouseDelta.y);
		}
	}
    
    // glm::vec2 scroll = InputManager::GetInstance().GetScrollDelta();
}

void Application::OnWindowResized(GLFWwindow* window, int windowWidth, int windowHeight)
{
    if (0 == windowWidth || 0 == windowHeight) return;
    Application* application = reinterpret_cast<Application*>(glfwGetWindowUserPointer(window));
    application->Resized(windowWidth, windowHeight);
}

void Application::OnMouseMoved(GLFWwindow* window, double x, double y)
{
    Application* application = reinterpret_cast<Application*>(glfwGetWindowUserPointer(window));
    application->MouseMoved((float)x, (float)y);
}

void Application::OnMouseWheelScrolled(GLFWwindow* window, double x, double y)
{
    Application* application = reinterpret_cast<Application*>(glfwGetWindowUserPointer(window));
    application->MouseScrolled((float)x, (float)y);
}

void Application::OnMouseButton(GLFWwindow* window, int button, int action, int modifiers)
{
    Application* application = reinterpret_cast<Application*>(glfwGetWindowUserPointer(window));
    application->MouseButton(button, action, modifiers);
}

void Application::OnKeyboard(GLFWwindow* window, int key, int scancode, int action, int modifiers)
{
    Application* application = reinterpret_cast<Application*>(glfwGetWindowUserPointer(window));
    application->Keyboard(key, scancode, action, modifiers);
}

void Application::MouseMoved(float x, float y)
{
    InputManager::GetInstance().ProcessMouseMove(x, y);
}

void Application::MouseScrolled(float x, float y)
{
    InputManager::GetInstance().ProcessMouseScroll(x, y);
}

void Application::MouseButton(int button, int action, int mods)
{
    InputManager::GetInstance().ProcessMouseButton(button, action, mods);
}

void Application::Keyboard(int key, int scancode, int action, int modifiers)
{
    InputManager::GetInstance().ProcessKeyEvent(key, scancode, action, modifiers);
}

void Application::Resized(int width, int height) {
    m_WWidth = width;
    m_WHeight = height;
    m_ActiveCamera.SetProjectionMatrix((float)width/(float)height, m_ActiveCamera.GetNear(), m_ActiveCamera.GetFar());
}