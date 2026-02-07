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
    glfwWindowHint(GLFW_SCALE_TO_MONITOR, GLFW_TRUE);
    m_WPosX = 100;
    m_WPosY = 100;
    m_WWidth = 1280;
    m_WHeight = 720;
    m_WFullscreen = false;
    glfwWindowHint(GLFW_POSITION_X, m_WPosX);
    glfwWindowHint(GLFW_POSITION_Y, m_WPosY);
    m_Window = glfwCreateWindow(m_WWidth, m_WHeight, "EchoEngine", nullptr, nullptr);
    if (!m_Window)
    {
        glfwTerminate();
        return;
    }
    
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

    colors[ImGuiCol_Text]                   = ImVec4(0.94f, 0.94f, 0.94f, 1.00f);
    colors[ImGuiCol_TextDisabled]           = ImVec4(0.33f, 0.33f, 0.33f, 1.00f);
    colors[ImGuiCol_WindowBg]               = ImVec4(0.06f, 0.06f, 0.06f, 1.00f);
    colors[ImGuiCol_ChildBg]                = ImVec4(0.09f, 0.09f, 0.09f, 1.00f);
    colors[ImGuiCol_PopupBg]                = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);
    colors[ImGuiCol_Border]                 = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
    colors[ImGuiCol_BorderShadow]           = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_FrameBg]                = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);
    colors[ImGuiCol_FrameBgHovered]         = ImVec4(0.13f, 0.13f, 0.13f, 1.00f);
    colors[ImGuiCol_FrameBgActive]          = ImVec4(0.16f, 0.16f, 0.16f, 1.00f);
    colors[ImGuiCol_TitleBg]                = ImVec4(0.06f, 0.06f, 0.06f, 1.00f);
    colors[ImGuiCol_TitleBgActive]          = ImVec4(0.09f, 0.09f, 0.09f, 1.00f);
    colors[ImGuiCol_TitleBgCollapsed]       = ImVec4(0.06f, 0.06f, 0.06f, 1.00f);
    colors[ImGuiCol_MenuBarBg]              = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);
    colors[ImGuiCol_ScrollbarBg]            = ImVec4(0.06f, 0.06f, 0.06f, 1.00f);
    colors[ImGuiCol_ScrollbarGrab]          = ImVec4(0.22f, 0.22f, 0.22f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabHovered]   = ImVec4(0.33f, 0.33f, 0.33f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabActive]    = ImVec4(0.54f, 0.67f, 0.55f, 1.00f);
    colors[ImGuiCol_CheckMark]              = ImVec4(0.54f, 0.67f, 0.55f, 1.00f);
    colors[ImGuiCol_SliderGrab]             = ImVec4(0.54f, 0.67f, 0.55f, 1.00f);
    colors[ImGuiCol_SliderGrabActive]       = ImVec4(0.60f, 0.73f, 0.60f, 1.00f);
    colors[ImGuiCol_Button]                 = ImVec4(0.13f, 0.13f, 0.13f, 1.00f);
    colors[ImGuiCol_ButtonHovered]          = ImVec4(0.16f, 0.16f, 0.16f, 1.00f);
    colors[ImGuiCol_ButtonActive]           = ImVec4(0.22f, 0.22f, 0.22f, 1.00f);
    colors[ImGuiCol_Header]                 = ImVec4(0.13f, 0.13f, 0.13f, 1.00f);
    colors[ImGuiCol_HeaderHovered]          = ImVec4(0.16f, 0.16f, 0.16f, 1.00f);
    colors[ImGuiCol_HeaderActive]           = ImVec4(0.22f, 0.22f, 0.22f, 1.00f);
    colors[ImGuiCol_Separator]              = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
    colors[ImGuiCol_SeparatorHovered]       = ImVec4(0.22f, 0.22f, 0.22f, 1.00f);
    colors[ImGuiCol_SeparatorActive]        = ImVec4(0.54f, 0.67f, 0.55f, 1.00f);
    colors[ImGuiCol_ResizeGrip]             = ImVec4(0.13f, 0.13f, 0.13f, 1.00f);
    colors[ImGuiCol_ResizeGripHovered]      = ImVec4(0.22f, 0.22f, 0.22f, 1.00f);
    colors[ImGuiCol_ResizeGripActive]       = ImVec4(0.54f, 0.67f, 0.55f, 1.00f);
    colors[ImGuiCol_Tab]                    = ImVec4(0.09f, 0.09f, 0.09f, 1.00f);
    colors[ImGuiCol_TabHovered]             = ImVec4(0.16f, 0.16f, 0.16f, 1.00f);
    colors[ImGuiCol_TabActive]              = ImVec4(0.13f, 0.13f, 0.13f, 1.00f);
    colors[ImGuiCol_TabUnfocused]           = ImVec4(0.09f, 0.09f, 0.09f, 1.00f);
    colors[ImGuiCol_TabUnfocusedActive]     = ImVec4(0.13f, 0.13f, 0.13f, 1.00f);
    colors[ImGuiCol_PlotLines]              = ImVec4(0.54f, 0.67f, 0.55f, 1.00f);
    colors[ImGuiCol_PlotLinesHovered]       = ImVec4(0.60f, 0.73f, 0.60f, 1.00f);
    colors[ImGuiCol_PlotHistogram]          = ImVec4(0.54f, 0.67f, 0.55f, 1.00f);
    colors[ImGuiCol_PlotHistogramHovered]   = ImVec4(0.60f, 0.73f, 0.60f, 1.00f);
    colors[ImGuiCol_TableHeaderBg]          = ImVec4(0.13f, 0.13f, 0.13f, 1.00f);
    colors[ImGuiCol_TableBorderStrong]      = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
    colors[ImGuiCol_TableBorderLight]       = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
    colors[ImGuiCol_TableRowBg]             = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_TableRowBgAlt]          = ImVec4(1.00f, 1.00f, 1.00f, 0.06f);
    colors[ImGuiCol_TextSelectedBg]         = ImVec4(0.56f, 0.54f, 0.67f, 0.35f);
    colors[ImGuiCol_DragDropTarget]         = ImVec4(0.54f, 0.67f, 0.55f, 0.90f);
    colors[ImGuiCol_NavHighlight]           = ImVec4(0.54f, 0.67f, 0.55f, 1.00f);
    colors[ImGuiCol_NavWindowingHighlight]  = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
    colors[ImGuiCol_NavWindowingDimBg]      = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
    colors[ImGuiCol_ModalWindowDimBg]       = ImVec4(0.00f, 0.00f, 0.00f, 0.35f);
    colors[ImGuiCol_DockingPreview]         = ImVec4(0.54f, 0.67f, 0.55f, 0.70f);
    colors[ImGuiCol_DockingEmptyBg]         = ImVec4(0.06f, 0.06f, 0.06f, 1.00f);

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
    InputManager::GetInstance().BindAction("ToggleCursor", InputType::Key, GLFW_KEY_LEFT_CONTROL);
    InputManager::GetInstance().BindAction("Fullscreen",   InputType::Key, GLFW_KEY_F11);
	InputManager::GetInstance().BindAction("MoveForward",  InputType::Key, GLFW_KEY_W);
	InputManager::GetInstance().BindAction("MoveBackward", InputType::Key, GLFW_KEY_S);
	InputManager::GetInstance().BindAction("MoveLeft",	   InputType::Key, GLFW_KEY_A);
	InputManager::GetInstance().BindAction("MoveRight",    InputType::Key, GLFW_KEY_D);
	InputManager::GetInstance().BindAction("MoveUp",       InputType::Key, GLFW_KEY_SPACE);
	InputManager::GetInstance().BindAction("MoveDown",     InputType::Key, GLFW_KEY_LEFT_SHIFT);
	InputManager::GetInstance().BindAction("ReloadShaders",InputType::Key, GLFW_KEY_R);
}

