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
    ImVec4* colors = style->Colors;

    // Color Palette Definitions (Mountain)
    // ------------------------------------
    // Black        #0f0f0f -> 0.06f, 0.06f, 0.06f
    // Black2       #181818 -> 0.09f, 0.09f, 0.09f
    // OneBg        #191919 -> 0.10f, 0.10f, 0.10f
    // OneBg2       #222222 -> 0.13f, 0.13f, 0.13f
    // OneBg3       #2a2a2a -> 0.16f, 0.16f, 0.16f
    // Grey         #373737 -> 0.22f, 0.22f, 0.22f
    // LightGrey    #535353 -> 0.33f, 0.33f, 0.33f
    // White        #F0f0f0 -> 0.94f, 0.94f, 0.94f
    // Green        #8aac8b -> 0.54f, 0.67f, 0.55f (Primary Accent)
    // VibrantGreen #99bb9a -> 0.60f, 0.73f, 0.60f
    // Line         #242424 -> 0.14f, 0.14f, 0.14f
    // NordBlue     #8F8AAC -> 0.56f, 0.54f, 0.67f (Selection)

    colors[ImGuiCol_Text]                   = ImVec4(0.94f, 0.94f, 0.94f, 1.00f); // White
    colors[ImGuiCol_TextDisabled]           = ImVec4(0.33f, 0.33f, 0.33f, 1.00f); // Light Grey
    colors[ImGuiCol_WindowBg]               = ImVec4(0.06f, 0.06f, 0.06f, 1.00f); // Black
    colors[ImGuiCol_ChildBg]                = ImVec4(0.09f, 0.09f, 0.09f, 1.00f); // Black2
    colors[ImGuiCol_PopupBg]                = ImVec4(0.10f, 0.10f, 0.10f, 1.00f); // OneBg
    colors[ImGuiCol_Border]                 = ImVec4(0.14f, 0.14f, 0.14f, 1.00f); // Line
    colors[ImGuiCol_BorderShadow]           = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_FrameBg]                = ImVec4(0.10f, 0.10f, 0.10f, 1.00f); // OneBg
    colors[ImGuiCol_FrameBgHovered]         = ImVec4(0.13f, 0.13f, 0.13f, 1.00f); // OneBg2
    colors[ImGuiCol_FrameBgActive]          = ImVec4(0.16f, 0.16f, 0.16f, 1.00f); // OneBg3
    colors[ImGuiCol_TitleBg]                = ImVec4(0.06f, 0.06f, 0.06f, 1.00f); // Black
    colors[ImGuiCol_TitleBgActive]          = ImVec4(0.09f, 0.09f, 0.09f, 1.00f); // Black2
    colors[ImGuiCol_TitleBgCollapsed]       = ImVec4(0.06f, 0.06f, 0.06f, 1.00f); // Black
    colors[ImGuiCol_MenuBarBg]              = ImVec4(0.10f, 0.10f, 0.10f, 1.00f); // OneBg
    colors[ImGuiCol_ScrollbarBg]            = ImVec4(0.06f, 0.06f, 0.06f, 1.00f); // Black
    colors[ImGuiCol_ScrollbarGrab]          = ImVec4(0.22f, 0.22f, 0.22f, 1.00f); // Grey
    colors[ImGuiCol_ScrollbarGrabHovered]   = ImVec4(0.33f, 0.33f, 0.33f, 1.00f); // Light Grey
    colors[ImGuiCol_ScrollbarGrabActive]    = ImVec4(0.54f, 0.67f, 0.55f, 1.00f); // Green
    colors[ImGuiCol_CheckMark]              = ImVec4(0.54f, 0.67f, 0.55f, 1.00f); // Green
    colors[ImGuiCol_SliderGrab]             = ImVec4(0.54f, 0.67f, 0.55f, 1.00f); // Green
    colors[ImGuiCol_SliderGrabActive]       = ImVec4(0.60f, 0.73f, 0.60f, 1.00f); // Vibrant Green
    colors[ImGuiCol_Button]                 = ImVec4(0.13f, 0.13f, 0.13f, 1.00f); // OneBg2
    colors[ImGuiCol_ButtonHovered]          = ImVec4(0.16f, 0.16f, 0.16f, 1.00f); // OneBg3
    colors[ImGuiCol_ButtonActive]           = ImVec4(0.22f, 0.22f, 0.22f, 1.00f); // Grey
    colors[ImGuiCol_Header]                 = ImVec4(0.13f, 0.13f, 0.13f, 1.00f); // OneBg2
    colors[ImGuiCol_HeaderHovered]          = ImVec4(0.16f, 0.16f, 0.16f, 1.00f); // OneBg3
    colors[ImGuiCol_HeaderActive]           = ImVec4(0.22f, 0.22f, 0.22f, 1.00f); // Grey
    colors[ImGuiCol_Separator]              = ImVec4(0.14f, 0.14f, 0.14f, 1.00f); // Line
    colors[ImGuiCol_SeparatorHovered]       = ImVec4(0.22f, 0.22f, 0.22f, 1.00f); // Grey
    colors[ImGuiCol_SeparatorActive]        = ImVec4(0.54f, 0.67f, 0.55f, 1.00f); // Green
    colors[ImGuiCol_ResizeGrip]             = ImVec4(0.13f, 0.13f, 0.13f, 1.00f); // OneBg2
    colors[ImGuiCol_ResizeGripHovered]      = ImVec4(0.22f, 0.22f, 0.22f, 1.00f); // Grey
    colors[ImGuiCol_ResizeGripActive]       = ImVec4(0.54f, 0.67f, 0.55f, 1.00f); // Green
    colors[ImGuiCol_Tab]                    = ImVec4(0.09f, 0.09f, 0.09f, 1.00f); // Black2
    colors[ImGuiCol_TabHovered]             = ImVec4(0.16f, 0.16f, 0.16f, 1.00f); // OneBg3
    colors[ImGuiCol_TabActive]              = ImVec4(0.13f, 0.13f, 0.13f, 1.00f); // OneBg2
    colors[ImGuiCol_TabUnfocused]           = ImVec4(0.09f, 0.09f, 0.09f, 1.00f); // Black2
    colors[ImGuiCol_TabUnfocusedActive]     = ImVec4(0.13f, 0.13f, 0.13f, 1.00f); // OneBg2
    colors[ImGuiCol_PlotLines]              = ImVec4(0.54f, 0.67f, 0.55f, 1.00f); // Green
    colors[ImGuiCol_PlotLinesHovered]       = ImVec4(0.60f, 0.73f, 0.60f, 1.00f); // Vibrant Green
    colors[ImGuiCol_PlotHistogram]          = ImVec4(0.54f, 0.67f, 0.55f, 1.00f); // Green
    colors[ImGuiCol_PlotHistogramHovered]   = ImVec4(0.60f, 0.73f, 0.60f, 1.00f); // Vibrant Green
    colors[ImGuiCol_TableHeaderBg]          = ImVec4(0.13f, 0.13f, 0.13f, 1.00f); // OneBg2
    colors[ImGuiCol_TableBorderStrong]      = ImVec4(0.14f, 0.14f, 0.14f, 1.00f); // Line
    colors[ImGuiCol_TableBorderLight]       = ImVec4(0.14f, 0.14f, 0.14f, 1.00f); // Line
    colors[ImGuiCol_TableRowBg]             = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_TableRowBgAlt]          = ImVec4(1.00f, 1.00f, 1.00f, 0.06f);
    colors[ImGuiCol_TextSelectedBg]         = ImVec4(0.56f, 0.54f, 0.67f, 0.35f); // Nord Blue (Alpha)
    colors[ImGuiCol_DragDropTarget]         = ImVec4(0.54f, 0.67f, 0.55f, 0.90f); // Green
    colors[ImGuiCol_NavHighlight]           = ImVec4(0.54f, 0.67f, 0.55f, 1.00f); // Green
    colors[ImGuiCol_NavWindowingHighlight]  = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
    colors[ImGuiCol_NavWindowingDimBg]      = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
    colors[ImGuiCol_ModalWindowDimBg]       = ImVec4(0.00f, 0.00f, 0.00f, 0.35f);
    colors[ImGuiCol_DockingPreview]      = ImVec4(0.54f, 0.67f, 0.55f, 0.70f); // Green
    colors[ImGuiCol_DockingEmptyBg]      = ImVec4(0.06f, 0.06f, 0.06f, 1.00f); // Black

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

