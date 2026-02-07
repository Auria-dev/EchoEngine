#include "Renderer.h"
#include <iostream>

GLenum glCheckError_(const char *file, int line)
{
    GLenum errorCode;
    while ((errorCode = glGetError()) != GL_NO_ERROR)
    {
        std::string error;
        switch (errorCode)
        {
            case GL_INVALID_ENUM:                  error = "INVALID_ENUM"; break;
            case GL_INVALID_VALUE:                 error = "INVALID_VALUE"; break;
            case GL_INVALID_OPERATION:             error = "INVALID_OPERATION"; break;
            case GL_STACK_OVERFLOW:                error = "STACK_OVERFLOW"; break;
            case GL_STACK_UNDERFLOW:               error = "STACK_UNDERFLOW"; break;
            case GL_OUT_OF_MEMORY:                 error = "OUT_OF_MEMORY"; break;
            case GL_INVALID_FRAMEBUFFER_OPERATION: error = "INVALID_FRAMEBUFFER_OPERATION"; break;
        }
        std::cout << error << " | " << file << " (" << line << ")" << std::endl;
    }
    return errorCode;
}
#define glCheckError() glCheckError_(__FILE__, __LINE__) 

void Renderer::Init(int width, int height)
{
    m_Width = width;
    m_Height = height;

    m_Exposure = 1.0;
    m_ForwardShader = new Shader("assets/shaders/forward.vert", "assets/shaders/forward.frag");

    m_GBufferShader = new Shader("assets/shaders/gbuffer.vert", "assets/shaders/gbuffer.frag");
    m_LightingShader = new Shader("assets/shaders/fullscreen.vert", "assets/shaders/lighting.frag");
    m_SSAOShader = new Shader("assets/shaders/fullscreen.vert", "assets/shaders/ssao.frag");
    m_SSAOBlurShader = new Shader("assets/shaders/fullscreen.vert", "assets/shaders/ssaoblur.frag");
    m_SkyboxShader = new Shader("assets/shaders/skybox.vert", "assets/shaders/skybox.frag");

    // m_EquirectangularToCubemapShader = new Shader("assets/shaders/cubemap.vert", "assets/shaders/equirectangular_to_cubemap.frag");
    // m_IrradianceShader = new Shader("assets/shaders/cubemap.vert", "assets/shaders/irradiance_convolution.frag");
    // m_PrefilterShader = new Shader("assets/shaders/cubemap.vert", "assets/shaders/prefilter.frag");
    m_BrdfShader = new Shader("assets/shaders/fullscreen.vert", "assets/shaders/brdf.frag");
    
    // m_AtmosphereShader = new Shader("assets/shaders/fullscreen.vert", "assets/shaders/volumetric.frag");
    m_AtmosphereShader = new Shader("assets/shaders/fullscreen.vert", "assets/shaders/atmosphere.frag");
    m_TransmittanceShader = new Shader("assets/shaders/fullscreen.vert", "assets/shaders/transmittance.frag");
    m_MultiScatteringShader = new Shader("assets/shaders/fullscreen.vert", "assets/shaders/multi_scattering.frag");
    m_ShadowMapShader = new Shader("assets/shaders/shadow_map.vert", "assets/shaders/shadow_map.frag");

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

    m_GBuffer.quad.Init();
    
    GeometryInit();
    SSAOInit();
    LightingInit();
    SkyCaptureInit();    
    AtmosphereInit();
    ShadowMapInit();
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, m_Width, m_Height);

    m_ForwardShader->Bind();
    m_ForwardShader->SetUniform1i("uAlbedo", 0);
    m_ForwardShader->SetUniform1i("uNormal", 1);
    m_ForwardShader->SetUniform1i("uARM", 2);
}

void Renderer::Shutdown() { }

static void DebugTextureItem(const char* label, uint32_t texID, float width = 128.0f, float height = 72.0f)
{
    ImGui::BeginGroup();
    ImGui::Text("%s", label);
    ImGui::Image((void*)(intptr_t)texID, ImVec2(width, height), ImVec2(0, 1), ImVec2(1, 0));
    
    if (ImGui::IsItemHovered())
    {
        ImGui::BeginTooltip();
        ImGui::Text("ID: %d", texID);
        ImGui::Image((void*)(intptr_t)texID, ImVec2(width * 4, height * 4), ImVec2(0, 1), ImVec2(1, 0));
        ImGui::EndTooltip();
    }
    ImGui::EndGroup();
    
    ImGui::SameLine();
    if (ImGui::GetContentRegionAvail().x < width) ImGui::NewLine();
}

