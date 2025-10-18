// Icarian Engine - C# Game Engine
// 
// License at end of file.

#include "Rendering/Null/NullRenderEngineBackend.h"

#define GLM_FORCE_SWIZZLE
#include <glm/glm.hpp>

#include "Rendering/CameraBuffer.h"
#include "Runtime/RuntimeManager.h"

#include "EngineAmbientLightInteropStructures.h"
#include "EngineDirectionalLightInteropStructures.h"
#include "EngineMaterialInteropStructures.h"
#include "EnginePointLightInteropStructures.h"
#include "EngineSpotLightInteropStructures.h"

static NullRenderEngineBackend* Instance = nullptr;

#define NULLGRAPHICS_BINDING_FUNCTION_TABLE(F) \
    F(uint32_t, IcarianEngine.Rendering.Shaders, ComputeShader, GenerateGraphicsFromFile, { static uint32_t ComputeAddr = 0; return ComputeAddr++; }, MonoString* a_path) \
    F(void, IcarianEngine.Rendering.Shaders, ComputeShader, AddImport, { }, MonoString* a_key, MonoString* a_value) \
    F(void, IcarianEngine.Rendering.Shaders, ComputeShader, DestroyGraphicsShader, { }, uint32_t a_addr) \
    F(uint32_t, IcarianEngine.Rendering.Shaders, VertexShader, GenerateFromFile, { static uint32_t VertexAddr = 0; return VertexAddr++; }, MonoString* a_path) \
    F(void, IcarianEngine.Rendering.Shaders, VertexShader, AddImport, { }, MonoString* a_key, MonoString* a_value) \
    F(void, IcarianEngine.Rendering.Shaders, VertexShader, DestroyShader, { }, uint32_t a_addr) \
    F(uint32_t, IcarianEngine.Rendering.Shaders, MeshShader, GenerateFromFile, { static uint32_t MeshAddr = 0; return MeshAddr; }, MonoString* a_path) \
    F(void, IcarianEngine.Rendering.Shaders, MeshShader, AddImport, { }, MonoString* a_key, MonoString* a_value) \
    F(void, IcarianEngine.Rendering.Shaders, MeshShader, DestroyShader, { }, uint32_t a_addr) \
    F(uint32_t, IcarianEngine.Rendering.Shaders, PixelShader, GenerateFromFile, { static uint32_t PixelAddr = 0; return PixelAddr++; }, MonoString* a_path) \
    F(void, IcarianEngine.Rendering.Shaders, PixelShader, AddImport, { }, MonoString* a_key, MonoString* a_value) \
    F(void, IcarianEngine.Rendering.Shaders, PixelShader, DestroyShader, { }, uint32_t a_addr) \
    \
    F(uint32_t, IcarianEngine.Rendering, Material, GenerateProgram, { return Instance->CreateMaterialAddr(); }, uint32_t a_vertexShader, uint32_t a_pixelShader, uint16_t a_vertexStride, MonoArray* a_vertexInputAttribs, uint32_t a_cullMode, uint32_t a_primitiveMode, uint32_t a_colorBlendMode, uint32_t a_renderLayer, uint32_t a_shadowVertexShader, uint32_t a_uboSize, void* a_uboData, MonoArray* a_userArray, uint32_t a_arrayStride) \
    F(uint32_t, IcarianEngine.Rendering, Material, GenerateMeshProgram, { return Instance->CreateMaterialAddr(); }, uint32_t a_meshShader, uint32_t a_pixelShader, uint16_t a_vertexStride, uint32_t a_cullMode, uint32_t a_colorBlendMode, uint32_t a_renderLayer, uint32_t a_shadowVertexShader, uint32_t a_uboSize, void* a_uboData, MonoArray* a_userArray, uint32_t a_arrayStride) \
    F(uint32_t, IcarianEngine.Rendering, Material, GenerateComputeProgram, { return Instance->CreateMaterialAddr(); }, uint32_t a_computeShader, uint32_t a_uboSize, void* a_uboData) \
    F(void, IcarianEngine.Rendering, Material, DestroyMaterial, { }, uint32_t a_addr) \
    F(RenderProgram, IcarianEngine.Rendering, Material, GetProgramBuffer, { return RenderProgram(); }, uint32_t a_addr) \
    F(void, IcarianEngine.Rendering, Material, SetProgramBuffer, { }, uint32_t a_addr, RenderProgram a_program) \
    F(void, IcarianEngine.Rendering, Material, SetTexture, { }, uint32_t a_addr, uint32_t a_shaderSlot, uint32_t a_samplerAddr) \
    F(void, IcarianEngine.Rendering, Material, SetUserUniform, { }, uint32_t a_addr, uint32_t a_uboSize, void* a_uboData) \
    \
    F(uint32_t, IcarianEngine.Rendering, Camera, GenerateBuffer, { static uint32_t CameraAddr = 0; return CameraAddr++; }, uint32_t a_transformAddr) \
    F(void, IcarianEngine.Rendering, Camera, DestroyBuffer, { }, uint32_t a_addr) \
    F(CameraBuffer, IcarianEngine.Rendering, Camera, GetBuffer, { return CameraBuffer(); }, uint32_t a_addr) \
    F(void, IcarianEngine.Rendering, Camera, SetBuffer, { }, uint32_t a_addr, CameraBuffer a_buffer) \
    F(MonoArray*, IcarianEngine.Rendering, Camera, GetProjectionMatrix, { return NULL; }, uint32_t a_addr, uint32_t a_width, uint32_t a_height) \
    F(MonoArray*, IcarianEngine.Rendering, Camera, GetProjectionMatrixNF, { return NULL; }, uint32_t a_addr, uint32_t a_width, uint32_t a_height, float a_near, float a_far) \
    F(glm::vec3, IcarianEngine.Rendering, Camera, ScreenToWorld, { return glm::vec3(0.0f); }, uint32_t a_addr, glm::vec3 a_screenPos, glm::vec2 a_screenSize) \
    \
    F(uint32_t, IcarianEngine.Rendering, ModelRenderer, GenerateBuffer, { static uint32_t ModelAddr = 0; return ModelAddr++; }, uint32_t a_transformAddr, uint32_t a_materialAddr, uint32_t a_modelAddr) \
    F(void, IcarianEngine.Rendering, ModelRenderer, DestroyBuffer, { }, uint32_t a_addr) \
    F(void, IcarianEngine.Rendering, ModelRenderer, GenerateRenderStack, { }, uint32_t a_addr) \
    F(void, IcarianEngine.Rendering, ModelRenderer, DestroyRenderStack, { }, uint32_t a_addr) \
    \
    F(uint32_t, IcarianEngine.Rendering.Animation, SkinnedMeshRenderer, GenerateBuffer, {  static uint32_t SkinnedModelAddr = 0; return SkinnedModelAddr++; }, uint32_t a_transformAddr, uint32_t a_materialAddr, uint32_t a_modelAddr, uint32_t a_skeletonAddr) \
    F(void, IcarianEngine.Rendering.Animation, SkinnedMeshRenderer, DestroyBuffer, { }, uint32_t a_addr) \
    F(void, IcarianEngine.Rendering.Animation, SkinnedMeshRenderer, GenerateRenderStack, { }, uint32_t a_addr) \
    F(void, IcarianEngine.Rendering.Animation, SkinnedMeshRenderer, DestroyRenderStack, { }, uint32_t a_addr) \
    \
    F(void, IcarianEngine.Rendering, Texture, DestroyTexture, { }, uint32_t a_addr) \
    \
    F(uint32_t, IcarianEngine.Rendering, TextureSampler, GenerateTextureSampler, { return Instance->CreateTextureSamplerAddr(); }, uint32_t a_texture, uint32_t a_filter, uint32_t a_addressMode) \
    F(uint32_t, IcarianEngine.Rendering, TextureSampler, GenerateRenderTextureSampler, { return Instance->CreateTextureSamplerAddr(); }, uint32_t a_renderTexture, uint32_t a_textureIndex, uint32_t a_filter, uint32_t a_addressMode) \
    F(uint32_t, IcarianEngine.Rendering, TextureSampler, GenerateRenderTextureDepthSampler, { return Instance->CreateTextureSamplerAddr(); }, uint32_t a_renderTexture, uint32_t a_filter, uint32_t a_addressMode) \
    F(uint32_t, IcarianEngine.Rendering, TextureSampler, GenerateRenderTextureDepthSamplerDepth, { return Instance->CreateTextureSamplerAddr(); }, uint32_t a_renderTexture, uint32_t a_filter, uint32_t a_addressMode) \
    F(void, IcarianEngine.Rendering, TextureSampler, DestroySampler, { }, uint32_t a_addr) \
    \
    F(uint32_t, IcarianEngine.Rendering, RenderTextureCmd, GenerateRenderTexture, { return Instance->CreateRenderTextureAddr(); }, uint32_t a_count, uint32_t a_width, uint32_t a_height, uint32_t a_depthTexture, uint32_t a_hdr) \
    F(uint32_t, IcarianEngine.Rendering, RenderTextureCmd, GenerateRenderTextureD, { return Instance->CreateRenderTextureAddr(); }, uint32_t a_count, uint32_t a_width, uint32_t a_height, uint32_t a_depthHandle, uint32_t a_hdr) \
    F(void, IcarianEngine.Rendering, RenderTextureCmd, DestroyRenderTexture, { }, uint32_t a_addr) \
    F(uint32_t, IcarianEngine.Rendering, RenderTextureCmd, HasDepth, { return 1; }, uint32_t a_addr) \
    F(uint32_t, IcarianEngine.Rendering, RenderTextureCmd, GetWidth, { return 2; }, uint32_t a_addr) \
    F(uint32_t, IcarianEngine.Rendering, RenderTextureCmd, GetHeight, { return 2; }, uint32_t a_addr) \
    F(void, IcarianEngine.Rendering, RenderTextureCmd, Resize, { }, uint32_t a_addr, uint32_t a_width, uint32_t a_height) \
    \
    F(uint32_t, IcarianEngine.Rendering, DepthRenderTexture, GenerateRenderTexture, {  static uint32_t RenderTextureAddr = 0; return RenderTextureAddr++; }, uint32_t a_width, uint32_t a_height) \
    F(void, IcarianEngine.Rendering, DepthRenderTexture, DestroyRenderTexture, { }, uint32_t a_addr) \
    F(uint32_t, IcarianEngine.Rendering, DepthRenderTexture, GetWidth, { return 2; }, uint32_t a_addr) \
    F(uint32_t, IcarianEngine.Rendering, DepthRenderTexture, GetHeight, { return 2; }, uint32_t a_addr) \
    F(void, IcarianEngine.Rendering, DepthRenderTexture, Resize, { }, uint32_t a_addr, uint32_t a_width, uint32_t a_height) \
    \
    F(uint32_t, IcarianEngine.Rendering, DepthCubeRenderTexture, GenerateRenderTexture, { static uint32_t RenderTextureAddr = 0; return RenderTextureAddr++; }, uint32_t a_width, uint32_t a_height) \
    F(void, IcarianEngine.Rendering, DepthCubeRenderTexture, DestroyRenderTexture, { }, uint32_t a_addr) \
    F(uint32_t, IcarianEngine.Rendering, DepthCubeRenderTexture, GetWidth, { return 2; }, uint32_t a_addr) \
    F(uint32_t, IcarianEngine.Rendering, DepthCubeRenderTexture, GetHeight, { return 2; }, uint32_t a_addr) \
    F(void, IcarianEngine.Rendering, DepthCubeRenderTexture, Resize, { }, uint32_t a_addr, uint32_t a_width, uint32_t a_height) \
    \
    F(uint32_t, IcarianEngine.Rendering, MultiRenderTexture, GetTextureCount, { return 1; }, uint32_t a_addr) \
    \
    F(uint32_t, IcarianEngine.Rendering, Model, GenerateModel, { return Instance->CreateModelAddr(); }, MonoArray* a_vertices, MonoArray* a_indices, uint16_t a_vertexStride, float a_radius) \
    F(void, IcarianEngine.Rendering, Model, DestroyModel, { }, uint32_t a_addr) \
    \
    F(uint32_t, IcarianEngine.Rendering.Lighting, AmbientLight, GenerateBuffer, { static uint32_t AmbientLightAddr = 0; return AmbientLightAddr++; }, uint32_t a_transformAddr) \
    F(void, IcarianEngine.Rendering.Lighting, AmbientLight, DestroyBuffer, { }, uint32_t a_addr) \
    F(AmbientLightBuffer, IcarianEngine.Rendering.Lighting, AmbientLight, GetBuffer, { return AmbientLightBuffer(); }, uint32_t a_addr) \
    F(void, IcarianEngine.Rendering.Lighting, AmbientLight, SetBuffer, { }, uint32_t a_addr, AmbientLightBuffer a_buffer) \
    \
    F(uint32_t, IcarianEngine.Rendering.Lighting, DirectionalLight, GenerateBuffer, { static uint32_t DirectionalLightAddr = 0; return DirectionalLightAddr++; }, uint32_t a_transformAddr) \
    F(void, IcarianEngine.Rendering.Lighting, DirectionalLight, DestroyBuffer, { }, uint32_t a_addr) \
    F(DirectionalLightBuffer, IcarianEngine.Rendering.Lighting, DirectionalLight, GetBuffer, { return DirectionalLightBuffer(); }, uint32_t a_addr) \
    F(void, IcarianEngine.Rendering.Lighting, DirectionalLight, SetBuffer, { }, uint32_t a_addr, DirectionalLightBuffer a_buffer) \
    F(void, IcarianEngine.Rendering.Lighting, DirectionalLight, AddShadowMap, { }, uint32_t a_addr, uint32_t a_shadowMapAddr) \
    F(void, IcarianEngine.Rendering.Lighting, DirectionalLight, RemoveShadowMap, { }, uint32_t a_addr, uint32_t a_shadowMapAddr) \
    F(MonoArray*, IcarianEngine.Rendering.Lighting, DirectionalLight, GetShadowMaps, { return NULL; }, uint32_t a_addr) \
    \
    F(uint32_t, IcarianEngine.Rendering.Lighting, PointLight, GenerateBuffer, { static uint32_t PointLightAddr = 0; return PointLightAddr++; }, uint32_t a_transformAddr) \
    F(void, IcarianEngine.Rendering.Lighting, PointLight, DestroyBuffer, { }, uint32_t a_addr) \
    F(PointLightBuffer, IcarianEngine.Rendering.Lighting, PointLight, GetBuffer, { return PointLightBuffer(); }, uint32_t a_addr) \
    F(void, IcarianEngine.Rendering.Lighting, PointLight, SetBuffer, { }, uint32_t a_addr, PointLightBuffer a_buffer) \
    F(uint32_t, IcarianEngine.Rendering.Lighting, PointLight, GetShadowMap, { return uint32_t(-1); }, uint32_t a_addr) \
    F(void, IcarianEngine.Rendering.Lighting, PointLight, SetShadowMap, { }, uint32_t a_addr, uint32_t a_shadowMapAddr) \
    \
    F(uint32_t, IcarianEngine.Rendering.Lighting, SpotLight, GenerateBuffer, { static uint32_t SpotLightAddr = 0; return SpotLightAddr++; }, uint32_t a_transformAddr) \
    F(void, IcarianEngine.Rendering.Lighting, SpotLight, DestroyBuffer, { }, uint32_t a_addr) \
    F(SpotLightBuffer, IcarianEngine.Rendering.Lighting, SpotLight, GetBuffer, { return SpotLightBuffer(); }, uint32_t a_addr) \
    F(void, IcarianEngine.Rendering.Lighting, SpotLight, SetBuffer, { }, uint32_t a_addr, SpotLightBuffer a_buffer) \
    F(uint32_t, IcarianEngine.Rendering.Lighting, SpotLight, GetShadowMap, { return uint32_t(-1); }, uint32_t a_addr) \
    F(void, IcarianEngine.Rendering.Lighting, SpotLight, SetShadowMap, { }, uint32_t a_addr, uint32_t a_shadowMapAddr) \
    \
    F(uint32_t, IcarianEngine.Rendering.UI, CanvasRenderer, GenerateBuffer, { static uint32_t CanvasAddr = 0; return CanvasAddr++; }) \
    F(void, IcarianEngine.Rendering.UI, CanvasRenderer, DestroyBuffer, { }, uint32_t a_addr) \
    F(void, IcarianEngine.Rendering.UI, CanvasRenderer, SetCanvas, { }, uint32_t a_addr, uint32_t a_canvasAddr) \
    F(uint32_t, IcarianEngine.Rendering.UI, CanvasRenderer, GetCanvas, { return 0; }, uint32_t a_addr) \
    F(void, IcarianEngine.Rendering.UI, CanvasRenderer, SetRenderTexture, { }, uint32_t a_addr, uint32_t a_renderTextureAddr) \
    F(uint32_t, IcarianEngine.Rendering.UI, CanvasRenderer, GetRenderTexture, { return 0; }, uint32_t a_addr) \
    \
    F(void, IcarianEngine.Rendering, RenderCommand, BindMaterial, { }, uint32_t a_addr) \
    F(void, IcarianEngine.Rendering, RenderCommand, PushTexture, { }, uint32_t a_slot, uint32_t a_samplerAddr) \
    F(void, IcarianEngine.Rendering, RenderCommand, PushLight, { }, uint32_t a_slot, uint32_t a_lightType, uint32_t a_lightAddr) \
    F(void, IcarianEngine.Rendering, RenderCommand, PushShadowSplits, { }, uint32_t alot, MonoArray* a_splits) \
    F(void, IcarianEngine.Rendering, RenderCommand, BindRenderTexture, { }, uint32_t a_addr, uint32_t a_bindMode) \
    F(void, IcarianEngine.Rendering, RenderCommand, RTRTBlit, { }, uint32_t a_srcAddr, uint32_t a_dstAddr) \
    F(void, IcarianEngine.Rendering, RenderCommand, MTRTBlit, { }, uint32_t a_srcAddr, uint32_t a_index, uint32_t a_dstAddr) \
    F(void, IcarianEngine.Rendering, RenderCommand, DrawMaterial, { }) \
    F(void, IcarianEngine.Rendering, RenderCommand, DrawModel, { }, glm::mat4 a_transform, uint32_t a_addr) \
    F(void, IcarianEngine.Rendering, RenderCommand, MarkerStart, {  }, MonoString* a_name) \
    F(void, IcarianEngine.Rendering, RenderCommand, MarkerEnd, { }) \
    \
    F(void, IcarianEngine.Rendering, RenderPipeline, SetLightSplits, { }, MonoArray* a_splits) \
    \
    F(void, IcarianEngine.Rendering.Animation, SkeletonAnimator, PushTransform, { }, uint32_t a_addr, MonoString* a_object, MonoArray* a_transform) \

