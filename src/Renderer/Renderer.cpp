#include "Renderer.h"
#include <iostream>

void Renderer::Init(int width, int height)
{
    m_Width = width;
    m_Height = height;

    // glEnable(GL_BLEND);
	// glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    glViewport(0, 0, m_Width, m_Height);

    // fbo
    glGenFramebuffers(1, &m_GBuffer.FBO);
    glBindFramebuffer(GL_FRAMEBUFFER, m_GBuffer.FBO);

    // position rgb
    glGenTextures(1, &m_GBuffer.Position);
    glBindTexture(GL_TEXTURE_2D, m_GBuffer.Position);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, m_Width, m_Height, 0, GL_RGB, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_GBuffer.Position, 0);

    // normal rgb
    glGenTextures(1, &m_GBuffer.Normal);
    glBindTexture(GL_TEXTURE_2D, m_GBuffer.Normal);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, m_Width, m_Height, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, m_GBuffer.Normal, 0);

    // albedo rgb
    glGenTextures(1, &m_GBuffer.Albedo);
    glBindTexture(GL_TEXTURE_2D, m_GBuffer.Albedo);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m_Width, m_Height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, m_GBuffer.Albedo, 0);

    // ARM texture rgb
    glGenTextures(1, &m_GBuffer.ARM);
    glBindTexture(GL_TEXTURE_2D, m_GBuffer.ARM);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m_Width, m_Height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, m_GBuffer.ARM, 0);

    uint attachments[4] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3 };
    glDrawBuffers(4, attachments);

    // depth
    glGenTextures(1, &m_GBuffer.Depth);
    glBindTexture(GL_TEXTURE_2D, m_GBuffer.Depth);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, m_Width, m_Height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_GBuffer.Depth, 0);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        std::cout << "GBuffer framebuffer not complete!" << std::endl;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    m_GBuffer.quad.Init();

    // load shaders
    m_GBufferShader = new Shader("assets/shaders/gbuffer.vert", "assets/shaders/gbuffer.frag");
    m_LightingShader = new Shader("assets/shaders/fullscreen.vert", "assets/shaders/lighting.frag");

    m_LightingShader->Bind();
    m_LightingShader->SetUniform1i("gPosition", 0);
    m_LightingShader->SetUniform1i("gNormal", 1);
    m_LightingShader->SetUniform1i("gAlbedo", 2);
    m_LightingShader->SetUniform1i("gARM", 3);

    m_GBufferShader->Bind();
    m_GBufferShader->SetUniform1i("uAlbedo", 0);
    m_GBufferShader->SetUniform1i("uNormal", 1);
    m_GBufferShader->SetUniform1i("uARM", 2);
}

void Renderer::Shutdown() { }

