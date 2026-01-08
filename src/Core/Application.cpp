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
#include "Renderer/RenderTexture.h"
#include "Renderer/MeshResource.h"

#include "Resources/Texture.h"
#include "Resources/Material.h"
#include "Resources/Entity.h"
#include "Resources/OBJLoader.h"


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
    glfwSwapInterval(0);

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

        
    ImGuiStyle * style = &ImGui::GetStyle();

    // colors
    style->Colors[ImGuiCol_Text]                  = ImVec4(0.95f, 0.95f, 0.95f, 1.00f);
    style->Colors[ImGuiCol_TextDisabled]          = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
    style->Colors[ImGuiCol_WindowBg]              = ImVec4(0.11f, 0.11f, 0.12f, 1.00f);
    style->Colors[ImGuiCol_ChildBg]               = ImVec4(0.12f, 0.12f, 0.13f, 1.00f);
    style->Colors[ImGuiCol_PopupBg]               = ImVec4(0.14f, 0.14f, 0.15f, 1.00f);
    style->Colors[ImGuiCol_Border]                = ImVec4(0.30f, 0.30f, 0.32f, 1.00f);
    style->Colors[ImGuiCol_BorderShadow]          = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    style->Colors[ImGuiCol_FrameBg]               = ImVec4(0.18f, 0.18f, 0.20f, 1.00f);
    style->Colors[ImGuiCol_FrameBgHovered]        = ImVec4(0.25f, 0.25f, 0.27f, 1.00f);
    style->Colors[ImGuiCol_FrameBgActive]         = ImVec4(0.28f, 0.28f, 0.30f, 1.00f);
    style->Colors[ImGuiCol_TitleBg]               = ImVec4(0.13f, 0.13f, 0.14f, 1.00f);
    style->Colors[ImGuiCol_TitleBgActive]         = ImVec4(0.16f, 0.16f, 0.18f, 1.00f);
    style->Colors[ImGuiCol_TitleBgCollapsed]      = ImVec4(0.10f, 0.10f, 0.11f, 0.90f);
    style->Colors[ImGuiCol_MenuBarBg]             = ImVec4(0.15f, 0.15f, 0.16f, 1.00f);
    style->Colors[ImGuiCol_ScrollbarBg]           = ImVec4(0.12f, 0.12f, 0.14f, 1.00f);
    style->Colors[ImGuiCol_ScrollbarGrab]         = ImVec4(0.35f, 0.35f, 0.38f, 0.80f);
    style->Colors[ImGuiCol_ScrollbarGrabHovered]  = ImVec4(0.45f, 0.45f, 0.50f, 0.80f);
    style->Colors[ImGuiCol_ScrollbarGrabActive]   = ImVec4(0.55f, 0.55f, 0.60f, 1.00f);
    style->Colors[ImGuiCol_CheckMark]             = ImVec4(0.33f, 0.60f, 0.98f, 1.00f);
    style->Colors[ImGuiCol_SliderGrab]            = ImVec4(0.35f, 0.60f, 0.98f, 0.85f);
    style->Colors[ImGuiCol_SliderGrabActive]      = ImVec4(0.33f, 0.70f, 1.00f, 1.00f);
    style->Colors[ImGuiCol_Button]                = ImVec4(0.20f, 0.20f, 0.22f, 1.00f);
    style->Colors[ImGuiCol_ButtonHovered]         = ImVec4(0.30f, 0.30f, 0.33f, 1.00f);
    style->Colors[ImGuiCol_ButtonActive]          = ImVec4(0.40f, 0.40f, 0.45f, 1.00f);
    style->Colors[ImGuiCol_Header]                = ImVec4(0.25f, 0.25f, 0.27f, 1.00f);
    style->Colors[ImGuiCol_HeaderHovered]         = ImVec4(0.33f, 0.33f, 0.37f, 1.00f);
    style->Colors[ImGuiCol_HeaderActive]          = ImVec4(0.45f, 0.45f, 0.50f, 1.00f);
    style->Colors[ImGuiCol_ResizeGrip]            = ImVec4(0.20f, 0.20f, 0.22f, 1.00f);
    style->Colors[ImGuiCol_ResizeGripHovered]     = ImVec4(0.35f, 0.35f, 0.38f, 1.00f);
    style->Colors[ImGuiCol_ResizeGripActive]      = ImVec4(0.50f, 0.50f, 0.55f, 1.00f);
    style->Colors[ImGuiCol_Tab]                   = ImVec4(0.15f, 0.15f, 0.16f, 1.00f);
    style->Colors[ImGuiCol_TabHovered]            = ImVec4(0.28f, 0.28f, 0.32f, 1.00f);
    style->Colors[ImGuiCol_TabActive]             = ImVec4(0.35f, 0.35f, 0.40f, 1.00f);
    style->Colors[ImGuiCol_TabUnfocused]          = ImVec4(0.12f, 0.12f, 0.13f, 1.00f);
    style->Colors[ImGuiCol_TabUnfocusedActive]    = ImVec4(0.20f, 0.20f, 0.22f, 1.00f);
    style->Colors[ImGuiCol_PlotLines]             = ImVec4(0.33f, 0.60f, 0.98f, 1.00f);
    style->Colors[ImGuiCol_PlotLinesHovered]      = ImVec4(0.50f, 0.75f, 1.00f, 1.00f);
    style->Colors[ImGuiCol_PlotHistogram]         = ImVec4(0.33f, 0.60f, 0.98f, 1.00f);
    style->Colors[ImGuiCol_PlotHistogramHovered]  = ImVec4(0.50f, 0.75f, 1.00f, 1.00f);
    style->Colors[ImGuiCol_TextSelectedBg]        = ImVec4(0.25f, 0.50f, 0.85f, 0.40f);
    style->Colors[ImGuiCol_DockingPreview]        = ImVec4(0.33f, 0.60f, 0.98f, 0.28f);
    style->Colors[ImGuiCol_NavHighlight]          = ImVec4(0.33f, 0.60f, 0.98f, 0.80f);
    style->Colors[ImGuiCol_NavWindowingHighlight] = ImVec4(0.33f, 0.60f, 0.98f, 0.70f);
    style->Colors[ImGuiCol_NavWindowingDimBg]     = ImVec4(0.00f, 0.00f, 0.00f, 0.60f);
    style->Colors[ImGuiCol_ModalWindowDimBg]      = ImVec4(0.00f, 0.00f, 0.00f, 0.60f);
    style->Colors[ImGuiCol_PlotLines]             = ImVec4(0.95f, 0.95f, 0.95f, 1.00f);
    style->Colors[ImGuiCol_PlotLines]             = ImVec4(0.95f, 0.95f, 0.95f, 1.00f);
    style->Colors[ImGuiCol_PlotLinesHovered]      = ImVec4(0.95f, 0.15f, 0.15f, 1.00f);

    // rounding & spacing
    style->FrameRounding        = 5.0f;
    style->GrabRounding         = 5.0f;
    style->WindowRounding       = 6.0f;
    style->ChildRounding        = 4.0f;
    style->PopupRounding        = 4.0f;
    style->TabRounding          = 4.0f;
    style->ScrollbarRounding    = 5.0f;
    style->IndentSpacing        = 14.0f;
    style->ItemSpacing          = ImVec2(8, 4);
    style->ItemInnerSpacing     = ImVec2(4, 4);
    style->WindowPadding        = ImVec2(10, 10);
    style->FramePadding         = ImVec2(6, 4);
    style->DisplaySafeAreaPadding= ImVec2(4,4);

    // borders & separators
    style->WindowBorderSize     = 1.0f;
    style->ChildBorderSize      = 1.0f;
    style->PopupBorderSize      = 1.0f;
    style->FrameBorderSize      = 0.5f;
    style->TabBorderSize        = 0.0f;
    style->SeparatorTextBorderSize = 1.0f;
    style->FrameBorderSize = 0.0f;

    
    InputManager::GetInstance().BindAction("Quit",         InputType::Key, GLFW_KEY_ESCAPE);
    InputManager::GetInstance().BindAction("ToggleCursor", InputType::Key, GLFW_KEY_ENTER);
    InputManager::GetInstance().BindAction("Fullscreen",   InputType::Key, GLFW_KEY_F11);
	InputManager::GetInstance().BindAction("MoveForward",  InputType::Key, GLFW_KEY_W);
	InputManager::GetInstance().BindAction("MoveBackward", InputType::Key, GLFW_KEY_S);
	InputManager::GetInstance().BindAction("MoveLeft",	   InputType::Key, GLFW_KEY_A);
	InputManager::GetInstance().BindAction("MoveRight",    InputType::Key, GLFW_KEY_D);
	InputManager::GetInstance().BindAction("MoveUp",       InputType::Key, GLFW_KEY_SPACE);
	InputManager::GetInstance().BindAction("MoveDown",     InputType::Key, GLFW_KEY_LEFT_SHIFT);
	InputManager::GetInstance().BindAction("ReloadShaders",InputType::Key, GLFW_KEY_R);
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
    std::cout << std::endl;

    Entity room;
    room.LoadFromOBJ("assets/models/room.obj");
    room.Translate(glm::vec3(3.3f, 2.5f, 0.0f));
    room.Rotate(glm::vec3(0.0f, 20.0f, 0.0f));
    m_Scene.Entities.push_back(&room);
    
    // Entity ground;
    // ground.LoadFromOBJ("assets/models/landscape.obj");
    // ground.Scale(glm::vec3(1.0,0.5,1.0));
    // m_Scene.Entities.push_back(&ground);

    glm::vec3 lightColor(1.0f, 0.83f, 0.72);

    // Entity BistroExt;
    // BistroExt.LoadFromOBJ("assets/models/heavy/BistroExterior.obj");
    // BistroExt.SetScale(glm::vec3(0.01, 0.01, 0.01));
    // m_Scene.Entities.push_back(&BistroExt);

    // Entity BistroInt;
    // BistroInt.LoadFromOBJ("assets/models/heavy/interior.obj");
    // BistroInt.SetScale(glm::vec3(0.01, 0.01, 0.01));
    // m_Scene.Entities.push_back(&BistroInt);

    Camera t = Camera();
    t.SetPosition(glm::vec3(6.3f, 4.0f, 4.0f));
    
    // PointLight* pointlight = new PointLight();
    // pointlight->Position = glm::vec3(2.5f, 2.7f, -0.8f);
    // pointlight->Color = lightColor;
    // pointlight->Intensity = 4.2f;
    // m_Scene.Lights.push_back(pointlight);
    
    // DirectionalLight* dirLight = new DirectionalLight();
    // dirLight->Direction = glm::vec3(0.482f, -0.626, 0.613);
    // dirLight->Color = glm::vec3(0.9f, 0.6f, 0.34f);
    // dirLight->Intensity = 20.0f;
    // m_Scene.Lights.push_back(dirLight);

    // glm::vec3 positions[4] = {
    //     { 6.100, 2.600, -8.500},
    //     { 4.25, 2.600, -4.920},
    //     { 5.0, 2.6, -0.5},
    //     { 8.0, 2.6, 1.0}
    // };

    // for (int i=0;i<4;i++) {
    //     SpotLight* spotLight = new SpotLight();
    //     spotLight->Direction = glm::vec3(0.0f, -1.0f, 0.0f);
    //     spotLight->Position = positions[i];
    //     spotLight->Color = lightColor;
    //     spotLight->InnerCutoff = 16.0f;
    //     spotLight->OuterCutoff = 120.0f;
    //     m_Scene.Lights.push_back(spotLight);
    // }

    m_Scene.activeCamera = &t;
    m_Scene.activeCamera->SetProjectionMatrix((float)m_WWidth / (float)m_WHeight, m_Scene.activeCamera->GetNear(), m_Scene.activeCamera->GetFar());

    
    m_Renderer.SetScene(m_Scene);
    m_Renderer.Init(m_WWidth, m_WHeight);
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
        glm::vec3 camPos = m_Scene.activeCamera->GetPosition();
        ImGui::Text("Position: (%.2f, %.2f, %.2f)", camPos.x, camPos.y, camPos.z);
        ImGui::Text("Pitch: %.2f, Yaw: %.2f", m_Scene.activeCamera->GetPitch(), m_Scene.activeCamera->GetYaw());

        ImGui::End();

        ImGui::Begin("Style editor");        
        ImGui::ShowStyleEditor();
        ImGui::End();


        ImGui::Begin("Scene Inspector");

        if (ImGui::BeginTabBar("SceneTabs"))
        {
            if (ImGui::BeginTabItem("Entities"))
            {
                for (size_t i = 0; i < m_Scene.Entities.size(); ++i)
                {
                    Entity& entity = *m_Scene.Entities[i]; 
                    std::string entityName = "Entity " + std::to_string(i);
                    ImGui::PushID(static_cast<int>(i));

                    if (ImGui::CollapsingHeader(entityName.c_str()))
                    {
                        if (ImGui::DragFloat3("Position", &entity.position.x, 0.1f)) entity.SetPosition(entity.position); 
                        if (ImGui::DragFloat3("Rotation", &entity.rotation.x, 1.0f)) entity.SetRotation(entity.rotation);
                        if (ImGui::DragFloat3("Scale", &entity.scale.x, 0.1f))       entity.SetScale(entity.scale);
                        
                        ImGui::Separator();

                        if (ImGui::TreeNode("Materials"))
                        {
                            for (size_t m = 0; m < entity.materials.size(); ++m)
                            {
                                auto& mat = entity.materials[m];
                                std::string matLabel = "Material " + std::to_string(m);

                                if (ImGui::TreeNode(matLabel.c_str()))
                                {
                                    auto ShowTextureSlot = [&](const char* name, Texture* cpuTex) {
                                        ImGui::Text("%s", name);
                                        if (cpuTex) {
                                            RenderTexture* gpuTex = m_Renderer.GetGPUTexture(cpuTex);
                                            if (gpuTex) {
                                                ImGui::Image((void*)(intptr_t)gpuTex->GetID(), ImVec2(64, 64));
                                                if (ImGui::IsItemHovered()) {
                                                    ImGui::BeginTooltip();
                                                    ImGui::Image((void*)(intptr_t)gpuTex->GetID(), ImVec2(256, 256));
                                                    ImGui::EndTooltip();
                                                }
                                            }
                                        } else {
                                            ImGui::TextDisabled("(Empty)");
                                        }
                                    };

                                    ShowTextureSlot("Diffuse", mat->DiffuseTexture.get());
                                    ShowTextureSlot("Normal",  mat->NormalTexture.get());
                                    ShowTextureSlot("ARM",     mat->ARMTexture.get());
                                    
                                    ImGui::TreePop();
                                }
                            }
                            ImGui::TreePop();
                        }
                    }
                    ImGui::PopID();
                }
                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("Lights"))
            {
                for (size_t i = 0; i < m_Scene.Lights.size(); ++i)
                {
                    Light* light = m_Scene.Lights[i];
                    ImGui::PushID(static_cast<int>(i));

                    DirectionalLight* dLight = dynamic_cast<DirectionalLight*>(light);
                    PointLight* pLight = dynamic_cast<PointLight*>(light);
                    SpotLight* sLight = dynamic_cast<SpotLight*>(light);

                    std::string headerName;
                    if (dLight) headerName = "Directional Light " + std::to_string(i);
                    else if (pLight) headerName = "Point Light " + std::to_string(i);
                    else if (sLight) headerName = "Spot Light " + std::to_string(i);
                    else headerName = "Unknown Light " + std::to_string(i);

                    if (ImGui::CollapsingHeader(headerName.c_str()))
                    {
                        if (dLight)
                        {
                            ImGui::ColorEdit3("Color", &dLight->Color.x);
                            ImGui::DragFloat("Intensity", &dLight->Intensity, 0.1f, 0.0f, 100.0f);
                            if (ImGui::DragFloat3("Direction", &dLight->Direction.x, 0.05f)) dLight->Direction = glm::normalize(dLight->Direction);
                        }
                        else if (pLight)
                        {
                            ImGui::ColorEdit3("Color", &pLight->Color.x);
                            ImGui::DragFloat("Intensity", &pLight->Intensity, 0.1f, 0.0f, 100.0f);
                            ImGui::DragFloat3("Position", &pLight->Position.x, 0.1f);
                            
                            ImGui::Text("Attenuation");
                            ImGui::DragFloat("Linear", &pLight->Linear, 0.01f, 0.0f, 1.0f);
                            ImGui::DragFloat("Quadratic", &pLight->Quadratic, 0.001f, 0.0f, 1.0f);
                        }
                        else if (sLight)
                        {
                            ImGui::ColorEdit3("Color", &sLight->Color.x);
                            ImGui::DragFloat("Intensity", &sLight->Intensity, 0.1f, 0.0f, 100.0f);
                            ImGui::DragFloat3("Position", &sLight->Position.x, 0.1f);
                            if (ImGui::DragFloat3("Direction", &sLight->Direction.x, 0.05f)) sLight->Direction = glm::normalize(sLight->Direction);

                            ImGui::DragFloat("Inner Cutoff", &sLight->InnerCutoff, 0.1f, 0.0f, 180.0f);
                            ImGui::DragFloat("Outer Cutoff", &sLight->OuterCutoff, 0.1f, 0.0f, 180.0f);
                        }
                    }
                    ImGui::PopID();
                }
                ImGui::EndTabItem();
            }
            ImGui::EndTabBar();
        }

        m_Renderer.OnImGuiRender();

        ImGui::End();

        ImGui::Render();
        
        m_Renderer.BeginFrame();
		m_Renderer.DrawScene();
		m_Renderer.EndFrame();

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
    
    if (InputManager::GetInstance().IsActionDown("MoveForward"))  m_Scene.activeCamera->ProcessKeyboard(CameraMovement::FORWARD,  m_DeltaTime);
	if (InputManager::GetInstance().IsActionDown("MoveBackward")) m_Scene.activeCamera->ProcessKeyboard(CameraMovement::BACKWARD, m_DeltaTime);
	if (InputManager::GetInstance().IsActionDown("MoveLeft"))     m_Scene.activeCamera->ProcessKeyboard(CameraMovement::LEFT,     m_DeltaTime);
	if (InputManager::GetInstance().IsActionDown("MoveRight"))    m_Scene.activeCamera->ProcessKeyboard(CameraMovement::RIGHT,    m_DeltaTime);
	if (InputManager::GetInstance().IsActionDown("MoveUp"))       m_Scene.activeCamera->ProcessKeyboard(CameraMovement::UP,       m_DeltaTime);
	if (InputManager::GetInstance().IsActionDown("MoveDown"))     m_Scene.activeCamera->ProcessKeyboard(CameraMovement::DOWN,     m_DeltaTime);

	glm::vec2 mouseDelta = InputManager::GetInstance().GetMouseDelta();
	if (glfwGetInputMode(m_Window, GLFW_CURSOR) == GLFW_CURSOR_DISABLED)
    {
		if (mouseDelta.x != 0.0f || mouseDelta.y != 0.0f)
        {
            m_Scene.activeCamera->ProcessMouseMovement(mouseDelta.x, -mouseDelta.y);
		}
	}
    
    // glm::vec2 scroll = InputManager::GetInstance().GetScrollDelta();
    if (InputManager::GetInstance().IsActionPressed("ReloadShaders")) m_Renderer.ReloadShaders();
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
    m_Scene.activeCamera->SetProjectionMatrix((float)width/(float)height, m_Scene.activeCamera->GetNear(), m_Scene.activeCamera->GetFar());
    m_Renderer.Resize(width, height);
}