NULLGRAPHICS_BINDING_FUNCTION_TABLE(RUNTIME_FUNCTION_DEFINITION);

NullRenderEngineBackend::NullRenderEngineBackend(RenderEngine* a_engine) : RenderEngineBackend(a_engine)
{
    m_materialAddr = 0;
    m_meshAddr = 0;
    m_modelAddr = 0;
    m_textureAddr = 0;
    m_textureSamplerAddr = 0;
    m_renderTextureAddr = 0;

    Instance = this;

    NULLGRAPHICS_BINDING_FUNCTION_TABLE(RUNTIME_FUNCTION_ATTACH);
}
NullRenderEngineBackend::~NullRenderEngineBackend()
{

}

uint32_t NullRenderEngineBackend::GenerateMesh
(
    const void* a_vertices,
    uint32_t a_vertexCount,
    uint16_t a_vertexStride,
    const uint32_t* a_meshletVertices,
    uint32_t a_meshletVertexCount,
    const uint8_t* a_meshletTriangles,
    uint32_t a_meshletTriangleCount,
    const IcarianCore::ShaderMeshletBuffer* a_meshlets,
    uint32_t a_meshletCount,
    float a_radius
)
{
    return m_meshAddr++;
}
void NullRenderEngineBackend::DestroyMesh(uint32_t a_addr)
{

}

