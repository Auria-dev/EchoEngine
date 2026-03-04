#include "../Renderer.h"
#include <random>
#include <algorithm>

void Renderer::SSAOInit()
{
    m_EnableSSAO = true;
    m_DebugSSAO = false;
    
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
    for (uint i = 0; i < 64; ++i) 
    {
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
    for (uint i=0; i < 16; i++)
    {
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
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    
    m_SSAOShader->Bind();
    m_SSAOShader->SetUniform1i("gPosition", 0);
    m_SSAOShader->SetUniform1i("gNormal", 1);
    m_SSAOShader->SetUniform1i("gNoiseTexture", 2);
    m_SSAOBlurShader->Bind();
    m_SSAOBlurShader->SetUniform1i("gSSAOInput", 0);
}

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