bool DirectionGizmo(const char* label, glm::vec3& direction) {
    bool value_changed = false;
    ImGuiIO& io = ImGui::GetIO();
    ImDrawList* draw_list = ImGui::GetWindowDrawList();

    float radius = 40.0f;
    float thickness = 2.0f;
    ImVec2 p = ImGui::GetCursorScreenPos();
    ImVec2 center = ImVec2(p.x + radius, p.y + radius);
    ImGuiID id = ImGui::GetID(label);

    ImGui::InvisibleButton(label, ImVec2(radius * 2, radius * 2));
    ImGuiStorage* storage = ImGui::GetStateStorage();
    
    if (ImGui::IsItemActivated()) {
        storage->SetFloat(id, io.MousePos.x);
        storage->SetFloat(id + 1, io.MousePos.y);
    }

    if (ImGui::IsItemActive()) {
        ImGui::SetMouseCursor(ImGuiMouseCursor_None);
        ImVec2 delta = io.MouseDelta;
        
        if (delta.x != 0.0f || delta.y != 0.0f) {
            float sensitivity = 0.0005f;

            glm::mat4 yaw = glm::rotate(glm::mat4(1.0f), -delta.x * sensitivity, glm::vec3(0, 1, 0));
            glm::vec3 right = glm::cross(direction, glm::vec3(0, 1, 0));
            
            if (glm::length(right) < 0.001f) right = glm::vec3(1, 0, 0);
            
            glm::mat4 pitch = glm::rotate(glm::mat4(1.0f), delta.y * sensitivity, glm::normalize(right));

            direction = glm::vec3(yaw * pitch * glm::vec4(direction, 0.0f));
            direction = glm::normalize(direction);
            value_changed = true;
            
            ImVec2 lock_pos = ImVec2(storage->GetFloat(id), storage->GetFloat(id + 1));
            
            io.MousePos = lock_pos;
            io.WantSetMousePos = true; 
        }
    }

    ImU32 col_bg = ImGui::GetColorU32(ImGuiCol_FrameBg);
    ImU32 col_border = ImGui::GetColorU32(ImGuiCol_Text);
    ImU32 col_arrow = ImGui::GetColorU32(ImGuiCol_ButtonActive);

    if (ImGui::IsItemHovered() || ImGui::IsItemActive()) {
        col_border = ImGui::GetColorU32(ImGuiCol_TextDisabled); 
    }

    draw_list->AddCircleFilled(center, radius, col_bg);
    draw_list->AddCircle(center, radius, col_border, 12, thickness);

    float arrow_len = radius * 0.9f;
    glm::vec3 up(0, 1, 0);
    glm::vec3 draw_right = glm::normalize(glm::cross(direction, up)); 
    ImVec2 arrow_end(
        center.x + direction.x * arrow_len, 
        center.y - direction.y * arrow_len
    );
    
    draw_list->AddLine(center, arrow_end, col_arrow, 3.0f);
    draw_list->AddCircleFilled(arrow_end, 4.0f, col_arrow);

    ImGui::SetCursorScreenPos(ImVec2(p.x, p.y + radius * 2 + 5));
    ImGui::Text("%s", label);

    return value_changed;
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

    // Entity room;
    // room.LoadFromOBJ("assets/models/room.obj");
    // room.Translate(glm::vec3(3.3f, 2.5f, 0.0f));
    // room.Rotate(glm::vec3(0.0f, 20.0f, 0.0f));
    // m_Scene.Entities.push_back(&room);
    
    Entity ground;
    ground.LoadFromOBJ("assets/models/terrain.obj");
    ground.Translate(glm::vec3(0.0, 0.0, 0.0));
    m_Scene.m_Entities.push_back(&ground);

    // Entity BistroExt;
    // BistroExt.LoadFromOBJ("assets/models/heavy/BistroExterior.obj");
    // BistroExt.SetScale(glm::vec3(0.01, 0.01, 0.01));
    // BistroExt.SetPosition(glm::vec3(-89.0, 132.0, -867.0));
    // m_Scene.m_Entities.push_back(&BistroExt);

    // Entity BistroInt;
    // BistroInt.LoadFromOBJ("assets/models/heavy/interior.obj");
    // BistroInt.SetScale(glm::vec3(0.01, 0.01, 0.01));
    // m_Scene.m_Entities.push_back(&BistroInt);

    Camera t = Camera();
    t.SetPosition(glm::vec3(-109.0, 152.0, -867.0));
    t.SetPitch(6.0);
    t.SetYaw(54.03);
    t.m_MovementSpeed = 10.0f;
    
    
    m_Scene.m_Sun.Direction = glm::normalize(glm::vec3(-0.26, -0.56, -0.78));
    m_Scene.m_Sun.Color = glm::vec3(1.0f, 1.0f, 1.0f);
    m_Scene.m_Sun.Intensity = 1.0f;

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
        ImGui::NewLine();
        std::unordered_map<std::string, std::chrono::nanoseconds>::iterator it;
        for (it=m_Renderer.m_PerformanceTimer.begin(); it!=m_Renderer.m_PerformanceTimer.end(); ++it)
        {
            ImGui::Text("%-16s: %4lld ms",it->first.c_str(), (long long)std::chrono::duration_cast<std::chrono::milliseconds>(it->second).count());
        }

        glm::vec3 camPos = m_Scene.activeCamera->GetPosition();
        glm::vec3 camFront = m_Scene.activeCamera->GetFront();
        ImGui::DragFloat("Move speed", &m_Scene.activeCamera->m_MovementSpeed, 0.1f, 0.01f, 400.0f);
        ImGui::Text("Position: (%.2f, %.2f, %.2f)", camPos.x, camPos.y, camPos.z);
        ImGui::Text("Direction: (%.2f, %.2f, %.2f)", camFront.x, camFront.y, camFront.z);
        ImGui::Text("Pitch: %.2f, Yaw: %.2f", m_Scene.activeCamera->GetPitch(), m_Scene.activeCamera->GetYaw());

        ImGui::End();

        // ImGui::Begin("Style editor");        
        // ImGui::ShowStyleEditor();
        // ImGui::End();

        ImGui::Begin("Scene Inspector");
        ImGui::Text("Sun parameters");
        ImGui::ColorEdit3("Color", &m_Scene.m_Sun.Color.x);
        ImGui::DragFloat("Intensity", &m_Scene.m_Sun.Intensity, 0.1f, 0.0f, 100.0f);
        if (DirectionGizmo("Direction", m_Scene.m_Sun.Direction)) m_Scene.m_Sun.Direction = glm::normalize(m_Scene.m_Sun.Direction);

        if (ImGui::BeginTabBar("SceneTabs"))
        {
            if (ImGui::BeginTabItem("Entities"))
            {
                for (size_t i = 0; i < m_Scene.m_Entities.size(); ++i)
                {
                    Entity& entity = *m_Scene.m_Entities[i]; 
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
                for (size_t i = 0; i < m_Scene.m_Lights.size(); ++i)
                {
                    Light* light = m_Scene.m_Lights[i];
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
                            // if (ImGui::DragFloat3("Direction", &dLight->Direction.x, 0.05f)) dLight->Direction = glm::normalize(dLight->Direction);
                            if (DirectionGizmo("Direction", dLight->Direction)) dLight->Direction = glm::normalize(dLight->Direction);
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