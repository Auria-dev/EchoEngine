#include "Renderer.h"
#include <iostream>
#include <random>

void Renderer::Init(int width, int height)
{
    m_Width = width;
    m_Height = height;

    m_GBufferShader = new Shader("assets/shaders/gbuffer.vert", "assets/shaders/gbuffer.frag");
    m_LightingShader = new Shader("assets/shaders/fullscreen.vert", "assets/shaders/lighting.frag");
    m_SSAOShader = new Shader("assets/shaders/fullscreen.vert", "assets/shaders/ssao.frag");
    m_SSAOBlurShader = new Shader("assets/shaders/fullscreen.vert", "assets/shaders/ssaoblur.frag");
    m_SkyboxShader = new Shader("assets/shaders/skybox.vert", "assets/shaders/skybox.frag");

    m_EquirectangularToCubemapShader = new Shader("assets/shaders/cubemap.vert", "assets/shaders/equirectangular_to_cubemap.frag");
    m_IrradianceShader = new Shader("assets/shaders/cubemap.vert", "assets/shaders/irradiance_convolution.frag");
    m_PrefilterShader = new Shader("assets/shaders/cubemap.vert", "assets/shaders/prefilter.frag");
    m_BrdfShader = new Shader("assets/shaders/fullscreen.vert", "assets/shaders/brdf.frag");

    // glEnable(GL_BLEND);
	// glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

    glViewport(0, 0, m_Width, m_Height);

    // fbo
    glGenFramebuffers(1, &m_GBuffer.FBO);
    glBindFramebuffer(GL_FRAMEBUFFER, m_GBuffer.FBO);

    // position rgb
    glGenTextures(1, &m_GBuffer.Position);
    glBindTexture(GL_TEXTURE_2D, m_GBuffer.Position);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, m_Width, m_Height, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_GBuffer.Position, 0);

    // normal rgb
    glGenTextures(1, &m_GBuffer.Normal);
    glBindTexture(GL_TEXTURE_2D, m_GBuffer.Normal);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, m_Width, m_Height, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, m_GBuffer.Normal, 0);

    // albedo rgb
    glGenTextures(1, &m_GBuffer.Albedo);
    glBindTexture(GL_TEXTURE_2D, m_GBuffer.Albedo);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_Width, m_Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, m_GBuffer.Albedo, 0);

    // ARM texture rgb
    glGenTextures(1, &m_GBuffer.ARM);
    glBindTexture(GL_TEXTURE_2D, m_GBuffer.ARM);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_Width, m_Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, m_GBuffer.ARM, 0);

    uint attachments[4] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3 };
    glDrawBuffers(4, attachments);

    // depth
    glGenTextures(1, &m_GBuffer.Depth);
    glBindTexture(GL_TEXTURE_2D, m_GBuffer.Depth);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, m_Width, m_Height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_GBuffer.Depth, 0);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        std::cout << "GBuffer framebuffer not complete!" << std::endl;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
 
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

    m_GBuffer.quad.Init();

    // skybox
    float cubeVertices[] = {-0.5f, -0.5f, -0.5f,  0.0f, 0.0f, 0.5f, -0.5f, -0.5f,  1.0f, 0.0f, 0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 0.5f,  0.5f, -0.5f,  1.0f, 1.0f, -0.5f,  0.5f, -0.5f,  0.0f, 1.0f, -0.5f, -0.5f, -0.5f,  0.0f, 0.0f, -0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 0.5f,  0.5f,  0.5f,  1.0f, 1.0f, -0.5f,  0.5f,  0.5f,  0.0f, 1.0f, -0.5f, -0.5f,  0.5f,  0.0f, 0.0f, -0.5f,  0.5f,  0.5f,  1.0f, 0.0f, -0.5f,  0.5f, -0.5f,  1.0f, 1.0f, -0.5f, -0.5f, -0.5f,  0.0f, 1.0f, -0.5f, -0.5f, -0.5f,  0.0f, 1.0f, -0.5f, -0.5f,  0.5f,  0.0f, 0.0f, -0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 0.5f, -0.5f, -0.5f,  0.0f, 1.0f, 0.5f, -0.5f, -0.5f,  0.0f, 1.0f, 0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 0.5f,  0.5f,  0.5f,  1.0f, 0.0f, -0.5f, -0.5f, -0.5f,  0.0f, 1.0f, 0.5f, -0.5f, -0.5f,  1.0f, 1.0f, 0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 0.5f, -0.5f,  0.5f,  1.0f, 0.0f, -0.5f, -0.5f,  0.5f,  0.0f, 0.0f, -0.5f, -0.5f, -0.5f,  0.0f, 1.0f, -0.5f,  0.5f, -0.5f,  0.0f, 1.0f, 0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 0.5f,  0.5f,  0.5f,  1.0f, 0.0f, -0.5f,  0.5f,  0.5f,  0.0f, 0.0f, -0.5f,  0.5f, -0.5f,  0.0f, 1.0f };
    float skyboxVertices[] = {-1.0f,  1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f,  1.0f, -1.0f, -1.0f,  1.0f, -1.0f, -1.0f, -1.0f,  1.0f, -1.0f, -1.0f, -1.0f, -1.0f,  1.0f, -1.0f, -1.0f,  1.0f, -1.0f, -1.0f,  1.0f,  1.0f, -1.0f, -1.0f,  1.0f, 1.0f, -1.0f, -1.0f, 1.0f, -1.0f,  1.0f, 1.0f,  1.0f,  1.0f, 1.0f,  1.0f,  1.0f, 1.0f,  1.0f, -1.0f, 1.0f, -1.0f, -1.0f, -1.0f, -1.0f,  1.0f, -1.0f,  1.0f,  1.0f, 1.0f,  1.0f,  1.0f, 1.0f,  1.0f,  1.0f, 1.0f, -1.0f,  1.0f, -1.0f, -1.0f,  1.0f, -1.0f,  1.0f, -1.0f, 1.0f,  1.0f, -1.0f, 1.0f,  1.0f,  1.0f, 1.0f,  1.0f,  1.0f, -1.0f,  1.0f,  1.0f, -1.0f,  1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f,  1.0f, 1.0f, -1.0f, -1.0f, 1.0f, -1.0f, -1.0f, -1.0f, -1.0f,  1.0f, 1.0f, -1.0f,  1.0f };

    // cube VAO
    glGenVertexArrays(1, &m_CubeVAO);
    glGenBuffers(1, &m_CubeVBO);
    glBindVertexArray(m_CubeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_CubeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), &cubeVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));

    // skybox VAO
    glGenVertexArrays(1, &m_SkyboxVAO);
    glGenBuffers(1, &m_SkyboxVBO);
    glBindVertexArray(m_SkyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_SkyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    stbi_set_flip_vertically_on_load(true);
    int sb_width, sb_height, sb_channels;
    // float* skyboxData = stbi_loadf(("assets/textures/kloofendal_48d_partly_cloudy.hdr"), &sb_width, &sb_height, &sb_channels, 0);
    float* skyboxData = stbi_loadf(("assets/textures/plains_sunset_4k.hdr"), &sb_width, &sb_height, &sb_channels, 0);
    if (skyboxData)
    {
        glGenTextures(1, &m_SkyboxTexture);
        glBindTexture(GL_TEXTURE_CUBE_MAP, m_SkyboxTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, sb_width, sb_height, 0, GL_RGB, GL_FLOAT, skyboxData);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(skyboxData);
    }
    else
    {
        std::cout << "Failed to load HDR image" << std::endl;
    }

    // convert into cubemap
    // first capture to 2D texture
    int cubemapResolution = 2048;
    glGenFramebuffers(1, &m_CaptureFBO);
    glGenRenderbuffers(1, &m_CaptureRBO);

    glBindFramebuffer(GL_FRAMEBUFFER, m_CaptureFBO);
    glBindRenderbuffer(GL_RENDERBUFFER, m_CaptureRBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, cubemapResolution, cubemapResolution);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_CaptureRBO);

    glGenTextures(1, &m_EnvCubemap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_EnvCubemap);
    for (uint i = 0; i < 6; ++i)
    {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, cubemapResolution, cubemapResolution, 0, GL_RGB, GL_FLOAT, nullptr);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // set up projection and view matrices for capturing data onto the 6 cubemap face directions
    glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
    glm::mat4 captureViews[] =
    {
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f)) 
    };

    m_EquirectangularToCubemapShader->Bind();
    m_EquirectangularToCubemapShader->SetUniform1i("equirectangularMap", 0);
    m_EquirectangularToCubemapShader->SetUniformMat4f("uProj", captureProjection);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_SkyboxTexture);

    glViewport(0, 0, cubemapResolution, cubemapResolution);
    glBindFramebuffer(GL_FRAMEBUFFER, m_CaptureFBO);
    for (uint i = 0; i < 6; ++i)
    {
        m_EquirectangularToCubemapShader->SetUniformMat4f("uView", captureViews[i]);
        glDisable(GL_CULL_FACE); // avoid missing faces due to inverted cube windin)
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, m_EnvCubemap, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glBindVertexArray(m_CubeVAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
    }
    
    int irradianceMapResolution = 32;
    glGenTextures(1, &m_IrradianceMap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_IrradianceMap);
    for (uint i = 0; i < 6; ++i)
    {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, irradianceMapResolution, irradianceMapResolution, 0, GL_RGB, GL_FLOAT, nullptr);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // re scale capture FBO to irradiance map size
    glBindFramebuffer(GL_FRAMEBUFFER, m_CaptureFBO);
    glBindRenderbuffer(GL_RENDERBUFFER, m_CaptureRBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, irradianceMapResolution, irradianceMapResolution);

    m_IrradianceShader->Bind();
    m_IrradianceShader->SetUniform1i("environmentMap", 0);
    m_EquirectangularToCubemapShader->SetUniformMat4f("uProj", captureProjection);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_EnvCubemap);
    glViewport(0, 0, irradianceMapResolution, irradianceMapResolution);
    glBindFramebuffer(GL_FRAMEBUFFER, m_CaptureFBO);
    for (uint i = 0; i < 6; ++i)
    {
        m_IrradianceShader->SetUniformMat4f("uView", captureViews[i]);
        glDisable(GL_CULL_FACE); // avoid missing faces due to inverted cube windin)
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, m_IrradianceMap, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glBindVertexArray(m_CubeVAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
    }

    // prefilter cubemap
    uint maxMipLevels = 5;
    int prefilterMapResolution = 128;
    glGenTextures(1, &m_PrefilterMap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_PrefilterMap);
    for (uint i = 0; i < 6; ++i) {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, prefilterMapResolution, prefilterMapResolution, 0, GL_RGB, GL_FLOAT, nullptr);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAX_LEVEL, maxMipLevels - 1);
    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

    m_PrefilterShader->Bind();
    m_PrefilterShader->SetUniform1i("environmentMap", 0);
    m_PrefilterShader->SetUniformMat4f("uProj", captureProjection);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_EnvCubemap);

    glBindFramebuffer(GL_FRAMEBUFFER, m_CaptureFBO);
    for (uint mip = 0; mip < maxMipLevels; ++mip)
    {
        // reisze framebuffer according to mip-level size.
        uint mipWidth  = static_cast<uint>(prefilterMapResolution * std::pow(0.5, mip));
        uint mipHeight = static_cast<uint>(prefilterMapResolution * std::pow(0.5, mip));
        glBindRenderbuffer(GL_RENDERBUFFER, m_CaptureRBO);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, mipWidth, mipHeight);
        glViewport(0, 0, mipWidth, mipHeight);

        float roughness = (float)mip / (float)(maxMipLevels - 1);
        m_PrefilterShader->SetUniform1f("roughness", roughness);
        for (uint i = 0; i < 6; ++i)
        {
            m_PrefilterShader->SetUniformMat4f("uView", captureViews[i]);
            glDisable(GL_CULL_FACE); // avoid missing faces due to inverted cube windin)
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, m_PrefilterMap, mip);

            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            glBindVertexArray(m_CubeVAO);
            glDrawArrays(GL_TRIANGLES, 0, 36);
            glBindVertexArray(0);
        }
    }

    // generate BRDF LUT texture
    int brdfLUTResolution = 512;
    glGenTextures(1, &m_BRDFLUTTexture);
    glBindTexture(GL_TEXTURE_2D, m_BRDFLUTTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, brdfLUTResolution, brdfLUTResolution, 0, GL_RG, GL_FLOAT, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // reconfigure capture framebuffer object and render screen-space quad with BRDF shader
    glBindFramebuffer(GL_FRAMEBUFFER, m_CaptureFBO);
    glBindRenderbuffer(GL_RENDERBUFFER, m_CaptureRBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, brdfLUTResolution, brdfLUTResolution);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_BRDFLUTTexture, 0);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        std::cout << "BRDF LUT Framebuffer not complete!" << std::endl;
    }
    glViewport(0, 0, brdfLUTResolution, brdfLUTResolution);
    m_BrdfShader->Bind();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    m_GBuffer.quad.Draw();

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    m_LightingShader->Bind();
    m_LightingShader->SetUniform1i("gPosition", 0);
    m_LightingShader->SetUniform1i("gNormal", 1);
    m_LightingShader->SetUniform1i("gAlbedo", 2);
    m_LightingShader->SetUniform1i("gARM", 3);
    m_LightingShader->SetUniform1i("gSSAO", 4);
    
    m_LightingShader->SetUniform1i("irradianceMap", 5);
    m_LightingShader->SetUniform1i("prefilterMap", 6);
    m_LightingShader->SetUniform1i("brdfLUT", 7);


    m_SSAOShader->Bind();
    m_SSAOShader->SetUniform1i("gPosition", 0);
    m_SSAOShader->SetUniform1i("gNormal", 1);
    m_SSAOShader->SetUniform1i("gNoiseTexture", 2);

    m_SSAOBlurShader->Bind();
    m_SSAOBlurShader->SetUniform1i("gSSAOInput", 0);

    m_GBufferShader->Bind();
    m_GBufferShader->SetUniform1i("uAlbedo", 0);
    m_GBufferShader->SetUniform1i("uNormal", 1);
    m_GBufferShader->SetUniform1i("uARM", 2);
}

