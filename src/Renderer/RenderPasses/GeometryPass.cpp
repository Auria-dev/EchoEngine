#include "../Renderer.h"

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
        BindMaterial(cmd.Material);
        cmd.Mesh->Bind();
        cmd.Mesh->DrawSubMesh(cmd.SubMeshIndex);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}