bool DirectionGizmo(const char* label, glm::vec3& direction, float sense_normal = 0.001f, float sense_shift = 0.0001f) 
{
    bool value_changed = false;
    
    ImGui::BeginGroup();
    
    ImGuiIO& io = ImGui::GetIO();
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    ImGuiStorage* storage = ImGui::GetStateStorage();

    const float radius = 40.0f;
    const float thickness = 2.0f;
    
    ImVec2 p = ImGui::GetCursorScreenPos();
    ImVec2 center = ImVec2(p.x + radius, p.y + radius);
    ImGuiID id = ImGui::GetID(label);

    ImGui::InvisibleButton(label, ImVec2(radius * 2, radius * 2));
    
    bool is_active = ImGui::IsItemActive();
    bool is_hovered = ImGui::IsItemHovered();
    
    if (ImGui::IsItemActivated()) 
    {
        storage->SetFloat(id, io.MousePos.x);
        storage->SetFloat(id + 1, io.MousePos.y);
    }

    if (is_active) 
    {
        ImGui::SetMouseCursor(ImGuiMouseCursor_None);
        ImVec2 delta = io.MouseDelta;
        
        if (delta.x != 0.0f || delta.y != 0.0f) 
        {
            float sensitivity = InputManager::GetInstance().IsActionDown("MoveDown") ? sense_shift : sense_normal;
            
            float pitch = asinf(direction.y);
            float yaw = atan2f(direction.z, direction.x);

            yaw   += delta.x * sensitivity;
            pitch += delta.y * sensitivity;

            const float PITCH_LIMIT = glm::half_pi<float>() - 0.01f; 
            if (pitch > PITCH_LIMIT) pitch = PITCH_LIMIT;
            if (pitch < -PITCH_LIMIT) pitch = -PITCH_LIMIT;

            direction.y = sinf(pitch);
            float xz_plane_len = cosf(pitch);
            direction.x = xz_plane_len * cosf(yaw);
            direction.z = xz_plane_len * sinf(yaw);

            direction = glm::normalize(direction);
            value_changed = true;
            
            ImVec2 lock_pos = ImVec2(storage->GetFloat(id), storage->GetFloat(id + 1));
            io.MousePos = lock_pos;
            io.WantSetMousePos = true; 
        }
    }

    if (is_hovered && ImGui::IsMouseDoubleClicked(0)) 
    {
        direction = glm::vec3(0, 0, 1);
        value_changed = true;
    }

    ImU32 col_bg = ImGui::GetColorU32(ImGuiCol_FrameBg);
    ImU32 col_border = ImGui::GetColorU32(ImGuiCol_Text);
    ImU32 col_arrow = ImGui::GetColorU32(ImGuiCol_ButtonActive);

    if (is_active || is_hovered) 
    {
        col_border = ImGui::GetColorU32(ImGuiCol_TextDisabled); 
    }

    draw_list->AddCircleFilled(center, radius, col_bg);
    draw_list->AddCircle(center, radius, col_border, 32, thickness);

    float arrow_len = radius * 0.9f;
    ImVec2 arrow_end(
        center.x + direction.x * arrow_len, 
        center.y - direction.y * arrow_len
    );
    
    bool pointing_forward = direction.z > 0.0f;
    float arrow_width = pointing_forward ? 3.0f : 1.0f;
    
    draw_list->AddCircleFilled(center, 3.0f, col_border);
    draw_list->AddLine(center, arrow_end, col_arrow, arrow_width);
    draw_list->AddCircleFilled(arrow_end, 4.0f, col_arrow);

    float text_x = p.x + (radius * 2.0f) + 12.0f;
    float line_height = ImGui::GetTextLineHeight();
    
    ImGui::SetCursorScreenPos(ImVec2(text_x, center.y - line_height));
    ImGui::TextUnformatted(label);
    ImGui::SetCursorScreenPos(ImVec2(text_x, center.y)); 
    ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyle().Colors[ImGuiCol_TextDisabled]);
    ImGui::Text("X:%.2f Y:%.2f Z:%.2f", direction.x, direction.y, direction.z);
    ImGui::PopStyleColor();
    ImGui::EndGroup();
    ImGui::SetCursorScreenPos(ImVec2(p.x, p.y + radius * 2.0f + 5.0f));

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
    // m_Scene.m_Entities.push_back(&room);
    
    Entity ground;
    ground.LoadFromOBJ("assets/models/yosemite_valley2.obj");
    ground.Translate(glm::vec3(0.0, 0.0, 0.0));
    m_Scene.m_Entities.push_back(&ground);

    // Entity BistroExt;
    // BistroExt.LoadFromOBJ("assets/models/heavy/BistroExterior.obj");
    // BistroExt.SetScale(glm::vec3(0.01, 0.01, 0.01));
    // // BistroExt.SetPosition(glm::vec3(-89.0, 132.0, -867.0));
    // m_Scene.m_Entities.push_back(&BistroExt);

    // Entity BistroInt;
    // BistroInt.LoadFromOBJ("assets/models/heavy/interior.obj");
    // BistroInt.SetScale(glm::vec3(0.01, 0.01, 0.01));
    // m_Scene.m_Entities.push_back(&BistroInt);

    // Entity Sphere;
    // Sphere.LoadFromOBJ("assets/models/sphere.obj");
    // m_Scene.m_Entities.push_back(&Sphere);

    Camera t = Camera();
    t.SetPosition(glm::vec3(-1370.0, 91.0, 1145.0));
    t.SetPitch(0.5);
    t.SetYaw(318.13);
    t.m_MovementSpeed = 200.0f;
    
    m_Scene.m_Sun.Direction = glm::normalize(glm::vec3(-0.31, -0.30, 0.90));
    m_Scene.m_Sun.Color = glm::vec3(1.0f, 1.0f, 1.0f);
    m_Scene.m_Sun.Intensity = 20.0f;

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
        ImGui::Text("%.3f ms (%.1f FPS)", m_DeltaTime*1000.0, fps);
        static float fpsHistory[100] = { 0 };
        static int fpsHistoryIndex = 0;
        fpsHistory[fpsHistoryIndex] = fps;
        fpsHistoryIndex = (fpsHistoryIndex + 1) % 100;
        ImGui::PlotLines("FPS", fpsHistory, IM_ARRAYSIZE(fpsHistory), fpsHistoryIndex, nullptr, 0.0f, 100.0f, ImVec2(0, 80));
        glm::vec3 camPos = m_Scene.activeCamera->GetPosition();
        glm::vec3 camFront = m_Scene.activeCamera->GetFront();
        ImGui::DragFloat("Move speed", &m_Scene.activeCamera->m_MovementSpeed, 0.1f, 0.01f, 400.0f);
        ImGui::Text("Position: (%.2f, %.2f, %.2f)", camPos.x, camPos.y, camPos.z);
        ImGui::Text("Direction: (%.2f, %.2f, %.2f)", camFront.x, camFront.y, camFront.z);
        ImGui::Text("Pitch: %.2f, Yaw: %.2f", m_Scene.activeCamera->GetPitch(), m_Scene.activeCamera->GetYaw());

        ImGui::End();

        ImGui::Begin("GPU Profiler");

        auto& timerMap = RenderProfiler::GetTimerMap();
        auto& frameOrder = RenderProfiler::GetFrameOrder();

        float totalMs = 0.0f;
        for (const auto& name : frameOrder) totalMs += timerMap[name].TimeMs;

        ImGui::Text("Total Frame: %.3f ms", totalMs);

        ImVec2 pos = ImGui::GetCursorScreenPos();
        float width = ImGui::GetContentRegionAvail().x;
        float height = 30.0f;

        if (width > 10.0f && !frameOrder.empty())
        {
            ImDrawList* dl = ImGui::GetWindowDrawList();
            float targetMs = std::max(8.333f, totalMs);
            float currentX = 0.0f;

            dl->AddRectFilled(pos, ImVec2(pos.x + width, pos.y + height), ImColor(40, 40, 40), 5.0f);
            ImGui::InvisibleButton("##bar", ImVec2(width, height));

            for (const auto& name : frameOrder)
            {
                auto& timer = timerMap[name];
                float w = (timer.TimeMs / targetMs) * width;
                if (w < 1.0f) continue;

                dl->AddRectFilled(ImVec2(pos.x + currentX, pos.y), ImVec2(pos.x + currentX + w, pos.y + height), timer.Color);
                
                if (ImGui::IsMouseHoveringRect(ImVec2(pos.x + currentX, pos.y), ImVec2(pos.x + currentX + w, pos.y + height)))
                {
                    ImGui::SetTooltip("%s: %.3f ms", name.c_str(), timer.TimeMs);
                }
                currentX += w;
            }
        }

        for (const auto& name : frameOrder)
        {
            auto& timer = timerMap[name];
            ImGui::TextColored(timer.Color, "%-15s: %.3f ms", name.c_str(), timer.TimeMs);
        }

        frameOrder.clear(); 
        ImGui::End();

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
    
    if (glfwGetInputMode(m_Window, GLFW_CURSOR) == GLFW_CURSOR_DISABLED)
    {
        if (InputManager::GetInstance().IsActionDown("MoveUp"))       m_Scene.activeCamera->ProcessKeyboard(CameraMovement::UP,       m_DeltaTime);
        if (InputManager::GetInstance().IsActionDown("MoveDown"))     m_Scene.activeCamera->ProcessKeyboard(CameraMovement::DOWN,     m_DeltaTime);

        glm::vec2 mouseDelta = InputManager::GetInstance().GetMouseDelta();
        if (mouseDelta.x != 0.0f || mouseDelta.y != 0.0f)
        {
            m_Scene.activeCamera->ProcessMouseMovement(mouseDelta.x, -mouseDelta.y);
        }
    }

    if (InputManager::GetInstance().IsActionDown("MoveForward"))  m_Scene.activeCamera->ProcessKeyboard(CameraMovement::FORWARD,  m_DeltaTime);
    if (InputManager::GetInstance().IsActionDown("MoveBackward")) m_Scene.activeCamera->ProcessKeyboard(CameraMovement::BACKWARD, m_DeltaTime);
    if (InputManager::GetInstance().IsActionDown("MoveLeft"))     m_Scene.activeCamera->ProcessKeyboard(CameraMovement::LEFT,     m_DeltaTime);
    if (InputManager::GetInstance().IsActionDown("MoveRight"))    m_Scene.activeCamera->ProcessKeyboard(CameraMovement::RIGHT,    m_DeltaTime);

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
