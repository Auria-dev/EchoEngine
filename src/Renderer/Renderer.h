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

    void DrawEntity(const Entity& entity, Shader& shader);
    void ClearCache();

    RenderTexture* GetGPUTexture(const Texture* cpuTexture);

    void SetScene(SceneData& s) { m_Scene = &s; }
    SceneData* GetScene(void) { return m_Scene; }

private:
    SceneData* m_Scene;
    
    GBuffer m_GBuffer;
    uint m_SSAOFBO, m_SSAOBlurFBO, m_SSAONoise;
    uint m_SSAOColorBuffer, m_SSAOBlurBuffer;
    uint m_SkyboxTexture, m_IrradianceMap, m_EnvCubemap, m_SkyboxVAO, m_SkyboxVBO, m_CubeVAO, m_CubeVBO, m_CaptureFBO, m_CaptureRBO, m_PrefilterMap, m_BRDFLUTTexture;
    uint m_Width, m_Height;
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

    std::vector<glm::vec3> m_SSAOKernel;

    std::vector<DrawCmd> m_DeferredQueue;
    std::vector<DrawCmd> m_ForwardQueue;

    std::unordered_map<const Mesh*, std::unique_ptr<MeshResource>> m_MeshCache;
    std::unordered_map<const Texture*, std::unique_ptr<RenderTexture>> m_TextureCache;

    void BindMaterial(std::shared_ptr<Material> mat);
};