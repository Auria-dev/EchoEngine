#include "../Renderer.h"

void Renderer::LightingPass()
{
    glBindFramebuffer(GL_FRAMEBUFFER, m_LightingFBO);
    glViewport(0, 0, m_Width, m_Height);
    glDisable(GL_DEPTH_TEST);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    
    m_LightingShader->Bind();

    int dirLightCount = 0;
    int pointLightCount = 0;
    int spotLightCount = 0;

    for (const Light* light : m_Scene->m_Lights)
    {
        switch (light->GetType())
        {
            case Light::LightType::Directional:
            {
            //     const DirectionalLight* dLight = static_cast<const DirectionalLight*>(light);
                
            //     std::string base = "uDirLights[" + std::to_string(dirLightCount) + "]";
            //     glm::vec4 dir = m_Scene->activeCamera->GetViewMatrix() * glm::vec4(dLight->Direction, 0.0f);

            //     m_LightingShader->SetUniform3f(base + ".direction", dir.x, dir.y, dir.z);
            //     m_LightingShader->SetUniform3f(base + ".color",     dLight->Color.x, dLight->Color.y, dLight->Color.z);
            //     m_LightingShader->SetUniform1f(base + ".intensity", dLight->Intensity);
                
            //     dirLightCount++;
            } break;

            case Light::LightType::Point:
            {
                const PointLight* pLight = static_cast<const PointLight*>(light);
                
                std::string base = "uPointLights[" + std::to_string(pointLightCount) + "]";
                glm::vec4 pos = m_Scene->activeCamera->GetViewMatrix() * glm::vec4(pLight->Position, 1.0f);

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
                glm::vec4 pos = m_Scene->activeCamera->GetViewMatrix() * glm::vec4(sLight->Position, 1.0f);
                glm::vec4 dir = m_Scene->activeCamera->GetViewMatrix() * glm::vec4(sLight->Direction, 0.0f);

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

    // m_LightingShader->SetUniform1i("uDirLightCount", dirLightCount);
    m_LightingShader->SetUniform1i("uPointLightCount", pointLightCount);
    m_LightingShader->SetUniform1i("uSpotLightCount", spotLightCount);
    m_LightingShader->SetUniformMat4f("uInverseView", glm::inverse(m_Scene->activeCamera->GetViewMatrix()));
    m_LightingShader->SetUniformMat4f("uLightProj", m_LightProj);

    m_LightingShader->SetUniform3f("uSunDirection", m_Scene->m_Sun.Direction.x, m_Scene->m_Sun.Direction.y, m_Scene->m_Sun.Direction.z);
    m_LightingShader->SetUniform3f("uSunColor",     m_Scene->m_Sun.Color.x, m_Scene->m_Sun.Color.y, m_Scene->m_Sun.Color.z);
    m_LightingShader->SetUniform1f("uSunIntensity", m_Scene->m_Sun.Intensity);

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
    glActiveTexture(GL_TEXTURE5);
    glBindTexture(GL_TEXTURE_2D, m_ShadowMap);
    glActiveTexture(GL_TEXTURE6);
    glBindTexture(GL_TEXTURE_2D, m_TransmittanceLUT);
    glActiveTexture(GL_TEXTURE7);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_SkyProbeMap);

    m_GBuffer.quad.Draw();
}