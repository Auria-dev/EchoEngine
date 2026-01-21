#include "../Renderer.h"

void Renderer::SSAOPass()
{
    glBindFramebuffer(GL_FRAMEBUFFER, m_SSAOFBO);
    glClear(GL_COLOR_BUFFER_BIT);
    m_SSAOShader->Bind();
    m_SSAOShader->SetUniformMat4f("uProjection", m_Scene->activeCamera->GetProjectionMatrix());
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

}