void Renderer::OnImGuiRender()
{
    ImGui::Begin("GPU Texture Debugger");

    if (ImGui::CollapsingHeader("GBuffers"))
    {
        DebugTextureItem("Position", m_GBuffer.Position);
        DebugTextureItem("Normal", m_GBuffer.Normal);
        DebugTextureItem("Albedo", m_GBuffer.Albedo);
        DebugTextureItem("ARM", m_GBuffer.ARM);
        DebugTextureItem("Depth", m_GBuffer.Depth);
        DebugTextureItem("Lighting", m_LightingResult);
    }
    
    ImGui::NewLine(); 

    if (ImGui::CollapsingHeader("Lighting"))
    {
        DebugTextureItem("SSAO Raw", m_SSAOColorBuffer);
        DebugTextureItem("SSAO Blur", m_SSAOBlurBuffer);
        // DebugTextureItem("BRDF LUT", m_BRDFLUTTexture, 128,128);

        ImGui::NewLine();
        if (ImGui::TreeNode("Cascaded Shadow Maps"))
        {
            for (uint i = 0; i < m_ShadowMapDebugTextures.size(); ++i)
            {
                std::string label = "Cascade " + std::to_string(i);
                DebugTextureItem(label.c_str(), m_ShadowMapDebugTextures[i], 150, 150);
                if (i < m_ShadowMapDebugTextures.size() - 1) ImGui::SameLine();
            }
            ImGui::NewLine();
            ImGui::Text("%zu cascades", m_ShadowCascadeLevels.size());
            ImGui::DragFloat("Cascade 1 dist", &m_ShadowCascadeLevelOne, 1.0, 0.0f, m_ShadowCascadeLevelTwo);
            ImGui::DragFloat("Cascade 2 dist", &m_ShadowCascadeLevelTwo, 1.0, m_ShadowCascadeLevelOne, m_ShadowCascadeLevelThree);
            ImGui::DragFloat("Cascade 3 dist", &m_ShadowCascadeLevelThree, 1.0, m_ShadowCascadeLevelTwo, m_ShadowCascadeLevelFour);
            ImGui::DragFloat("Cascade 4 dist", &m_ShadowCascadeLevelFour, 1.0, m_ShadowCascadeLevelThree, m_Scene->activeCamera->GetFar());
            ImGui::DragFloat("Maximum shadow depth", &m_FarShadowRenderDistance, 1.0, 50.0f, m_Scene->activeCamera->GetFar());

            ImGui::TreePop();
        }

        ImGui::NewLine();
        DebugTextureItem("Transmittance LUT", m_TransmittanceLUT, 256, 64);
        ImGui::NewLine();
        ImGui::DragFloat("Exposure", &m_Exposure, 0.05, 0.0, 3.0);
    }

    ImGui::NewLine();

    if (ImGui::CollapsingHeader("Material Cache (Uploaded)"))
    {
        int count = 0;
        for (const auto& [cpuTex, gpuTex] : m_TextureCache)
        {
            uint32_t id = gpuTex->GetID(); 
            std::string label = "Tex " + std::to_string(count++);
            
            DebugTextureItem(label.c_str(), id, 128,128);
        }
        
        if (m_TextureCache.empty())
        {
            ImGui::Text("No materials uploaded to cache yet.");
        }
    }
    
    ImGui::NewLine();

    std::string DrawCmdCount = "Opaque: " + std::to_string(m_DeferredQueue.size()) + " Transparent: " + std::to_string(m_ForwardQueue.size());
    ImGui::Text("%s",DrawCmdCount.c_str());

    ImGui::End();
}

void Renderer::BeginFrame()
{
	glBindFramebuffer(GL_FRAMEBUFFER, m_GBuffer.FBO);
	glViewport(0, 0, m_Width, m_Height);
	glEnable(GL_DEPTH_TEST);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_DeferredQueue.clear();
    m_ForwardQueue.clear();
}

void Renderer::EndFrame() { }

void Renderer::DrawScene()
{

    for (Entity* e : m_Scene->m_Entities)
    {
        SubmitDrawCmd(*e, *m_GBufferShader);
    }
    
    { ProfileScope p("Geometry"); GeometryPass(); }
    { ProfileScope p("SSAO"); SSAOPass(); }
    { ProfileScope p("SkyCap"); SkyCapturePass(); }
    { ProfileScope p("ShadowMap"); ShadowMapPass(); }
    { ProfileScope p("Lighting"); LightingPass(); }
    { ProfileScope p("Atmosphere"); AtmospherePass(); }
    { ProfileScope p("Forward"); ForwardPass(); }
}

