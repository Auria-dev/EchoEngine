#include "../Renderer.h"

void Renderer::AtmospherePass()
{
    glBindFramebuffer(GL_READ_FRAMEBUFFER, m_GBuffer.FBO);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glBlitFramebuffer(0, 0, m_Width, m_Height, 0, 0, m_Width, m_Height, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    m_AtmosphereShader->Bind();

    glm::mat4 proj = m_Scene->activeCamera->GetProjectionMatrix();
    glm::mat4 view = m_Scene->activeCamera->GetViewMatrix();
    glm::mat4 invViewProj = glm::inverse(proj * view);
    glm::vec3 skyLightColor = m_Scene->m_Sun.Color * m_Scene->m_Sun.Intensity;
    glm::vec3 camPos = m_Scene->activeCamera->GetPosition();

    m_AtmosphereShader->SetUniform1f("exposure", m_Exposure);
    m_AtmosphereShader->SetUniform3f("viewPos", camPos.x, camPos.y, camPos.z);
    m_AtmosphereShader->SetUniform3f("uLightDir", -m_Scene->m_Sun.Direction.x, -m_Scene->m_Sun.Direction.y, -m_Scene->m_Sun.Direction.z);
    m_AtmosphereShader->SetUniformMat4f("uInvViewProj", invViewProj);
    m_AtmosphereShader->SetUniformMat4f("uLightProj", m_LightProj);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_GBuffer.Depth);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, m_TransmittanceLUT);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, m_MultiScatteringLUT);
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, m_ShadowMap); 

    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_SRC_ALPHA);

    m_GBuffer.quad.Draw();
}