uint32_t NullRenderEngineBackend::GenerateModel
(
    const void* a_vertices,
    uint32_t a_vertexCount,
    uint16_t a_vertexStride,
    const uint32_t* a_indices,
    uint32_t a_indexCount,
    float a_radius
)
{
    return m_modelAddr++;
}
void NullRenderEngineBackend::DestroyModel(uint32_t a_addr)
{

}

uint32_t NullRenderEngineBackend::GenerateTexture(uint32_t a_width, uint32_t a_height, e_TextureFormat a_format, const void* a_data)
{
    return m_textureAddr++;
}
uint32_t NullRenderEngineBackend::GenerateTextureMipMapped
(
    uint32_t a_width,
    uint32_t a_height,
    uint32_t a_levels,
    uint64_t* a_offsets,
    e_TextureFormat a_format,
    const void* a_data,
    uint64_t a_dataSize
)
{
    return m_textureAddr++;
}
void NullRenderEngineBackend::DestroyTexture(uint32_t a_texture)
{

}

uint32_t NullRenderEngineBackend::GenerateTextureSampler
(
    uint32_t a_textureAddr,
    e_TextureMode a_textureMode,
    e_TextureFilter a_filterMode,
    e_TextureAddress a_addressMode,
    uint32_t a_slot
)
{
    return m_textureSamplerAddr++;
}
void NullRenderEngineBackend::DestroyTextureSampler(uint32_t a_sampler)
{

}

void NullRenderEngineBackend::Update(double a_delta, double a_time)
{

}

// MIT License
// 
// Copyright (c) 2025 River Govers
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.