#include "../Renderer.h"

void Renderer::ForwardPass()
{
    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    m_ForwardShader->Bind();
    m_ForwardShader->SetUniformMat4f("uView", m_Scene->activeCamera->GetViewMatrix());
    m_ForwardShader->SetUniformMat4f("uProjection", m_Scene->activeCamera->GetProjectionMatrix());

    glEnable(GL_BLEND);
    glDisable(GL_CULL_FACE);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_FALSE); 

    std::sort(m_ForwardQueue.begin(), m_ForwardQueue.end(), [](const DrawCmd& a, const DrawCmd& b) { return a.depth > b.depth; });

    for (const DrawCmd& cmd : m_ForwardQueue)
    {
        m_ForwardShader->SetUniformMat4f("uModel", cmd.Model);
        
        float opacity = cmd.Material ? cmd.Material->Dissolve : 1.0f;
        m_ForwardShader->SetUniform1f("uOpacity", opacity);

        BindMaterial(cmd.Material);
        cmd.Mesh->Bind();
        cmd.Mesh->DrawSubMesh(cmd.SubMeshIndex);
    }
    
    glDepthMask(GL_TRUE);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
}