#include "Rendering/Null/NullRenderEngineBackend.h"

#define GLM_FORCE_SWIZZLE
#include <glm/glm.hpp>

#include "Rendering/CameraBuffer.h"
#include "Rendering/Light.h"
#include "Rendering/UI/Font.h"
#include "Runtime/RuntimeManager.h"

#include "EngineMaterialInteropStructures.h"

#define NULLGRAPHICS_BINDING_FUNCTION_TABLE(F) \
    F(uint32_t, IcarianEngine.Rendering, VertexShader, GenerateFromFile, { return 0; }, MonoString* a_path) \
    F(void, IcarianEngine.Rendering, VertexShader, DestroyShader, { }, uint32_t a_addr) \
    F(uint32_t, IcarianEngine.Rendering, PixelShader, GenerateFromFile, { return 0; }, MonoString* a_path) \
    F(void, IcarianEngine.Rendering, PixelShader, DestroyShader, { }, uint32_t a_addr) \
    \
    F(uint32_t, IcarianEngine.Rendering, Material, GenerateMaterial, { return 0; }, uint32_t a_vertexShaderAddr, uint32_t a_pixelShaderAddr, uint16_t a_vertexStride, MonoArray* a_vertexInputAttribs, MonoArray* a_shaderInputs, uint32_t a_cullingMode, uint32_t a_primitiveMode, uint32_t a_colorBlendingEnabled) \
    F(void, IcarianEngine.Rendering, Material, DestroyMaterial, { }, uint32_t a_addr) \
    F(RenderProgram, IcarianEngine.Rendering, Material, GetProgramBuffer, { return RenderProgram(); }, uint32_t a_addr) \
    F(void, IcarianEngine.Rendering, Material, SetProgramBuffer, { }, uint32_t a_addr, RenderProgram a_program) \
    F(void, IcarianEngine.Rendering, Material, SetTexture, { }, uint32_t a_addr, uint32_t a_shaderSlot, uint32_t a_samplerAddr) \
    \
    F(uint32_t, IcarianEngine.Rendering, Camera, GenerateBuffer, { return 0; }, uint32_t a_transformAddr) \
    F(void, IcarianEngine.Rendering, Camera, DestroyBuffer, { }, uint32_t a_addr) \
    F(CameraBuffer, IcarianEngine.Rendering, Camera, GetBuffer, { return CameraBuffer(); }, uint32_t a_addr) \
    F(void, IcarianEngine.Rendering, Camera, SetBuffer, { }, uint32_t a_addr, CameraBuffer a_buffer) \
    F(MonoArray*, IcarianEngine.Rendering, Camera, GetProjectionMatrix, { return NULL; }, uint32_t a_addr, uint32_t a_width, uint32_t a_height) \
    F(MonoArray*, IcarianEngine.Rendering, Camera, GetProjectionMatrixNF, { return NULL; }, uint32_t a_addr, uint32_t a_width, uint32_t a_height, float a_near, float a_far) \
    F(glm::vec3, IcarianEngine.Rendering, Camera, ScreenToWorld, { return glm::vec3(0.0f); }, uint32_t a_addr, glm::vec3 a_screenPos, glm::vec2 a_screenSize) \
    \
    F(uint32_t, IcarianEngine.Rendering, MeshRenderer, GenerateBuffer, { return 0; }, uint32_t a_transformAddr, uint32_t a_materialAddr, uint32_t a_modelAddr) \
    F(void, IcarianEngine.Rendering, MeshRenderer, DestroyBuffer, { }, uint32_t a_addr) \
    F(void, IcarianEngine.Rendering, MeshRenderer, GenerateRenderStack, { }, uint32_t a_addr) \
    F(void, IcarianEngine.Rendering, MeshRenderer, DestroyRenderStack, { }, uint32_t a_addr) \
    \
    F(uint32_t, IcarianEngine.Rendering.Animation, SkinnedMeshRenderer, GenerateBuffer, { return 0; }, uint32_t a_transformAddr, uint32_t a_materialAddr, uint32_t a_modelAddr, uint32_t a_skeletonAddr) \
    F(void, IcarianEngine.Rendering.Animation, SkinnedMeshRenderer, DestroyBuffer, { }, uint32_t a_addr) \
    F(void, IcarianEngine.Rendering.Animation, SkinnedMeshRenderer, GenerateRenderStack, { }, uint32_t a_addr) \
    F(void, IcarianEngine.Rendering.Animation, SkinnedMeshRenderer, DestroyRenderStack, { }, uint32_t a_addr) \
    \
    F(uint32_t, IcarianEngine.Rendering, Texture, GenerateFromFile, { return 0; }, MonoString* a_path) \
    F(void, IcarianEngine.Rendering, Texture, DestroyTexture, { }, uint32_t a_addr) \
    \
    F(uint32_t, IcarianEngine.Rendering, TextureSampler, GenerateTextureSampler, { return 0; }, uint32_t a_texture, uint32_t a_filter, uint32_t a_addressMode) \
    F(uint32_t, IcarianEngine.Rendering, TextureSampler, GenerateRenderTextureSampler, { return 0; }, uint32_t a_renderTexture, uint32_t a_textureIndex, uint32_t a_filter, uint32_t a_addressMode) \
    F(uint32_t, IcarianEngine.Rendering, TextureSampler, GenerateRenderTextureDepthSampler, { return 0; }, uint32_t a_renderTexture, uint32_t a_filter, uint32_t a_addressMode) \
    F(uint32_t, IcarianEngine.Rendering, TextureSampler, GenerateRenderTextureDepthSamplerDepth, { return 0; }, uint32_t a_renderTexture, uint32_t a_filter, uint32_t a_addressMode) \
    F(void, IcarianEngine.Rendering, TextureSampler, DestroySampler, { }, uint32_t a_addr) \
    \
    F(uint32_t, IcarianEngine.Rendering, RenderTextureCmd, GenerateRenderTexture, { return 0; }, uint32_t a_count, uint32_t a_width, uint32_t a_height, uint32_t a_depthTexture, uint32_t a_hdr) \
    F(void, IcarianEngine.Rendering, RenderTextureCmd, DestroyRenderTexture, { }, uint32_t a_addr) \
    F(uint32_t, IcarianEngine.Rendering, RenderTextureCmd, HasDepth, { return 0; }, uint32_t a_addr) \
    F(uint32_t, IcarianEngine.Rendering, RenderTextureCmd, GetWidth, { return 0; }, uint32_t a_addr) \
    F(uint32_t, IcarianEngine.Rendering, RenderTextureCmd, GetHeight, { return 0; }, uint32_t a_addr) \
    F(void, IcarianEngine.Rendering, RenderTextureCmd, Resize, { }, uint32_t a_addr, uint32_t a_width, uint32_t a_height) \
    \
    F(uint32_t, IcarianEngine.Rendering, DepthRenderTexture, GenerateRenderTexture, {  return 0; }, uint32_t a_width, uint32_t a_height) \
    F(void, IcarianEngine.Rendering, DepthRenderTexture, DestroyRenderTexture, { }, uint32_t a_addr) \
    \
    F(uint32_t, IcarianEngine.Rendering, MultiRenderTexture, GetTextureCount, { return 0; }, uint32_t a_addr) \
    \
    F(uint32_t, IcarianEngine.Rendering, Model, GenerateModel, { return 0; }, MonoArray* a_vertices, MonoArray* a_indices, uint16_t a_vertexStride, float a_radius) \
    F(uint32_t, IcarianEngine.Rendering, Model, GenerateFromFile, { return 0; }, MonoString* a_path) \
    F(uint32_t, IcarianEngine.Rendering, Model, GenerateSkinnedFromFile, { return 0; }, MonoString* a_path) \
    F(void, IcarianEngine.Rendering, Model, DestroyModel, { }, uint32_t a_addr) \
    \
    F(uint32_t, IcarianEngine.Rendering.Lighting, DirectionalLight, GenerateBuffer, { return 0; }, uint32_t a_transformAddr) \
    F(void, IcarianEngine.Rendering.Lighting, DirectionalLight, DestroyBuffer, { }, uint32_t a_addr) \
    F(DirectionalLightBuffer, IcarianEngine.Rendering.Lighting, DirectionalLight, GetBuffer, { return DirectionalLightBuffer(); }, uint32_t a_addr) \
    F(void, IcarianEngine.Rendering.Lighting, DirectionalLight, SetBuffer, { }, uint32_t a_addr, DirectionalLightBuffer a_buffer) \
    F(void, IcarianEngine.Rendering.Lighting, DirectionalLight, AddShadowMap, { }, uint32_t a_addr, uint32_t a_shadowMapAddr) \
    F(void, IcarianEngine.Rendering.Lighting, DirectionalLight, RemoveShadowMap, { }, uint32_t a_addr, uint32_t a_shadowMapAddr) \
    F(MonoArray*, IcarianEngine.Rendering.Lighting, DirectionalLight, GetShadowMaps, { return NULL; }, uint32_t a_addr) \
    \
    F(uint32_t, IcarianEngine.Rendering.Lighting, PointLight, GenerateBuffer, { return 0; }, uint32_t a_transformAddr) \
    F(void, IcarianEngine.Rendering.Lighting, PointLight, DestroyBuffer, { }, uint32_t a_addr) \
    F(PointLightBuffer, IcarianEngine.Rendering.Lighting, PointLight, GetBuffer, { return PointLightBuffer(); }, uint32_t a_addr) \
    F(void, IcarianEngine.Rendering.Lighting, PointLight, SetBuffer, { }, uint32_t a_addr, PointLightBuffer a_buffer) \
    \
    F(uint32_t, IcarianEngine.Rendering.Lighting, SpotLight, GenerateBuffer, { return 0; }, uint32_t a_transformAddr) \
    F(void, IcarianEngine.Rendering.Lighting, SpotLight, DestroyBuffer, { }, uint32_t a_addr) \
    F(SpotLightBuffer, IcarianEngine.Rendering.Lighting, SpotLight, GetBuffer, { return SpotLightBuffer(); }, uint32_t a_addr) \
    F(void, IcarianEngine.Rendering.Lighting, SpotLight, SetBuffer, { }, uint32_t a_addr, SpotLightBuffer a_buffer) \
    \
    F(uint32_t, IcarianEngine.Rendering.UI, Font, GenerateFont, { return 0; }, MonoString* a_path) \
    F(void, IcarianEngine.Rendering.UI, Font, DestroyFont, { }, uint32_t a_addr) \
    \
    F(uint32_t, IcarianEngine.Rendering.UI, CanvasRenderer, GenerateBuffer, { return 0; }) \
    F(void, IcarianEngine.Rendering.UI, CanvasRenderer, DestroyBuffer, { }, uint32_t a_addr) \
    F(void, IcarianEngine.Rendering.UI, CanvasRenderer, SetCanvas, { }, uint32_t a_addr, uint32_t a_canvasAddr) \
    F(uint32_t, IcarianEngine.Rendering.UI, CanvasRenderer, GetCanvas, { return 0; }, uint32_t a_addr) \
    F(void, IcarianEngine.Rendering.UI, CanvasRenderer, SetRenderTexture, { }, uint32_t a_addr, uint32_t a_renderTextureAddr) \
    F(uint32_t, IcarianEngine.Rendering.UI, CanvasRenderer, GetRenderTexture, { return 0; }, uint32_t a_addr) \
    \
    F(void, IcarianEngine.Rendering, RenderCommand, BindMaterial, { }, uint32_t a_addr) \
    F(void, IcarianEngine.Rendering, RenderCommand, PushTexture, { }, uint32_t a_slot, uint32_t a_samplerAddr) \
    F(void, IcarianEngine.Rendering, RenderCommand, BindRenderTexture, { }, uint32_t a_addr) \
    F(void, IcarianEngine.Rendering, RenderCommand, RTRTBlit, { }, uint32_t a_srcAddr, uint32_t a_dstAddr) \
    F(void, IcarianEngine.Rendering, RenderCommand, DrawMaterial, { }) \
    F(void, IcarianEngine.Rendering, RenderCommand, DrawModel, { }, MonoArray* a_transform, uint32_t a_addr) \
    \
    F(void, IcarianEngine.Rendering, RenderPipeline, SetLightLVP, { }, MonoArray* a_lvp) \
    F(void, IcarianEngine.Rendering, RenderPipeline, SetLightSplits, { }, MonoArray* a_splits) \
    \
    F(void, IcarianEngine.Rendering.Animation, SkeletonAnimator, PushTransform, { }, uint32_t a_addr, MonoString* a_object, MonoArray* a_transform) \

NULLGRAPHICS_BINDING_FUNCTION_TABLE(RUNTIME_FUNCTION_DEFINITION)

NullRenderEngineBackend::NullRenderEngineBackend(RenderEngine* a_engine) : RenderEngineBackend(a_engine)
{
    NULLGRAPHICS_BINDING_FUNCTION_TABLE(RUNTIME_FUNCTION_ATTACH);
}
NullRenderEngineBackend::~NullRenderEngineBackend()
{

}

uint32_t NullRenderEngineBackend::GenerateAlphaTexture(uint32_t a_width, uint32_t a_height, const void* a_data)
{
    return 0;
}
void NullRenderEngineBackend::DestroyTexture(uint32_t a_texture)
{

}

uint32_t NullRenderEngineBackend::GenerateTextureSampler(uint32_t a_textureAddr, e_TextureMode a_textureMode, e_TextureFilter a_filterMode, e_TextureAddress a_addressMode, uint32_t a_slot)
{
    return 0;
}
void NullRenderEngineBackend::DestroyTextureSampler(uint32_t a_sampler)
{

}

Font* NullRenderEngineBackend::GetFont(uint32_t a_addr)
{
    return nullptr;
}

void NullRenderEngineBackend::Update(double a_delta, double a_time)
{

}