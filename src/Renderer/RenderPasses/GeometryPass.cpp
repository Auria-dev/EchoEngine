#include "../Renderer.h"

void Renderer::GeometryInit()
{
    glViewport(0, 0, m_Width, m_Height);

    // fbo
    glGenFramebuffers(1, &m_GBuffer.FBO);
    glBindFramebuffer(GL_FRAMEBUFFER, m_GBuffer.FBO);

    // position rgb
    glGenTextures(1, &m_GBuffer.Position);
    glBindTexture(GL_TEXTURE_2D, m_GBuffer.Position);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, m_Width, m_Height, 0, GL_RGBA, GL_FLOAT, NULL);
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
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_Width, m_Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, m_GBuffer.Albedo, 0);

    // ARM texture rgb
    glGenTextures(1, &m_GBuffer.ARM);
    glBindTexture(GL_TEXTURE_2D, m_GBuffer.ARM);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_Width, m_Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
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
    
    m_GBufferShader->Bind();
    m_GBufferShader->SetUniform1i("uAlbedo", 0);
    m_GBufferShader->SetUniform1i("uNormal", 1);
    m_GBufferShader->SetUniform1i("uARM", 2);
}

void Renderer::GeometryPass()
{
    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    m_GBufferShader->Bind();
    m_GBufferShader->SetUniformMat4f("uView", m_Scene->activeCamera->GetViewMatrix());
    m_GBufferShader->SetUniformMat4f("uProjection", m_Scene->activeCamera->GetProjectionMatrix());

    for (const DrawCmd& cmd : m_DeferredQueue)
    {
        m_GBufferShader->SetUniformMat4f("uModel", cmd.Model);
        m_GBufferShader->SetUniform1i("uTiling", cmd.Tiling);
        BindMaterial(cmd.Material);
        cmd.Mesh->Bind();
        cmd.Mesh->DrawSubMesh(cmd.SubMeshIndex);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}