void Renderer::Shutdown() { }

static void DebugTextureItem(const char* label, uint32_t texID, float width = 128.0f, float height = 72.0f)
{
    ImGui::BeginGroup();
    ImGui::Text("%s", label);
    ImGui::Image((void*)(intptr_t)texID, ImVec2(width, height), ImVec2(0, 1), ImVec2(1, 0));
    
    if (ImGui::IsItemHovered())
    {
        ImGui::BeginTooltip();
        ImGui::Text("ID: %d", texID);
        ImGui::Image((void*)(intptr_t)texID, ImVec2(width * 4, height * 4), ImVec2(0, 1), ImVec2(1, 0));
        ImGui::EndTooltip();
    }
    ImGui::EndGroup();
    
    ImGui::SameLine();
    if (ImGui::GetContentRegionAvail().x < width) ImGui::NewLine();
}

void Renderer::OnImGuiRender()
{
    ImGui::Begin("GPU Texture Debugger");

    if (ImGui::CollapsingHeader("GBuffer Targets", ImGuiTreeNodeFlags_DefaultOpen))
    {
        DebugTextureItem("Position", m_GBuffer.Position);
        DebugTextureItem("Normal", m_GBuffer.Normal);
        DebugTextureItem("Albedo", m_GBuffer.Albedo);
        DebugTextureItem("ARM", m_GBuffer.ARM);
        DebugTextureItem("Depth", m_GBuffer.Depth);
    }
    
    ImGui::NewLine(); 

    if (ImGui::CollapsingHeader("SSAO & Lighting"))
    {
        DebugTextureItem("SSAO Raw", m_SSAOColorBuffer);
        DebugTextureItem("SSAO Blur", m_SSAOBlurBuffer);
        DebugTextureItem("BRDF LUT", m_BRDFLUTTexture, 128,128);
    }

    ImGui::NewLine();

    if (ImGui::CollapsingHeader("Material Cache (Uploaded)"))
    {
        int count = 0;
        for (const auto& [cpuTex, gpuTex] : m_TextureCache)
        {
            uint32_t id = gpuTex->GetID(); 
            std::string label = "Tex " + std::to_string(count++);
            
            DebugTextureItem(label.c_str(), id, 128,128);
        }
        
        if (m_TextureCache.empty())
        {
            ImGui::Text("No materials uploaded to cache yet.");
        }
    }
    
    ImGui::NewLine();

    std::string DrawCmdCount = "Opaque: " + std::to_string(m_OpaqueQueue.size()) + " Transparent: " + std::to_string(m_TransparentQueue.size());
    ImGui::Text("%s",DrawCmdCount.c_str());

    ImGui::End();
}

