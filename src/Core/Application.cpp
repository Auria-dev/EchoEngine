#include "Application.h"

#include <iostream>
#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>

Application::Application()
{
    if (!glfwInit()) return;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    m_Window = glfwCreateWindow(1280, 720, "EchoEngine", nullptr, nullptr);
    if (!m_Window)
    {
        glfwTerminate();
        return;
    }
    
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
    while (!glfwWindowShouldClose(m_Window))
    {
        float currentFrame = (float)glfwGetTime();
        m_DeltaTime = currentFrame - m_LastFrameTime;
        m_LastFrameTime = currentFrame;

        glfwPollEvents();
        Update();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("Engine Control");
        ImGui::Text("Application Running!");
        ImGui::End();

        ImGui::Render();
        int w, h;
        glfwGetFramebufferSize(m_Window, &w, &h);
        glViewport(0, 0, w, h);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        
        InputManager::GetInstance().EndFrame();
        glfwSwapBuffers(m_Window);
    }
}

void Application::Update()
{
    std::cout << "updated " << m_DeltaTime << " (" << 1.0/m_DeltaTime << " FPS)" << std::endl;
    if (InputManager::GetInstance().IsActionPressed("Quit")) glfwSetWindowShouldClose(m_Window, true);

    if (InputManager::GetInstance().IsActionPressed("ToggleCursor")) {
		int currentMode = glfwGetInputMode(m_Window, GLFW_CURSOR);
		int newMode = (currentMode == GLFW_CURSOR_DISABLED) ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED;
		glfwSetInputMode(m_Window, GLFW_CURSOR, newMode);
	}

    // glm::vec2 mouseDelta = InputManager::GetInstance().GetMouseDelta();
	// glm::vec2 scroll = InputManager::GetInstance().GetScrollDelta();
}

void Application::OnWindowResized(GLFWwindow* window, int windowWidth, int windowHeight)
{
    if (0 == windowWidth || 0 == windowHeight) return;
    Application* application = reinterpret_cast<Application*>(glfwGetWindowUserPointer(window));
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