void Renderer::Resize(int nWidth, int nHeight)
{
    glBindTexture(GL_TEXTURE_2D, m_GBuffer.Position);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, nWidth, nHeight, 0, GL_RGB, GL_FLOAT, NULL);

    glBindTexture(GL_TEXTURE_2D, m_GBuffer.Normal);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, nWidth, nHeight, 0, GL_RGBA, GL_FLOAT, NULL);

    glBindTexture(GL_TEXTURE_2D, m_GBuffer.Albedo);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, nWidth, nHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

    glBindTexture(GL_TEXTURE_2D, m_GBuffer.ARM);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, nWidth, nHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

    glBindTexture(GL_TEXTURE_2D, m_GBuffer.Depth);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, nWidth, nHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

    glBindTexture(GL_TEXTURE_2D, m_SSAOColorBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, nWidth, nHeight, 0, GL_RED, GL_FLOAT, NULL);

    glBindTexture(GL_TEXTURE_2D, m_SSAOBlurBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, nWidth, nHeight, 0, GL_RED, GL_FLOAT, NULL);

    glBindTexture(GL_TEXTURE_2D, m_LightingResult);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, nWidth, nHeight, 0, GL_RGBA, GL_FLOAT, NULL);

	m_Width = nWidth;
	m_Height = nHeight;
}

void Renderer::ReloadShaders()
{
    m_GBufferShader->Reload("assets/shaders/gbuffer.vert", "assets/shaders/gbuffer.frag");
	m_LightingShader->Reload("assets/shaders/fullscreen.vert", "assets/shaders/lighting.frag");
    m_SSAOShader->Reload("assets/shaders/fullscreen.vert", "assets/shaders/ssao.frag");
    m_SSAOBlurShader->Reload("assets/shaders/fullscreen.vert", "assets/shaders/ssaoblur.frag");
    m_SkyboxShader->Reload("assets/shaders/skybox.vert", "assets/shaders/skybox.frag");
    // m_EquirectangularToCubemapShader->Reload("assets/shaders/cubemap.vert", "assets/shaders/equirectangular_to_cubemap.frag");
    // m_IrradianceShader->Reload("assets/shaders/cubemap.vert", "assets/shaders/irradiance_convolution.frag");
    // m_PrefilterShader->Reload("assets/shaders/cubemap.vert", "assets/shaders/prefilter.frag");
    m_BrdfShader->Reload("assets/shaders/fullscreen.vert", "assets/shaders/brdf.frag");
    // m_AtmosphereShader->Reload("assets/shaders/fullscreen.vert", "assets/shaders/volumetric.frag");
    m_AtmosphereShader->Reload("assets/shaders/fullscreen.vert", "assets/shaders/atmosphere.frag");
    m_TransmittanceShader->Reload("assets/shaders/fullscreen.vert", "assets/shaders/transmittance.frag");
    m_MultiScatteringShader->Reload("assets/shaders/fullscreen.vert", "assets/shaders/multi_scattering.frag");
}

void Renderer::SubmitDrawCmd(const Entity& entity, Shader& shader)
{
    if (!entity.meshAsset) return;

    shader.Bind();
    shader.SetUniformMat4f("uModel", entity.transform);

    if (m_MeshCache.find(entity.meshAsset.get()) == m_MeshCache.end())
    {
        m_MeshCache[entity.meshAsset.get()] = std::make_unique<MeshResource>(*entity.meshAsset);
    }
    
    MeshResource* mesh = m_MeshCache[entity.meshAsset.get()].get();
    mesh->Bind();

    const std::vector<SubMesh>& subMeshes = entity.meshAsset->SubMeshes;
    for (int i = 0; i < subMeshes.size(); ++i)
    {
        const SubMesh& subMesh = subMeshes[i];
        
        if (subMesh.MaterialIndex < entity.materials.size())
        {
            Material* mat = entity.materials[subMesh.MaterialIndex].get();

            DrawCmd item;
            item.shadowCasting = true;
            item.Mesh = mesh;
            item.Material = entity.materials[subMesh.MaterialIndex];
            item.Model = entity.transform;
            item.SubMeshIndex = i;
            
            glm::vec4 worldCenter = item.Model * glm::vec4(subMesh.LocalCenter, 1.0f);
            glm::vec4 viewCenter  = m_Scene->activeCamera->GetViewMatrix() * worldCenter;
            item.depth = -viewCenter.z;
            
            m_DeferredQueue.push_back(item);
            // if (mat->Translucent) m_ForwardQueue.push_back(item);
            // else                  m_DeferredQueue.push_back(item);
        }
    }
}

RenderTexture* Renderer::GetGPUTexture(const Texture* cpuTexture)
{
    if (m_TextureCache.find(cpuTexture) == m_TextureCache.end())
    {
        m_TextureCache[cpuTexture] = std::make_unique<RenderTexture>(*cpuTexture);
    }

    return m_TextureCache[cpuTexture].get();
}

void Renderer::ClearCache()
{
    m_MeshCache.clear();
    m_TextureCache.clear();
}

void Renderer::BindMaterial(std::shared_ptr<Material> mat)
{
    if (mat->DiffuseTexture) GetGPUTexture(mat->DiffuseTexture.get())->Bind(0);
    if (mat->NormalTexture) GetGPUTexture(mat->NormalTexture.get())->Bind(1);
    if (mat->ARMTexture) GetGPUTexture(mat->ARMTexture.get())->Bind(2);
}
