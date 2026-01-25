#include "../Renderer.h"

void Renderer::SkyCapture()
{
    glViewport(0, 0, m_SkyCaptureSize, m_SkyCaptureSize);
    glBindFramebuffer(GL_FRAMEBUFFER, m_SkyProbeFBO);
    
    glDisable(GL_CULL_FACE); 
    glDisable(GL_DEPTH_TEST);
    
    m_AtmosphereShader->Bind();

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, m_TransmittanceLUT);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, m_MultiScatteringLUT);
    
    m_AtmosphereShader->SetUniform3f("viewPos", 0.0, 0.0, 0.0);
    m_AtmosphereShader->SetUniform1f("exposure", m_Exposure);
    m_AtmosphereShader->SetUniform3f("uLightDir", -m_Scene->m_Sun.Direction.x, -m_Scene->m_Sun.Direction.y, -m_Scene->m_Sun.Direction.z);
    m_AtmosphereShader->SetUniform1i("uIsIBLPass", 1);

    glm::mat4 proj = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);    glm::mat4 captureViews[] = {
        glm::lookAt(glm::vec3(0.0f), glm::vec3( 1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
        glm::lookAt(glm::vec3(0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
        glm::lookAt(glm::vec3(0.0f), glm::vec3( 0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
        glm::lookAt(glm::vec3(0.0f), glm::vec3( 0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
        glm::lookAt(glm::vec3(0.0f), glm::vec3( 0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
        glm::lookAt(glm::vec3(0.0f), glm::vec3( 0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
    };
    
    for (unsigned int i = 0; i < 6; ++i)
    {
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, m_SkyProbeMap, 0);
        glm::mat4 viewProj = proj * captureViews[i];
        m_AtmosphereShader->SetUniformMat4f("uInvViewProj", glm::inverse(viewProj)); 
        
        glClear(GL_COLOR_BUFFER_BIT);
        m_GBuffer.quad.Draw(); 
    }

    glBindTexture(GL_TEXTURE_CUBE_MAP, m_SkyProbeMap);
    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

    glBindFramebuffer(GL_READ_FRAMEBUFFER, m_SkyProbeFBO);
    glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X, m_SkyProbeMap, 0);
}