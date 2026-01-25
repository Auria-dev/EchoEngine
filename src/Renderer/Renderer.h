#pragma once

#include <unordered_map>
#include <memory>

#include "Resources/Entity.h"
#include "Core/Scene.h"
#include "MeshResource.h"
#include "RenderTexture.h"
#include "Shader.h"
#include "FullscreenQuad.h"

#include "imgui.h"
#include "stb_image.h"

struct GBuffer
{

	uint Position;
	uint Normal;
	uint Albedo;
    uint ARM;
	uint Depth;
	uint FBO;

	FullscreenQuad quad;
    
};

struct DrawCmd
{
    
    MeshResource* Mesh;
    uint32_t SubMeshIndex;
    glm::mat4 Model;
    std::shared_ptr<Material> Material;
    bool shadowCasting;
    
    float depth;

    bool operator<(const DrawCmd& x) const { return depth < x.depth; }
};

class Renderer
{

public:
	void Init(int width, int height);
    void Shutdown();
    
    void OnImGuiRender();
    
    void BeginFrame();
    void EndFrame();

    void DrawScene();
    void Resize(int nWidth, int nHeight);
    void ReloadShaders();

    void SubmitDrawCmd(const Entity& entity, Shader& shader);
    void ClearCache();

    RenderTexture* GetGPUTexture(const Texture* cpuTexture);

    void SetScene(SceneData& s) { m_Scene = &s; }
    SceneData* GetScene(void) { return m_Scene; }

    std::unordered_map<std::string, std::chrono::nanoseconds> m_PerformanceTimer;
private:
    SceneData* m_Scene;
    
    GBuffer m_GBuffer;

    // TODO: clean this up...
    uint m_SSAOFBO, m_SSAOBlurFBO, m_SSAONoise;
    uint m_SSAOColorBuffer, m_SSAOBlurBuffer;
    uint m_ShadowMapFBO, m_ShadowMap, m_ShadowWidth, m_ShadowHeight;
    // uint m_SkyboxTexture, m_IrradianceMap, m_EnvCubemap, m_SkyboxVAO, m_SkyboxVBO, m_CubeVAO, m_CubeVBO, m_CaptureFBO, m_CaptureRBO, m_PrefilterMap, m_BRDFLUTTexture;
    uint m_Width, m_Height;
    uint m_TransmittanceLUT, m_TransmittanceFBO;
    uint m_MultiScatteringLUT, m_MultiScatteringFBO;
    uint m_PrefilteredMap;
    uint m_PostProcess;
    uint m_LightingFBO, m_LightingResult;
    uint m_SkyProbeMap, m_SkyProbeFBO;
    uint m_SkyCaptureSize = 64;
    Shader* m_ForwardShader;
    Shader* m_GBufferShader;
    Shader* m_LightingShader;
    Shader* m_SSAOShader;
    Shader* m_SSAOBlurShader;
    Shader* m_SkyboxShader;
    Shader* m_EquirectangularToCubemapShader;
    Shader* m_IrradianceShader;
    Shader* m_PrefilterShader;
    Shader* m_BrdfShader;
    Shader* m_AtmosphereShader;
    Shader* m_TransmittanceShader;
    Shader* m_MultiScatteringShader;
    Shader* m_ShadowMapShader;
    Shader* m_PostProcessShader;
    
    float m_LightDistance;
    glm::mat4 m_OrthoProj;
    glm::mat4 m_LightView;
    glm::mat4 m_LightProj;

    uint m_TimeElapsedQuery;

    void GeometryPass();
    void SkyCapture();
    void SSAOPass();
    void ShadowMapPass();
    void LightingPass();
    void AtmospherePass();
    void ForwardPass();

    float m_Exposure;

    std::vector<glm::vec3> m_SSAOKernel;

    std::vector<DrawCmd> m_DeferredQueue;
    std::vector<DrawCmd> m_ForwardQueue;

    std::unordered_map<const Mesh*, std::unique_ptr<MeshResource>> m_MeshCache;
    std::unordered_map<const Texture*, std::unique_ptr<RenderTexture>> m_TextureCache;

    void BindMaterial(std::shared_ptr<Material> mat);
};