void Renderer::BeginFrame()
{
	glBindFramebuffer(GL_FRAMEBUFFER, m_GBuffer.FBO);
	glViewport(0, 0, m_Width, m_Height);
	glEnable(GL_DEPTH_TEST);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Renderer::EndFrame() { }

void Renderer::DrawScene(const SceneData& scene)
{
    // geometry pass
    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);

    m_GBufferShader->Bind();
    m_GBufferShader->SetUniformMat4f("uView", scene.activeCamera->GetViewMatrix());
    m_GBufferShader->SetUniformMat4f("uProjection", scene.activeCamera->GetProjectionMatrix());

    for (Entity* e : scene.Entities)
    {
        DrawEntity(*e, *m_GBufferShader);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // lighting pass
    glViewport(0, 0, m_Width, m_Height);
    glDisable(GL_DEPTH_TEST);
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    
    m_LightingShader->Bind();

    int dirLightCount = 0;
    int pointLightCount = 0;
    int spotLightCount = 0;

    for (const Light* light : scene.Lights)
    {
        switch (light->GetType())
        {
            case Light::LightType::Directional:
            {
                const DirectionalLight* dLight = static_cast<const DirectionalLight*>(light);
                
                std::string base = "uDirLights[" + std::to_string(dirLightCount) + "]";
                glm::vec4 dir = scene.activeCamera->GetViewMatrix() * glm::vec4(dLight->Direction, 0.0f);

                m_LightingShader->SetUniform3f(base + ".direction", dir.x, dir.y, dir.z);
                m_LightingShader->SetUniform3f(base + ".color",     dLight->Color.x, dLight->Color.y, dLight->Color.z);
                m_LightingShader->SetUniform1f(base + ".intensity", dLight->Intensity);

                dirLightCount++;
            } break;

            case Light::LightType::Point:
            {
                const PointLight* pLight = static_cast<const PointLight*>(light);
                
                std::string base = "uPointLights[" + std::to_string(pointLightCount) + "]";
                glm::vec4 pos = scene.activeCamera->GetViewMatrix() * glm::vec4(pLight->Position, 1.0f);

                m_LightingShader->SetUniform3f(base + ".position",  pos.x,  pos.y, pos.z);
                m_LightingShader->SetUniform3f(base + ".color",     pLight->Color.x, pLight->Color.y, pLight->Color.z);
                m_LightingShader->SetUniform1f(base + ".intensity", pLight->Intensity);
                
                m_LightingShader->SetUniform1f(base + ".constant",  pLight->Constant);
                m_LightingShader->SetUniform1f(base + ".linear",    pLight->Linear);
                m_LightingShader->SetUniform1f(base + ".quadratic", pLight->Quadratic);

                pointLightCount++;
            } break;

            case Light::LightType::Spotlight: 
            {
                const SpotLight* sLight = static_cast<const SpotLight*>(light);
                
                std::string base = "uSpotLights[" + std::to_string(spotLightCount) + "]";
                glm::vec4 pos = scene.activeCamera->GetViewMatrix() * glm::vec4(sLight->Position, 1.0f);
                glm::vec4 dir = scene.activeCamera->GetViewMatrix() * glm::vec4(sLight->Direction, 0.0f);

                m_LightingShader->SetUniform3f(base + ".position",  pos.x,  pos.y, pos.z);
                m_LightingShader->SetUniform3f(base + ".direction", dir.x,  dir.y, dir.z);
                m_LightingShader->SetUniform3f(base + ".color",     sLight->Color.x, sLight->Color.y, sLight->Color.z);
                m_LightingShader->SetUniform1f(base + ".intensity", sLight->Intensity);

                m_LightingShader->SetUniform1f(base + ".innerCutoff", glm::cos(glm::radians(sLight->InnerCutoff)));
                m_LightingShader->SetUniform1f(base + ".outerCutoff", glm::cos(glm::radians(sLight->OuterCutoff)));

                spotLightCount++; 
            } break;
        }
    }

    m_LightingShader->SetUniform1i("uDirLightCount", dirLightCount);
    m_LightingShader->SetUniform1i("uPointLightCount", pointLightCount);
    m_LightingShader->SetUniform1i("uSpotLightCount", spotLightCount);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_GBuffer.Position);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, m_GBuffer.Normal);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, m_GBuffer.Albedo);
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, m_GBuffer.ARM);
    m_GBuffer.quad.Draw();

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

    // glBindRenderbuffer(GL_RENDERBUFFER, m_GBuffer.Depth);
    // glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, nWidth, nHeight);

    glBindTexture(GL_TEXTURE_2D, m_GBuffer.Depth);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, nWidth, nHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

	m_Width = nWidth;
	m_Height = nHeight;
}

void Renderer::ReloadShaders()
{
    m_GBufferShader->Reload("assets/shaders/gbuffer.vert", "assets/shaders/gbuffer.frag");
	m_LightingShader->Reload("assets/shaders/fullscreen.vert", "assets/shaders/lighting.frag");
}

void Renderer::DrawEntity(const Entity& entity, Shader& shader)
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

    const auto& subMeshes = entity.meshAsset->SubMeshes;
    for (int i = 0; i < subMeshes.size(); ++i)
    {
        const SubMesh& subMesh = subMeshes[i];        
        if (subMesh.MaterialIndex < entity.materials.size())
        {
            auto cpuMat = entity.materials[subMesh.MaterialIndex];
            if (cpuMat->DiffuseTexture)
            {
                RenderTexture* tex = GetGPUTexture(cpuMat->DiffuseTexture.get());
                tex->Bind(0); 
            }
            
            if (cpuMat->NormalTexture)
            {
                RenderTexture* tex = GetGPUTexture(cpuMat->NormalTexture.get());
                tex->Bind(1);
            }

            if (cpuMat->ARMTexture)
            {
                RenderTexture* tex = GetGPUTexture(cpuMat->ARMTexture.get());
                tex->Bind(2);
            }
        }

        mesh->DrawSubMesh(i);
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