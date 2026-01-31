#include "../Renderer.h"

void Renderer::AtmospherePass()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

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
    m_AtmosphereShader->SetUniform1i("uCascadeCount", m_ShadowCascadeLevels.size());
    for (size_t i = 0; i < m_ShadowCascadeMatrices.size(); ++i)
    {
        std::string name = "uCascadeMatrices[" + std::to_string(i) + "]";
        m_AtmosphereShader->SetUniformMat4f(name, m_ShadowCascadeMatrices[i]);
    }

    m_AtmosphereShader->SetUniform1i("uIsIBLPass", 0);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_GBuffer.Depth);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, m_TransmittanceLUT);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, m_MultiScatteringLUT);
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D_ARRAY, m_ShadowMapTexture);
    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, m_LightingResult);

    glDisable(GL_BLEND); 
    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);

    m_GBuffer.quad.Draw();
}