void Renderer::BeginFrame()
{
	glBindFramebuffer(GL_FRAMEBUFFER, m_GBuffer.FBO);
	glViewport(0, 0, m_Width, m_Height);
	glEnable(GL_DEPTH_TEST);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_OpaqueQueue.clear();
    m_TransparentQueue.clear();
}

void Renderer::EndFrame() { }

void Renderer::DrawScene()
{
    for (Entity* e : m_Scene->Entities)
    {
        DrawEntity(*e, *m_GBufferShader);
    }

    // geometry pass
    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);

    m_GBufferShader->Bind();
    m_GBufferShader->SetUniformMat4f("uView", m_Scene->activeCamera->GetViewMatrix());
    m_GBufferShader->SetUniformMat4f("uProjection", m_Scene->activeCamera->GetProjectionMatrix());

    for (const DrawCmd& cmd : m_OpaqueQueue)
    {
        m_GBufferShader->SetUniformMat4f("uModel", cmd.Model);
        BindMaterial(cmd.Material);
        cmd.Mesh->Bind();
        cmd.Mesh->DrawSubMesh(cmd.SubMeshIndex);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // ssao pass
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

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // lighting pass
    glViewport(0, 0, m_Width, m_Height);
    glDisable(GL_DEPTH_TEST);
    glClearColor(0.53f, 0.8f, 0.92f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    
    m_LightingShader->Bind();

    int dirLightCount = 0;
    int pointLightCount = 0;
    int spotLightCount = 0;

    for (const Light* light : m_Scene->Lights)
    {
        switch (light->GetType())
        {
            case Light::LightType::Directional:
            {
                const DirectionalLight* dLight = static_cast<const DirectionalLight*>(light);
                
                std::string base = "uDirLights[" + std::to_string(dirLightCount) + "]";
                glm::vec4 dir = m_Scene->activeCamera->GetViewMatrix() * glm::vec4(dLight->Direction, 0.0f);

                m_LightingShader->SetUniform3f(base + ".direction", dir.x, dir.y, dir.z);
                m_LightingShader->SetUniform3f(base + ".color",     dLight->Color.x, dLight->Color.y, dLight->Color.z);
                m_LightingShader->SetUniform1f(base + ".intensity", dLight->Intensity);

                dirLightCount++;
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

    m_LightingShader->SetUniform1i("uDirLightCount", dirLightCount);
    m_LightingShader->SetUniform1i("uPointLightCount", pointLightCount);
    m_LightingShader->SetUniform1i("uSpotLightCount", spotLightCount);
    m_LightingShader->SetUniformMat4f("uInverseView", glm::inverse(m_Scene->activeCamera->GetViewMatrix()));

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
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_IrradianceMap);
    glActiveTexture(GL_TEXTURE6);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_PrefilterMap);
    glActiveTexture(GL_TEXTURE7);
    glBindTexture(GL_TEXTURE_2D, m_BRDFLUTTexture);

    m_GBuffer.quad.Draw();

    // forward pass
    glBindFramebuffer(GL_READ_FRAMEBUFFER, m_GBuffer.FBO);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glBlitFramebuffer(0, 0, m_Width, m_Height, 0, 0, m_Width, m_Height, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glEnable(GL_DEPTH_TEST);

    // draw transparent objects here

    glDepthFunc(GL_LEQUAL);
    m_SkyboxShader->Bind();
    m_SkyboxShader->SetUniformMat4f("uView", m_Scene->activeCamera->GetViewMatrix());
    m_SkyboxShader->SetUniformMat4f("uProj", m_Scene->activeCamera->GetProjectionMatrix());
    glActiveTexture(GL_TEXTURE0);
    // glBindTexture(GL_TEXTURE_CUBE_MAP, m_IrradianceMap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_EnvCubemap);
    glBindVertexArray(m_SkyboxVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
    glDepthFunc(GL_LESS);
}

void Renderer::Resize(int nWidth, int nHeight)
{
    glBindTexture(GL_TEXTURE_2D, m_GBuffer.Position);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, nWidth, nHeight, 0, GL_RGB, GL_FLOAT, NULL);

    glBindTexture(GL_TEXTURE_2D, m_GBuffer.Normal);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, nWidth, nHeight, 0, GL_RGBA, GL_FLOAT, NULL);

    glBindTexture(GL_TEXTURE_2D, m_GBuffer.Albedo);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, nWidth, nHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

    glBindTexture(GL_TEXTURE_2D, m_GBuffer.ARM);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, nWidth, nHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

    glBindTexture(GL_TEXTURE_2D, m_GBuffer.Depth);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, nWidth, nHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

    glBindTexture(GL_TEXTURE_2D, m_SSAOColorBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, nWidth, nHeight, 0, GL_RED, GL_FLOAT, NULL);

    glBindTexture(GL_TEXTURE_2D, m_SSAOBlurBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, nWidth, nHeight, 0, GL_RED, GL_FLOAT, NULL);

	m_Width = nWidth;
	m_Height = nHeight;
}

void Renderer::ReloadShaders()
{
    m_GBufferShader->Reload("assets/shaders/gbuffer.vert", "assets/shaders/gbuffer.frag");
	m_LightingShader->Reload("assets/shaders/fullscreen.vert", "assets/shaders/lighting.frag");
    m_SSAOShader->Reload("assets/shaders/fullscreen.vert", "assets/shaders/ssao.frag");
    m_SSAOBlurShader->Reload("assets/shaders/fullscreen.vert", "assets/shaders/ssaoblur.frag");
    m_SkyboxShader->Reload("assets/shaders/skybox.vert", "assets/shaders/skybox.frag");
    m_EquirectangularToCubemapShader->Reload("assets/shaders/cubemap.vert", "assets/shaders/equirectangular_to_cubemap.frag");
    m_IrradianceShader->Reload("assets/shaders/cubemap.vert", "assets/shaders/irradiance_convolution.frag");
    m_PrefilterShader->Reload("assets/shaders/cubemap.vert", "assets/shaders/prefilter.frag");
    m_BrdfShader->Reload("assets/shaders/fullscreen.vert", "assets/shaders/brdf.frag");
}

void Renderer::DrawEntity(const Entity& entity, Shader& shader)
{
    if (!entity.meshAsset) return;

    shader.Bind();
    shader.SetUniformMat4f("uModel", entity.transform);

    if (m_MeshCache.find(entity.meshAsset.get()) == m_MeshCache.end())
    {
        m_MeshCache[entity.meshAsset.get()] = std::make_unique<MeshResource>(*entity.meshAsset);
    }
    
    MeshResource* mesh = m_MeshCache[entity.meshAsset.get()].get();
    mesh->Bind();

    const std::vector<SubMesh>& subMeshes = entity.meshAsset->SubMeshes;
    for (int i = 0; i < subMeshes.size(); ++i)
    {
        const SubMesh& subMesh = subMeshes[i];
        
        if (subMesh.MaterialIndex < entity.materials.size())
        {
            Material* mat = entity.materials[subMesh.MaterialIndex].get();

            DrawCmd item;
            item.Mesh = mesh;
            item.Material = entity.materials[subMesh.MaterialIndex];
            item.Model = entity.transform;
            item.SubMeshIndex = i;
            
            glm::vec4 worldCenter = item.Model * glm::vec4(subMesh.LocalCenter, 1.0f);
            glm::vec4 viewCenter  = m_Scene->activeCamera->GetViewMatrix() * worldCenter;
            item.depth = -viewCenter.z;
            
            if (mat->Translucent) m_TransparentQueue.push_back(item);
            else                  m_OpaqueQueue.push_back(item);
        }
    }
}

RenderTexture* Renderer::GetGPUTexture(const Texture* cpuTexture)
{
    if (m_TextureCache.find(cpuTexture) == m_TextureCache.end())
    {
        m_TextureCache[cpuTexture] = std::make_unique<RenderTexture>(*cpuTexture);
    }

    return m_TextureCache[cpuTexture].get();
}

void Renderer::ClearCache()
{
    m_MeshCache.clear();
    m_TextureCache.clear();
}

void Renderer::BindMaterial(std::shared_ptr<Material> mat)
{
    if (mat->DiffuseTexture) GetGPUTexture(mat->DiffuseTexture.get())->Bind(0);
    if (mat->NormalTexture) GetGPUTexture(mat->NormalTexture.get())->Bind(1);
    if (mat->ARMTexture) GetGPUTexture(mat->ARMTexture.get())->Bind(2);
}