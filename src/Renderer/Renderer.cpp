#include "Renderer.h"
#include <iostream>
#include <random>

void Renderer::Init(int width, int height)
{
    m_Width = width;
    m_Height = height;

    m_GBufferShader = new Shader("assets/shaders/gbuffer.vert", "assets/shaders/gbuffer.frag");
    m_LightingShader = new Shader("assets/shaders/fullscreen.vert", "assets/shaders/lighting.frag");
    m_SSAOShader = new Shader("assets/shaders/fullscreen.vert", "assets/shaders/ssao.frag");
    m_SSAOBlurShader = new Shader("assets/shaders/fullscreen.vert", "assets/shaders/ssaoblur.frag");

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
 
    // SSAO 
    glGenFramebuffers(1, &m_SSAOFBO);
    glGenFramebuffers(1, &m_SSAOBlurFBO);

    // SSAO color buffer
    glBindFramebuffer(GL_FRAMEBUFFER, m_SSAOFBO);
    
    glGenTextures(1, &m_SSAOColorBuffer);
    glBindTexture(GL_TEXTURE_2D, m_SSAOColorBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, m_Width, m_Height, 0, GL_RED, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_SSAOColorBuffer, 0);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        std::cout << "SSAO Framebuffer not complete!" << std::endl;
    }

    // SSAO blur buffer
    glBindFramebuffer(GL_FRAMEBUFFER, m_SSAOBlurFBO);
    glGenTextures(1, &m_SSAOBlurBuffer);
    glBindTexture(GL_TEXTURE_2D, m_SSAOBlurBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, m_Width, m_Height, 0, GL_RED, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_SSAOBlurBuffer, 0);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        std::cout << "SSAO Framebuffer not complete!" << std::endl;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // SSAO sample kernel
    std::uniform_real_distribution<GLfloat> randomFloats(0.0, 1.0);
    std::default_random_engine generator;

    m_SSAOShader->Bind();
    for (unsigned int i = 0; i < 64; ++i) {
        glm::vec3 sample(randomFloats(generator) * 2.0 - 1.0, randomFloats(generator) * 2.0 - 1.0, randomFloats(generator));
        sample = glm::normalize(sample);
        sample *= randomFloats(generator);
        float scale = float(i) / 64.0f;

        scale = 0.1f + scale * (1.0f - 0.1f);
        sample *= scale;
        m_SSAOKernel.push_back(sample);
        m_SSAOShader->SetUniform3f("samples[" + std::to_string(i) + "]", sample.x, sample.y, sample.z);
    }

    // SSAO noise texture
    std::vector<glm::vec3> ssaoNoise;
    for (uint i=0; i < 16; i++) {
        glm::vec3 noise(randomFloats(generator) * 2.0 - 1.0, randomFloats(generator) * 2.0 - 1.0, 0.0f);
        ssaoNoise.push_back(noise);
    }

    glGenTextures(1, &m_SSAONoise);
    glBindTexture(GL_TEXTURE_2D, m_SSAONoise);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, 4, 4, 0, GL_RGB, GL_FLOAT, &ssaoNoise[0]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    m_GBuffer.quad.Init();

    m_LightingShader->Bind();
    m_LightingShader->SetUniform1i("gPosition", 0);
    m_LightingShader->SetUniform1i("gNormal", 1);
    m_LightingShader->SetUniform1i("gAlbedo", 2);
    m_LightingShader->SetUniform1i("gARM", 3);
    m_LightingShader->SetUniform1i("gSSAO", 4);

    m_SSAOShader->Bind();
    m_SSAOShader->SetUniform1i("gPosition", 0);
    m_SSAOShader->SetUniform1i("gNormal", 1);
    m_SSAOShader->SetUniform1i("gNoiseTexture", 2);

    m_SSAOBlurShader->Bind();
    m_SSAOBlurShader->SetUniform1i("gSSAOInput", 0);

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
	glClearColor(0.53f, 0.8f, 0.92f, 1.0f);
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

    // ssao pass
    glBindFramebuffer(GL_FRAMEBUFFER, m_SSAOFBO);
    glClear(GL_COLOR_BUFFER_BIT);
    m_SSAOShader->Bind();
    m_SSAOShader->SetUniformMat4f("uProjection", scene.activeCamera->GetProjectionMatrix());
    m_SSAOShader->SetUniform2f("uResolution", m_Width, m_Height);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_GBuffer.Position);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, m_GBuffer.Normal);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, m_SSAONoise);
    m_GBuffer.quad.Draw();

    // blur SSAO texture
    glBindFramebuffer(GL_FRAMEBUFFER, m_SSAOBlurFBO);
    glClear(GL_COLOR_BUFFER_BIT);
    m_SSAOBlurShader->Bind();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_SSAOColorBuffer);
    m_GBuffer.quad.Draw();

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // lighting pass
    glViewport(0, 0, m_Width, m_Height);
    glDisable(GL_DEPTH_TEST);
    glClearColor(0.53f, 0.8f, 0.92f, 1.0f);
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
    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, m_SSAOBlurBuffer);
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

    glBindTexture(GL_TEXTURE_2D, m_GBuffer.Depth);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, nWidth, nHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

    glBindTexture(GL_TEXTURE_2D, m_SSAOColorBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, nWidth, nHeight, 0, GL_RED, GL_FLOAT, NULL);

    glBindTexture(GL_TEXTURE_2D, m_SSAOBlurBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, nWidth, nHeight, 0, GL_RED, GL_FLOAT, NULL);

	m_Width = nWidth;
	m_Height = nHeight;
}

void Renderer::ReloadShaders()
{
    m_GBufferShader->Reload("assets/shaders/gbuffer.vert", "assets/shaders/gbuffer.frag");
	m_LightingShader->Reload("assets/shaders/fullscreen.vert", "assets/shaders/lighting.frag");
    m_SSAOShader->Reload("assets/shaders/fullscreen.vert", "assets/shaders/ssao.frag");
    m_SSAOBlurShader->Reload("assets/shaders/fullscreen.vert", "assets/shaders/ssaoblur.frag");
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