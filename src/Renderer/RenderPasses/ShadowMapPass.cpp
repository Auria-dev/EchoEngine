#include "../Renderer.h"

void Renderer::ShadowMapPass()
{
    m_ShadowMapShader->Bind();
    glm::vec3 lightFocus = glm::vec3(0.0f);
    m_LightView = glm::lookAt(lightFocus - glm::normalize(m_Scene->m_Sun.Direction) * m_LightDistance, lightFocus, glm::vec3(0.0f, 1.0f, 0.0f));
    m_LightProj = m_OrthoProj * m_LightView;
    m_ShadowMapShader->SetUniformMat4f("uLightProj", m_LightProj);

    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    // glCullFace(GL_FRONT);
    glViewport(0,0,m_ShadowWidth, m_ShadowHeight);
    glBindFramebuffer(GL_FRAMEBUFFER, m_ShadowMapFBO);
    glClear(GL_DEPTH_BUFFER_BIT);
    for (const DrawCmd& cmd : m_DeferredQueue)
    {
        if (!cmd.shadowCasting) continue;
        m_ShadowMapShader->SetUniformMat4f("uModel", cmd.Model);
        BindMaterial(cmd.Material);
        cmd.Mesh->Bind();
        cmd.Mesh->DrawSubMesh(cmd.SubMeshIndex);
    }
}