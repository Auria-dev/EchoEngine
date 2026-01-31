#include "../Renderer.h"

std::vector<glm::vec4> Renderer::GetFrustumCornersWorldSpace(const glm::mat4& proj, const glm::mat4& view)
{
    const auto inv = glm::inverse(proj * view);
    
    std::vector<glm::vec4> frustumCorners;
    for (unsigned int x = 0; x < 2; ++x)
    {
        for (unsigned int y = 0; y < 2; ++y)
        {
            for (unsigned int z = 0; z < 2; ++z)
            {
                const glm::vec4 pt = 
                    inv * glm::vec4(
                        2.0f * x - 1.0f,
                        2.0f * y - 1.0f,
                        2.0f * z - 1.0f,
                        1.0f);
                frustumCorners.push_back(pt / pt.w);
            }
        }
    }
    return frustumCorners;
}

glm::mat4 Renderer::GetLightSpaceMatrix(const float nearPlane, const float farPlane)
{
    const auto proj = glm::perspective(
        glm::radians(m_Scene->activeCamera->GetFOV()), 
        (float)m_Width / (float)m_Height, 
        nearPlane, 
        farPlane
    );

    const auto corners = GetFrustumCornersWorldSpace(proj, m_Scene->activeCamera->GetViewMatrix());

    glm::vec3 center = glm::vec3(0, 0, 0);
    for (const auto& v : corners)
    {
        center += glm::vec3(v);
    }
    center /= corners.size();

    float radius = 0.0f;
    for (const auto& v : corners)
    {
        float distance = glm::length(glm::vec3(v) - center);
        radius = std::max(radius, distance);
    }
    
    float texelsPerUnit = m_ShadowMapResolution / (radius * 2.0f);
    glm::mat4 tempLightView = glm::lookAt(
        glm::vec3(0.0f), 
        glm::normalize(m_Scene->m_Sun.Direction), 
        glm::vec3(0.0f, 1.0f, 0.0f)
    );
    
    glm::vec3 centerLS = glm::vec3(tempLightView * glm::vec4(center, 1.0f));
    centerLS *= texelsPerUnit;
    centerLS.x = std::floor(centerLS.x);
    centerLS.y = std::floor(centerLS.y);
    centerLS /= texelsPerUnit;

    glm::vec3 centerWorld = glm::vec3(glm::inverse(tempLightView) * glm::vec4(centerLS, 1.0f));

    float farShadowRenderDistance = 3000.0f; 
    
    glm::mat4 lightProjection = glm::ortho(
        -radius, radius,
        -radius, radius,
        -farShadowRenderDistance,
        radius
    );

    glm::mat4 lightView = glm::lookAt(
        centerWorld,
        centerWorld + glm::normalize(m_Scene->m_Sun.Direction),
        glm::vec3(0.0f, 1.0f, 0.0f)
    );

    return lightProjection * lightView;
}

void Renderer::ShadowMapInit()
{
    m_ShadowCascadeLevelOne   = 50.0f;
    m_ShadowCascadeLevelTwo   = 200.0f;
    m_ShadowCascadeLevelThree = 1000.0f;
    m_ShadowCascadeLevelFour  = 2500.0f;

    m_ShadowMapSplit = 5;
    m_ShadowMapResolution = 2048;
    glGenFramebuffers(1, &m_ShadowMapFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, m_ShadowMapFBO);
    
    glGenTextures(1, &m_ShadowMapTexture);
    glBindTexture(GL_TEXTURE_2D_ARRAY, m_ShadowMapTexture);
    
    glBindTexture(GL_TEXTURE_2D_ARRAY, m_ShadowMapTexture);
    glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_DEPTH_COMPONENT32F, m_ShadowMapResolution, m_ShadowMapResolution, m_ShadowMapSplit, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
    
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_ShadowMapTexture, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

    m_ShadowMapDebugTextures.resize(m_ShadowMapSplit);
    glGenTextures(m_ShadowMapSplit, m_ShadowMapDebugTextures.data());

    for (uint i = 0; i < m_ShadowMapSplit; ++i)
    {
        glBindTexture(GL_TEXTURE_2D, m_ShadowMapDebugTextures[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, m_ShadowMapResolution, m_ShadowMapResolution, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        GLint swizzleMask[] = {GL_RED, GL_RED, GL_RED, GL_ONE};
        glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, swizzleMask);
    }
}


void Renderer::ShadowMapPass()
{
    m_ShadowCascadeLevels = { m_Scene->activeCamera->GetNear(), m_ShadowCascadeLevelOne, m_ShadowCascadeLevelTwo, m_ShadowCascadeLevelThree, m_ShadowCascadeLevelFour, m_Scene->activeCamera->GetFar() };
    m_ShadowCascadeMatrices.clear();

    m_ShadowMapShader->Bind();
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    
    glBindFramebuffer(GL_FRAMEBUFFER, m_ShadowMapFBO);
    glViewport(0, 0, m_ShadowMapResolution, m_ShadowMapResolution);

    for (size_t i = 0; i < m_ShadowCascadeLevels.size() - 1; ++i)
    {
        glm::mat4 lightSpaceMatrix = GetLightSpaceMatrix(m_ShadowCascadeLevels[i], m_ShadowCascadeLevels[i+1]);
        m_ShadowCascadeMatrices.push_back(lightSpaceMatrix);
        
        m_ShadowMapShader->SetUniformMat4f("uLightProj", lightSpaceMatrix);

        glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, m_ShadowMapTexture, 0, i);
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
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // for debugging
    if (m_ShadowMapDebugTextures.empty()) return;
    for (uint i = 0; i < m_ShadowMapSplit; ++i)
    {
        glCopyImageSubData(
            m_ShadowMapTexture, GL_TEXTURE_2D_ARRAY, 0, 0, 0, i,
            m_ShadowMapDebugTextures[i], GL_TEXTURE_2D, 0, 0, 0, 0,
            m_ShadowMapResolution, m_ShadowMapResolution, 1
        );
    }
}