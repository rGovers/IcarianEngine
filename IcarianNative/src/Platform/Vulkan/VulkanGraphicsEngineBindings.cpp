// Icarian Engine - C# Game Engine
// 
// License at end of file.

#ifdef ICARIANNATIVE_ENABLE_GRAPHICS_VULKAN

#include "Rendering/Vulkan/VulkanGraphicsEngineBindings.h"

#include <meshoptimizer.h>

#include "Core/IcarianDefer.h"
#include "Core/IcarianError.h"
#include "Core/IcarianLambda.h"
#include "Core/StringUtils.h"
#include "DeletionQueue.h"
#include "FileCache.h"
#include "ObjectManager.h"
#include "Rendering/RenderAssetStore.h"
#include "Rendering/RenderEngine.h"
#include "Rendering/ShaderTable.h"
#include "Rendering/Vulkan/VulkanDepthCubeRenderTexture.h"
#include "Rendering/Vulkan/VulkanDepthRenderTexture.h"
#include "Rendering/Vulkan/VulkanGraphicsEngine.h"
#include "Rendering/Vulkan/VulkanGraphicsParticle2D.h"
#include "Rendering/Vulkan/VulkanLightBuffer.h"
#include "Rendering/Vulkan/VulkanLightData.h"
#include "Rendering/Vulkan/VulkanMesh.h"
#include "Rendering/Vulkan/VulkanModel.h"
#include "Rendering/Vulkan/VulkanRenderCommand.h"
#include "Rendering/Vulkan/VulkanRenderEngineBackend.h"
#include "Rendering/Vulkan/VulkanRenderTexture.h"
#include "Rendering/Vulkan/VulkanShaderData.h"
#include "Rendering/Vulkan/VulkanTextureSampler.h"
#include "Rendering/Vulkan/VulkanVideoTexture.h"
#include "Runtime/RuntimeFunction.h"
#include "Runtime/RuntimeManager.h"
#include "Trace.h"

#ifdef WIN32
#include "Core/WindowsHeaders.h"
#endif

static VulkanGraphicsEngineBindings* Instance = nullptr;

// The lazy part of me won against the part that wants to write clean code
// My apologies to the poor soul that has to decipher this definition
#define VULKANGRAPHICS_BINDING_FUNCTION_TABLE(F) \
    F(void, IcarianEngine.Rendering.Shaders, ComputeShader, DestroyGraphicsShader, { IPUSHDELETIONFUNC(Instance->DestroyComputeshader(a_addr), DeletionIndex_Render); }, uint32_t a_addr) \
    F(void, IcarianEngine.Rendering.Shaders, VertexShader, DestroyShader, { IPUSHDELETIONFUNC(Instance->DestroyVertexShader(a_addr), DeletionIndex_Render); }, uint32_t a_addr) \
    F(void, IcarianEngine.Rendering.Shaders, MeshShader, DestroyShader, { IPUSHDELETIONFUNC(Instance->DestroyMeshShader(a_addr), DeletionIndex_Render); }, uint32_t a_addr) \
    F(void, IcarianEngine.Rendering.Shaders, PixelShader, DestroyShader, { IPUSHDELETIONFUNC(Instance->DestroyPixelShader(a_addr), DeletionIndex_Render); }, uint32_t a_addr) \
    \
    F(RenderProgram, IcarianEngine.Rendering, Material, GetProgramBuffer, { return Instance->GetRenderProgram(a_addr); }, uint32_t a_addr) \
    F(void, IcarianEngine.Rendering, Material, SetProgramBuffer, { Instance->SetRenderProgram(a_addr, a_program); }, uint32_t a_addr, RenderProgram a_program) \
    F(void, IcarianEngine.Rendering, Material, SetTexture, { Instance->RenderProgramSetTexture(a_addr, a_shaderSlot, a_samplerAddr); }, uint32_t a_addr, uint32_t a_shaderSlot, uint32_t a_samplerAddr) \
    \
    F(uint32_t, IcarianEngine.Rendering, Camera, GenerateBuffer, { return Instance->GenerateCameraBuffer(a_transformAddr); }, uint32_t a_transformAddr) \
    F(void, IcarianEngine.Rendering, Camera, DestroyBuffer, { Instance->DestroyCameraBuffer(a_addr); }, uint32_t a_addr) \
    F(CameraBuffer, IcarianEngine.Rendering, Camera, GetBuffer, { return Instance->GetCameraBuffer(a_addr); }, uint32_t a_addr) \
    F(void, IcarianEngine.Rendering, Camera, SetBuffer, { Instance->SetCameraBuffer(a_addr, a_buffer); }, uint32_t a_addr, CameraBuffer a_buffer) \
    F(glm::vec3, IcarianEngine.Rendering, Camera, ScreenToWorld, { return Instance->CameraScreenToWorld(a_addr, a_screenPos, a_screenSize); }, uint32_t a_addr, glm::vec3 a_screenPos, glm::vec2 a_screenSize) \
    \
    F(uint32_t, IcarianEngine.Rendering, ModelRenderer, GenerateBuffer, { return Instance->GenerateModelRenderBuffer(a_materialAddr, a_modelAddr, a_transformAddr); }, uint32_t a_transformAddr, uint32_t a_materialAddr, uint32_t a_modelAddr) \
    F(void, IcarianEngine.Rendering, ModelRenderer, DestroyBuffer, { IPUSHDELETIONFUNC(Instance->DestroyModelRenderBuffer(a_addr), DeletionIndex_Render); }, uint32_t a_addr) \
    F(void, IcarianEngine.Rendering, ModelRenderer, GenerateRenderStack, { IPUSHDELETIONFUNC(Instance->GenerateModelRenderStack(a_addr), DeletionIndex_Render); }, uint32_t a_addr) \
    F(void, IcarianEngine.Rendering, ModelRenderer, DestroyRenderStack, { IPUSHDELETIONFUNC(Instance->DestroyModelRenderStack(a_addr), DeletionIndex_Render); }, uint32_t a_addr) \
    \
    F(uint32_t, IcarianEngine.Rendering.Animation, SkinnedModelRenderer, GenerateBuffer, { return Instance->GenerateSkinnedModelRenderBuffer(a_materialAddr, a_modelAddr, a_transformAddr, a_skeletonAddr); }, uint32_t a_transformAddr, uint32_t a_materialAddr, uint32_t a_modelAddr, uint32_t a_skeletonAddr) \
    F(void, IcarianEngine.Rendering.Animation, SkinnedModelRenderer, DestroyBuffer, { IPUSHDELETIONFUNC(Instance->DestroySkinnedModelRenderBuffer(a_addr), DeletionIndex_Render); }, uint32_t a_addr) \
    F(void, IcarianEngine.Rendering.Animation, SkinnedModelRenderer, GenerateRenderStack, { IPUSHDELETIONFUNC(Instance->GenerateSkinnedModelRenderStack(a_addr), DeletionIndex_Render); }, uint32_t a_addr) \
    F(void, IcarianEngine.Rendering.Animation, SkinnedModelRenderer, DestroyRenderStack, { IPUSHDELETIONFUNC(Instance->DestroySkinnedModelRenderStack(a_addr), DeletionIndex_Render); }, uint32_t a_addr) \
    \
    F(uint32_t, IcarianEngine.Rendering, MeshRenderer, GenerateBuffer, { return Instance->GenerateMeshRenderBuffer(a_materialAddr, a_meshAddr, a_transformAddr, a_indexCount); }, uint32_t a_transformAddr, uint32_t a_materialAddr, uint32_t a_meshAddr, uint32_t a_indexCount) \
    F(void, IcarianEngine.Rendering, MeshRenderer, DestroyBuffer, { IPUSHDELETIONFUNC(Instance->DestroyMeshRenderBuffer(a_addr), DeletionIndex_Render); }, uint32_t a_addr) \
    F(void, IcarianEngine.Rendering, MeshRenderer, GenerateRenderStack, { IPUSHDELETIONFUNC(Instance->GenerateMeshRenderStack(a_addr), DeletionIndex_Render); }, uint32_t a_addr) \
    F(void, IcarianEngine.Rendering, MeshRenderer, DestroyRenderStack, { IPUSHDELETIONFUNC(Instance->DestroyMeshRenderStack(a_addr), DeletionIndex_Render); }, uint32_t a_addr) \
    \
    F(void, IcarianEngine.Rendering, Texture, DestroyTexture, { IPUSHDELETIONFUNC(Instance->DestroyTexture(a_addr), DeletionIndex_Render); }, uint32_t a_addr) \
    \
    F(uint32_t, IcarianEngine.Rendering.Video, VideoTexture, GenerateTexture, { return Instance->GenerateVideoTexture(a_addr); }, uint32_t a_addr) \
    F(void, IcarianEngine.Rendering.Video, VideoTexture, DestroyTexture, { IPUSHDELETIONFUNC({ Instance->DestroyVideoTexture(a_addr); }, DeletionIndex_Render); }, uint32_t a_addr) \
    \
    F(uint32_t, IcarianEngine.Rendering, TextureSampler, GenerateTextureSampler, { return Instance->GenerateTextureSampler(a_texture, (e_TextureFilter)a_filter, (e_TextureAddress)a_addressMode ); }, uint32_t a_texture, uint32_t a_filter, uint32_t a_addressMode) \
    F(uint32_t, IcarianEngine.Rendering, TextureSampler, GenerateRenderTextureSampler, { return Instance->GenerateRenderTextureSampler(a_renderTexture, a_textureIndex, (e_TextureFilter)a_filter, (e_TextureAddress)a_addressMode); }, uint32_t a_renderTexture, uint32_t a_textureIndex, uint32_t a_filter, uint32_t a_addressMode) \
    F(uint32_t, IcarianEngine.Rendering, TextureSampler, GenerateRenderTextureDepthSampler, { return Instance->GenerateRenderTextureDepthSampler(a_renderTexture, (e_TextureFilter)a_filter, (e_TextureAddress)a_addressMode); }, uint32_t a_renderTexture, uint32_t a_filter, uint32_t a_addressMode) \
    F(uint32_t, IcarianEngine.Rendering, TextureSampler, GenerateRenderTextureDepthSamplerDepth, { return Instance->GenerateRenderTextureDepthSamplerDepth(a_renderTexture, (e_TextureFilter)a_filter, (e_TextureAddress)a_addressMode); }, uint32_t a_renderTexture, uint32_t a_filter, uint32_t a_addressMode) \
    F(void, IcarianEngine.Rendering, TextureSampler, DestroySampler, { Instance->DestroyTextureSampler(a_addr); }, uint32_t a_addr) \
    \
    F(uint32_t, IcarianEngine.Rendering, RenderTextureCmd, GenerateRenderTexture, { return Instance->GenerateRenderTexture(a_count, a_width, a_height, (bool)a_depthTexture, (bool)a_hdr, a_channelCount); }, uint32_t a_count, uint32_t a_width, uint32_t a_height, uint32_t a_depthTexture, uint32_t a_hdr, uint32_t a_channelCount) \
    F(uint32_t, IcarianEngine.Rendering, RenderTextureCmd, GenerateRenderTextureD, { return Instance->GenerateRenderTextureD(a_count, a_width, a_height, a_depthHandle, (bool)a_hdr, a_channelCount); }, uint32_t a_count, uint32_t a_width, uint32_t a_height, uint32_t a_depthHandle, uint32_t a_hdr, uint32_t a_channelCount) \
    F(void, IcarianEngine.Rendering, RenderTextureCmd, DestroyRenderTexture, { return Instance->DestroyRenderTexture(a_addr); }, uint32_t a_addr) \
    F(uint32_t, IcarianEngine.Rendering, RenderTextureCmd, HasDepth, { return (uint32_t)Instance->RenderTextureHasDepth(a_addr); }, uint32_t a_addr) \
    F(uint32_t, IcarianEngine.Rendering, RenderTextureCmd, GetWidth, { return Instance->GetRenderTextureWidth(a_addr); }, uint32_t a_addr) \
    F(uint32_t, IcarianEngine.Rendering, RenderTextureCmd, GetHeight, { return Instance->GetRenderTextureHeight(a_addr); }, uint32_t a_addr) \
    F(void, IcarianEngine.Rendering, RenderTextureCmd, Resize, { return Instance->ResizeRenderTexture(a_addr, a_width, a_height); }, uint32_t a_addr, uint32_t a_width, uint32_t a_height) \
    \
    F(uint32_t, IcarianEngine.Rendering, DepthRenderTexture, GenerateRenderTexture, {  return Instance->GenerateDepthRenderTexture(a_width, a_height); }, uint32_t a_width, uint32_t a_height) \
    F(void, IcarianEngine.Rendering, DepthRenderTexture, DestroyRenderTexture, { Instance->DestroyDepthRenderTexture(a_addr); }, uint32_t a_addr) \
    F(uint32_t, IcarianEngine.Rendering, DepthRenderTexture, GetWidth, { return Instance->GetDepthRenderTextureWidth(a_addr); }, uint32_t a_addr) \
    F(uint32_t, IcarianEngine.Rendering, DepthRenderTexture, GetHeight, { return Instance->GetDepthRenderTextureHeight(a_addr); }, uint32_t a_addr) \
    F(void, IcarianEngine.Rendering, DepthRenderTexture, Resize, { return Instance->ResizeDepthRenderTexture(a_addr, a_width, a_height); }, uint32_t a_addr, uint32_t a_width, uint32_t a_height) \
    \
    F(uint32_t, IcarianEngine.Rendering, DepthCubeRenderTexture, GenerateRenderTexture, { return Instance->GenerateDepthCubeRenderTexture(a_width, a_height); }, uint32_t a_width, uint32_t a_height) \
    F(void, IcarianEngine.Rendering, DepthCubeRenderTexture, DestroyRenderTexture, { Instance->DestroyDepthCubeRenderTexture(a_addr); }, uint32_t a_addr) \
    F(uint32_t, IcarianEngine.Rendering, DepthCubeRenderTexture, GetWidth, { return Instance->GetDepthCubeRenderTextureWidth(a_addr); }, uint32_t a_addr) \
    F(uint32_t, IcarianEngine.Rendering, DepthCubeRenderTexture, GetHeight, { return Instance->GetDepthCubeRenderTextureHeight(a_addr); }, uint32_t a_addr) \
    F(void, IcarianEngine.Rendering, DepthCubeRenderTexture, Resize, { return Instance->ResizeDepthCubeRenderTexture(a_addr, a_width, a_height); }, uint32_t a_addr, uint32_t a_width, uint32_t a_height) \
    \
    F(uint32_t, IcarianEngine.Renddering, MultiRenderTexture, GetTextureCount, { return Instance->GetRenderTextureTextureCount(a_addr); }, uint32_t a_addr) \
    \
    F(void, IcarianEngine.Rendering, Mesh, DestroyMesh, { IPUSHDELETIONFUNC(Instance->DestroyMesh(a_addr), DeletionIndex_Render); }, uint32_t a_addr) \
    \
    F(void, IcarianEngine.Rendering, Model, DestroyModel, { IPUSHDELETIONFUNC(Instance->DestroyModel(a_addr), DeletionIndex_Render); }, uint32_t a_addr) \
    \
    F(uint32_t, IcarianEngine.Rendering, ParticleSystem2D, GenerateGraphicsParticleSystem, { return Instance->GenerateGraphicsParticle2D(a_computeBuffer); }, uint32_t a_computeBuffer) \
    F(void, IcarianEngine.Rendering, ParticleSystem2D, DestroyGraphicsParticleSystem, { IPUSHDELETIONFUNC(Instance->DestroyGraphicsParticle2D(a_bufferAddr), DeletionIndex_Render); }, uint32_t a_bufferAddr) \
    \
    F(uint32_t, IcarianEngine.Rendering.Lighting, AmbientLight, GenerateBuffer, { return Instance->GenerateAmbientLightBuffer(); }) \
    F(void, IcarianEngine.Rendering.Lighting, AmbientLight, DestroyBuffer, { Instance->DestroyAmbientLightBuffer(a_addr); }, uint32_t a_addr) \
    F(AmbientLightBuffer, IcarianEngine.Rendering.Lighting, AmbientLight, GetBuffer, { return Instance->GetAmbientLightBuffer(a_addr); }, uint32_t a_addr) \
    F(void, IcarianEngine.Rendering.Lighting, AmbientLight, SetBuffer, { Instance->SetAmbientLightBuffer(a_addr, a_buffer); }, uint32_t a_addr, AmbientLightBuffer a_buffer) \
    \
    F(uint32_t, IcarianEngine.Rendering.Lighting, DirectionalLight, GenerateBuffer, { return Instance->GenerateDirectionalLightBuffer(a_transformAddr); }, uint32_t a_transformAddr) \
    F(void, IcarianEngine.Rendering.Lighting, DirectionalLight, DestroyBuffer, { IPUSHDELETIONFUNC(Instance->DestroyDirectionalLightBuffer(a_addr), DeletionIndex_Render); }, uint32_t a_addr) \
    F(DirectionalLightBuffer, IcarianEngine.Rendering.Lighting, DirectionalLight, GetBuffer, { return Instance->GetDirectionalLightBuffer(a_addr); }, uint32_t a_addr) \
    F(void, IcarianEngine.Rendering.Lighting, DirectionalLight, SetBuffer, { Instance->SetDirectionalLightBuffer(a_addr, a_buffer); }, uint32_t a_addr, DirectionalLightBuffer a_buffer) \
    F(void, IcarianEngine.Rendering.Lighting, DirectionalLight, AddShadowMap, { IPUSHDELETIONFUNC(Instance->AddDirectionalLightShadowMap(a_addr, a_shadowMapAddr), DeletionIndex_Render); }, uint32_t a_addr, uint32_t a_shadowMapAddr) \
    F(void, IcarianEngine.Rendering.Lighting, DirectionalLight, RemoveShadowMap, { IPUSHDELETIONFUNC(Instance->RemoveDirectionalLightShadowMap(a_addr, a_shadowMapAddr), DeletionIndex_Render); }, uint32_t a_addr, uint32_t a_shadowMapAddr) \
    \
    F(uint32_t, IcarianEngine.Rendering.Lighting, PointLight, GenerateBuffer, { return Instance->GeneratePointLightBuffer(a_transformAddr); }, uint32_t a_transformAddr) \
    F(void, IcarianEngine.Rendering.Lighting, PointLight, DestroyBuffer, { IPUSHDELETIONFUNC(Instance->DestroyPointLightBuffer(a_addr), DeletionIndex_Render); }, uint32_t a_addr) \
    F(PointLightBuffer, IcarianEngine.Rendering.Lighting, PointLight, GetBuffer, { return Instance->GetPointLightBuffer(a_addr); }, uint32_t a_addr) \
    F(void, IcarianEngine.Rendering.Lighting, PointLight, SetBuffer, { Instance->SetPointLightBuffer(a_addr, a_buffer); }, uint32_t a_addr, PointLightBuffer a_buffer) \
    F(uint32_t, IcarianEngine.Rendering.Lighting, PointLight, GetShadowMap, { return Instance->GetPointLightShadowMap(a_addr); }, uint32_t a_addr) \
    F(void, IcarianEngine.Rendering.Lighting, PointLight, SetShadowMap, { Instance->SetPointLightShadowMap(a_addr, a_shadowMapAddr); }, uint32_t a_addr, uint32_t a_shadowMapAddr) \
    \
    F(uint32_t, IcarianEngine.Rendering.Lighting, SpotLight, GenerateBuffer, { return Instance->GenerateSpotLightBuffer(a_transformAddr); }, uint32_t a_transformAddr) \
    F(void, IcarianEngine.Rendering.Lighting, SpotLight, DestroyBuffer, { IPUSHDELETIONFUNC(Instance->DestroySpotLightBuffer(a_addr), DeletionIndex_Render); }, uint32_t a_addr) \
    F(SpotLightBuffer, IcarianEngine.Rendering.Lighting, SpotLight, GetBuffer, { return Instance->GetSpotLightBuffer(a_addr); }, uint32_t a_addr) \
    F(void, IcarianEngine.Rendering.Lighting, SpotLight, SetBuffer, { Instance->SetSpotLightBuffer(a_addr, a_buffer); }, uint32_t a_addr, SpotLightBuffer a_buffer) \
    F(void, IcarianEngine.Rendering.Lighting, SpotLight, SetShadowMap, { Instance->SetSpotLightShadowMap(a_addr, a_shadowMapAddr); }, uint32_t a_addr, uint32_t a_shadowMapAddr) \
    F(uint32_t, IcarianEngine.Rendering.Lighting, SpotLight, GetShadowMap, { return Instance->GetSpotLightShadowMap(a_addr); }, uint32_t a_addr) \
    \
    F(uint32_t, IcarianEngine.Rendering.UI, CanvasRenderer, GenerateBuffer, { return Instance->GenerateCanvasRenderer(); }) \
    F(void, IcarianEngine.Rendering.UI, CanvasRenderer, DestroyBuffer, { IPUSHDELETIONFUNC(Instance->DestroyCanvasRenderer(a_addr), DeletionIndex_Render); }, uint32_t a_addr) \
    F(void, IcarianEngine.Rendering.UI, CanvasRenderer, SetCanvas, { Instance->SetCanvasRendererCanvas(a_addr, a_canvasAddr); }, uint32_t a_addr, uint32_t a_canvasAddr) \
    F(uint32_t, IcarianEngine.Rendering.UI, CanvasRenderer, GetCanvas, { return Instance->GetCanvasRendererCanvas(a_addr); }, uint32_t a_addr) \
    F(void, IcarianEngine.Rendering.UI, CanvasRenderer, SetRenderTexture, { Instance->SetCanvasRendererRenderTexture(a_addr, a_renderTextureAddr); }, uint32_t a_addr, uint32_t a_renderTextureAddr) \
    F(uint32_t, IcarianEngine.Rendering.UI, CanvasRenderer, GetRenderTexture, { return Instance->GetCanvasRendererRenderTexture(a_addr); }, uint32_t a_addr) \
    \
    F(void, IcarianEngine.Rendering, RenderCommand, BindMaterial, { Instance->BindMaterial(a_addr); }, uint32_t a_addr) \
    F(void, IcarianEngine.Rendering, RenderCommand, PushTexture, { Instance->PushTexture(a_slot, a_samplerAddr); }, uint32_t a_slot, uint32_t a_samplerAddr) \
    F(void, IcarianEngine.Rendering, RenderCommand, PushLight, { Instance->PushLight(a_slot, (e_LightType)a_lightType, a_lightAddr); }, uint32_t a_slot, uint32_t a_lightType, uint32_t a_lightAddr) \
    F(void, IcarianEngine.Rendering, RenderCommand, PushShadowTextureArray, { Instance->PushShadowTextureArray(a_slot, a_lightAddr); }, uint32_t a_slot, uint32_t a_lightAddr) \
    F(void, IcarianEngine.Rendering, RenderCommand, BindRenderTexture, { Instance->BindRenderTexture(a_addr, (e_RenderTextureBindMode)a_bindMode); }, uint32_t a_addr, uint32_t a_bindMode) \
    F(void, IcarianEngine.Rendering, RenderCommand, RTRTBlit, { Instance->BlitRTRT(a_srcAddr, a_dstAddr); }, uint32_t a_srcAddr, uint32_t a_dstAddr) \
    F(void, IcarianEngine.Rendering, RenderCommand, MTRTBlit, { Instance->BlitMTRT(a_srcAddr, a_index, a_dstAddr); }, uint32_t a_srcAddr, uint32_t a_index, uint32_t a_dstAddr) \
    F(void, IcarianEngine.Rendering, RenderCommand, DrawMaterial, { Instance->DrawMaterial(); }) \
    F(void, IcarianEngine.Rendering, RenderCommand, DrawModel, { Instance->DrawModel(a_transform, a_addr); }, glm::mat4 a_transform, uint32_t a_addr) \
    F(void, IcarianEngine.Rendering, RenderCommand, MarkerStart, { char* str = mono_string_to_utf8(a_name); IDEFER(mono_free(str)); Instance->MarkerStart(str); }, MonoString* a_name) \
    F(void, IcarianEngine.Rendering, RenderCommand, MarkerEnd, { Instance->MarkerEnd(); }) \
    \
    F(void, IcarianEngine.Rendering.Animation, SkeletonAnimator, PushTransform, { }, uint32_t a_addr, MonoString* a_object, MonoArray* a_transform) \

VULKANGRAPHICS_BINDING_FUNCTION_TABLE(RUNTIME_FUNCTION_DEFINITION);

RUNTIME_FUNCTION(uint32_t, ComputeShader, GenerateGraphicsFromFile,
{
    IERRBLOCK;
    RENDERSCRATCHFRAME;

    char* str = mono_string_to_utf8(a_path);
    IDEFER(mono_free(str));

    const std::filesystem::path p = std::filesystem::path(str);
    const std::filesystem::path ext = p.extension();

    const std::string extStr = ext.string();

    switch (StringHash<uint32_t>(extStr.c_str())) 
    {
    case StringHash<uint32_t>(".fcomp"):
    {
        FileHandle* handle = FileCache::LoadFile(str);
        IERRCHECKRET(handle != nullptr, -1);
        IDEFER(delete handle);

        const uint64_t size = handle->GetSize();

        char* str = RenderScratchAlloc::TAllocate<char>(size);
        IERRCHECKRET(handle->Read(str, size) == size, -1);
        
        return Instance->GenerateFComputeShaderAddr(std::string_view(str, size));
    }
    default:
    {
        IWARN(std::string("ComputeShader invalid file format: ") + str);

        break;
    }
    }

    return -1;
}, MonoString* a_path)
RUNTIME_FUNCTION(void, ComputeShader, AddImport, 
{
    char* key = mono_string_to_utf8(a_key);
    IDEFER(mono_free(key));
    char* value = mono_string_to_utf8(a_value);
    IDEFER(mono_free(value));

    Instance->AddComputeShaderImport(key, value);
}, MonoString* a_key, MonoString* a_value)

RUNTIME_FUNCTION(uint32_t, VertexShader, GenerateFromFile, 
{
    IERRBLOCK;
    RENDERSCRATCHFRAME;

    char* str = mono_string_to_utf8(a_path);
    IDEFER(mono_free(str));

    // Faster to do the comparison if it is a single comparison
    // TODO: Should probably switch file IO to use actual protocol specifiers rather then a string comparison works for now but so low priority
    // That applies to all shader types as internal:// makes more sense then [INTERNAL]
    // I may do away with the shader table and move it to the filesystem and build a resource table for the filesystem
    // Not a fan as shader loading is handling the protocol part of the URI/URL and not the FileCache
    // Yes this means probably gonna be more work on the build system side to generate the table as I want to use hashes still
    // Hash fast string compare slow when multiple need to be done back to back
    if (strncmp(str, INTERNALSHADERPATHSTR, InternalShaderStringSize) == 0)
    {
        const char* shader = GetVertexShaderString(str + InternalShaderStringSize);
        IERRCHECKRET(shader != nullptr, -1);

        return Instance->GenerateFVertexShaderAddr(shader);
    }
    else
    {
        const std::filesystem::path p = std::filesystem::path(str);
        const std::filesystem::path ext = p.extension();

        const std::string extStr = ext.string();

        // Slower as just one comparison but can be expanded and consistant with pixel shader
        switch (StringHash<uint32_t>(extStr.c_str())) 
        {
        case StringHash<uint32_t>(".fvert"):
        {
            FileHandle* handle = FileCache::LoadFile(str);
            IERRCHECKRET(handle != nullptr, -1);
            IDEFER(delete handle);

            const uint64_t size = handle->GetSize();

            char* str = RenderScratchAlloc::TAllocate<char>(size);
            IERRCHECKRET(handle->Read(str, size) == size, -1);

            return Instance->GenerateFVertexShaderAddr(std::string_view(str, size));
        }
        default:
        {
            IWARN(std::string("VertexShader invalid file format: ") + str);

            break;
        }
        }
    }

    return -1;
}, MonoString* a_path)
RUNTIME_FUNCTION(void, VertexShader, AddImport,
{
    char* key = mono_string_to_utf8(a_key);
    IDEFER(mono_free(key));
    char* value = mono_string_to_utf8(a_value);
    IDEFER(mono_free(value));

    Instance->AddVertexShaderImport(key, value);
}, MonoString* a_key, MonoString* a_value)

RUNTIME_FUNCTION(uint32_t, MeshShader, GenerateFromFile, 
{
    IERRBLOCK;
    RENDERSCRATCHFRAME;

    char* str = mono_string_to_utf8(a_path);
    IDEFER(mono_free(str));

    const std::filesystem::path p = std::filesystem::path(str);
    const std::filesystem::path ext = p.extension();

    const std::string extStr = ext.string();

    // Slower as just one comparison but can be expanded and consistant with pixel shader
    switch (StringHash<uint32_t>(extStr.c_str())) 
    {
    case StringHash<uint32_t>(".fmesh"):
    {
        FileHandle* handle = FileCache::LoadFile(str);
        IERRCHECKRET(handle != nullptr, -1);
        IDEFER(delete handle);

        const uint64_t size = handle->GetSize();

        char* str = RenderScratchAlloc::TAllocate<char>(size);
        IERRCHECKRET(handle->Read(str, size) == size, -1);

        return Instance->GenerateFMeshShaderAddr(std::string_view(str, size));
    }
    default:
    {
        IWARN(std::string("MeshShader invalid file format: ") + str);

        break;
    }
    }

    return -1;
}, MonoString* a_path)
RUNTIME_FUNCTION(void, MeshShader, AddImport,
{
    char* key = mono_string_to_utf8(a_key);
    IDEFER(mono_free(key));
    char* value = mono_string_to_utf8(a_value);
    IDEFER(mono_free(value));

    Instance->AddMeshShaderImport(key, value);
}, MonoString* a_key, MonoString* a_value)

RUNTIME_FUNCTION(uint32_t, PixelShader, GenerateFromFile, 
{
    IERRBLOCK;
    RENDERSCRATCHFRAME;

    char* str = mono_string_to_utf8(a_path);
    IDEFER(mono_free(str));

    // Faster to do the comparison if it is a single comparison
    if (strncmp(str, INTERNALSHADERPATHSTR, InternalShaderStringSize) == 0)
    {
        const char* shader = GetPixelShaderString(str + InternalShaderStringSize);
        IERRCHECKRET(shader != nullptr, -1);

        return Instance->GenerateFPixelShaderAddr(shader);
    }
    else
    {
        const std::filesystem::path p = std::filesystem::path(str);
        const std::filesystem::path ext = p.extension();

        const std::string extStr = ext.string();

        switch (StringHash<uint32_t>(extStr.c_str())) 
        {
        case StringHash<uint32_t>(".fpix"):
        case StringHash<uint32_t>(".ffrag"):
        {
            FileHandle* handle = FileCache::LoadFile(str);
            IERRCHECKRET(handle != nullptr, -1);
            IDEFER(delete handle);

            const uint64_t size = handle->GetSize();

            char* str = RenderScratchAlloc::TAllocate<char>(size);
            IERRCHECKRET(handle->Read(str, size) == size, -1);

            return Instance->GenerateFPixelShaderAddr(std::string_view(str, size));
        }
        default:
        {
            IWARN(std::string("PixelShader invalid file format: ") + str);

            break;
        }
        }
    }

    return -1;
}, MonoString* a_path)
RUNTIME_FUNCTION(void, PixelShader, AddImport,
{
    char* key = mono_string_to_utf8(a_key);
    IDEFER(mono_free(key));
    char* value = mono_string_to_utf8(a_value);
    IDEFER(mono_free(value));

    Instance->AddPixelShaderImport(key, value);
}, MonoString* a_key, MonoString* a_value)

RUNTIME_FUNCTION(MonoArray*, Camera, GetProjectionMatrix, 
{
    const glm::mat4 proj = Instance->GetCameraProjectionMatrix(a_addr, a_width, a_height);

    MonoArray* arr = mono_array_new(mono_domain_get(), mono_get_single_class(), 16);

    const float* f = (float*)&proj;
    for (int i = 0; i < 16; ++i)
    {
        mono_array_set(arr, float, i, f[i]);
    }

    return arr;
}, uint32_t a_addr, uint32_t a_width, uint32_t a_height)
RUNTIME_FUNCTION(MonoArray*, Camera, GetProjectionMatrixNF, 
{
    const glm::mat4 proj = Instance->GetCameraProjectionMatrix(a_addr, a_width, a_height, a_near, a_far);

    MonoArray* arr = mono_array_new(mono_domain_get(), mono_get_single_class(), 16);

    const float* f = (float*)&proj;
    for (int i = 0; i < 16; ++i)
    {
        mono_array_set(arr, float, i, f[i]);
    }

    return arr;
}, uint32_t a_addr, uint32_t a_width, uint32_t a_height, float a_near, float a_far)

RUNTIME_FUNCTION(MonoArray*, DirectionalLight, GetShadowMaps, 
{
    const DirectionalLightBuffer buffer = Instance->GetDirectionalLightBuffer(a_addr);

    const VulkanLightBuffer* lightBuffer = (VulkanLightBuffer*)buffer.Data;
    
    MonoArray* arr = mono_array_new(mono_domain_get(), mono_get_uint32_class(), lightBuffer->LightRenderTextureCount);

    for (uint32_t i = 0; i < lightBuffer->LightRenderTextureCount; ++i)
    {
        mono_array_set(arr, uint32_t, i, lightBuffer->LightRenderTextures[i]);
    }

    return arr;
}, uint32_t a_addr)

RUNTIME_FUNCTION(uint32_t, Material, GenerateProgram, 
{
    // List initialisers are being drunk so guess zero and init it is
    RenderProgram program;
    memset(&program, 0, sizeof(RenderProgram));
    program.VertexShader = a_vertexShader;
    program.PixelShader = a_pixelShader;
    program.ShadowVertexShader = a_shadowVertexShader;
    program.VertexStride = a_vertexStride;
    program.CullingMode = (e_CullMode)a_cullMode;
    program.PrimitiveMode = (e_PrimitiveMode)a_primitiveMode;
    program.ColorBlendMode = (e_MaterialBlendMode)a_colorBlendMode;
    program.MaterialMode = MaterialMode_BaseVertex;
    program.RenderLayer = a_renderLayer;

    if (a_vertexInputAttribs != NULL)
    {
        program.VertexInputCount = (uint16_t)mono_array_length(a_vertexInputAttribs);
        program.VertexAttributes = new VertexInputAttribute[program.VertexInputCount];

        for (uint32_t i = 0; i < program.VertexInputCount; ++i)
        {
            program.VertexAttributes[i] = mono_array_get(a_vertexInputAttribs, VertexInputAttribute, i);
        }
    }

    if (a_uboData != NULL)
    {
        program.UBODataSize = a_uboSize;
        program.UBOData = malloc((size_t)program.UBODataSize);

        memcpy(program.UBOData, a_uboData, program.UBODataSize);
    }

    if (a_arrayStride > 0 && a_userArray != NULL)
    {
        program.UserArrayStride = a_arrayStride;
        program.UserArrayCount = (uint32_t)mono_array_length(a_userArray);
        const size_t size = (size_t)program.UserArrayStride * program.UserArrayCount;
        program.UserArrayData = malloc(size);

        for (uint32_t i = 0; i < program.UserArrayCount; ++i)
        {
            const uint8_t* data = (uint8_t*)mono_array_addr_with_size(a_userArray, program.UserArrayStride, i);

            memcpy((char*)program.UserArrayData + i * program.UserArrayStride, data, program.UserArrayStride);
        }
    }

    return Instance->GenerateShaderProgram(program);
}, uint32_t a_vertexShader, uint32_t a_pixelShader, uint16_t a_vertexStride, MonoArray* a_vertexInputAttribs, uint32_t a_cullMode, uint32_t a_primitiveMode, uint32_t a_colorBlendMode, uint32_t a_renderLayer, uint32_t a_shadowVertexShader, uint32_t a_uboSize, void* a_uboData, MonoArray* a_userArray, uint32_t a_arrayStride)
RUNTIME_FUNCTION(uint32_t, Material, GenerateMeshProgram, 
{
    // List initialisers are being drunk so guess zero and init it is
    RenderProgram program;
    memset(&program, 0, sizeof(RenderProgram));
    program.VertexShader = a_meshShader;
    program.PixelShader = a_pixelShader;
    program.VertexStride = a_vertexStride;
    program.CullingMode = (e_CullMode)a_cullMode;
    program.ColorBlendMode = (e_MaterialBlendMode)a_colorBlendMode;
    program.MaterialMode = MaterialMode_BaseMesh;
    program.RenderLayer = a_renderLayer;
    program.ExtraShader = -1;
    program.ShadowVertexShader = -1;

    if (a_uboData != NULL)
    {
        program.UBODataSize = a_uboSize;
        program.UBOData = malloc((size_t)program.UBODataSize);

        memcpy(program.UBOData, a_uboData, program.UBODataSize);
    }

    if (a_arrayStride > 0 && a_userArray != NULL)
    {
        program.UserArrayStride = a_arrayStride;
        program.UserArrayCount = (uint32_t)mono_array_length(a_userArray);
        const size_t size = (size_t)program.UserArrayStride * program.UserArrayCount;
        program.UserArrayData = malloc(size);

        for (uint32_t i = 0; i < program.UserArrayCount; ++i)
        {
            const uint8_t* data = (uint8_t*)mono_array_addr_with_size(a_userArray, program.UserArrayStride, i);

            memcpy((char*)program.UserArrayData + i * program.UserArrayStride, data, program.UserArrayStride);
        }
    }

    return Instance->GenerateShaderProgram(program);
}, uint32_t a_meshShader, uint32_t a_pixelShader, uint16_t a_vertexStride, uint32_t a_cullMode, uint32_t a_colorBlendMode, uint32_t a_renderLayer, uint32_t a_uboSize, void* a_uboData, MonoArray* a_userArray, uint32_t a_arrayStride)
RUNTIME_FUNCTION(uint32_t, Material, GenerateComputeProgram, 
{
    RenderProgram program;
    memset(&program, 0, sizeof(RenderProgram));
    program.ExtraShader = a_computeShader;
    program.MaterialMode = MaterialMode_Compute;

    if (a_uboData != NULL)
    {
        program.UBODataSize = a_uboSize;
        program.UBOData = malloc((size_t)program.UBODataSize);

        memcpy(program.UBOData, a_uboData, (size_t)program.UBODataSize);
    }

    return Instance->GenerateShaderProgram(program);
}, uint32_t a_computeShader, uint32_t a_uboSize, void* a_uboData)
RUNTIME_FUNCTION(void, Material, SetUserUniform,
{
    // Was staring at the code for too long and realised there was a memory safety issue when run async in the previous implementation
    // No idea how it did not cause issues
    // Should be resolved now but that is why I have a transfer function and is no longer in the big function table
    class VulkanMaterialUserBufferTransfer : public DeletionObject
    {
    private:
        void*    m_data;
        uint32_t m_size;
        uint32_t m_addr;

    protected:

    public:
        VulkanMaterialUserBufferTransfer(uint32_t a_addr, uint32_t a_size, void* a_data)
        {
            m_addr = a_addr;
            m_data = a_data;
            m_size = a_size;
        }
        virtual ~VulkanMaterialUserBufferTransfer()
        {
            if (m_data != NULL)
            {
                free(m_data);
            }
        }

        virtual void Destroy()
        {
            Instance->RenderProgramSetUserUBO(m_addr, m_size, m_data);

            m_data = NULL;
        }
    };

    if (a_uboSize > 0 && a_uboBuffer != NULL)
    {
        void* data = malloc(a_uboSize);

        memcpy(data, a_uboBuffer, a_uboSize);

        DeletionQueue::Push(new VulkanMaterialUserBufferTransfer(a_addr, a_uboSize, data), DeletionIndex_Render);

        return;
    }

    IPUSHDELETIONFUNC(Instance->RenderProgramSetUserUBO(a_addr, a_uboSize, a_uboBuffer), DeletionIndex_Render);
}, uint32_t a_addr, uint32_t a_uboSize, void* a_uboBuffer)
RUNTIME_FUNCTION(void, Material, SetUserArray,
{
    // Not normally an issue but code would be fragile and dependant on shutdown order as there would be a period that the pointer would not have an owner
    // Ergo we write a custom deletion object over using the macro
    // Current implementation it does not matter and would always be freed but do not want it fragile
    // And yes I still believe use of smart pointers are from poor architecture
    class VulkanMaterialUserArrayTransfer : public DeletionObject
    {
    private:
        void*    m_data;
        uint32_t m_count;
        uint32_t m_stride;
        uint32_t m_addr;

    protected:

    public:
        VulkanMaterialUserArrayTransfer(uint32_t a_addr, void* a_data, uint32_t a_count, uint32_t a_stride)
        {
            m_addr = a_addr;
            m_data = a_data;
            m_count = a_count;
            m_stride = a_stride;
        }
        virtual ~VulkanMaterialUserArrayTransfer()
        {
            if (m_data != NULL)
            {
                // This should not be called but cannot ensure that in future hence this exists
                free(m_data);
            }
        }

        virtual void Destroy()
        {
            Instance->RenderProgramSetUserArray(m_addr, m_data, m_count, m_stride);

            m_data = NULL;
        }
    };

    if (a_elementStride > 0 && a_array != NULL)
    {
        const uint32_t count = (uint32_t)mono_array_length(a_array);
        const size_t size = (size_t)a_elementStride * count;

        void* arrayData = malloc(size);

        for (uint32_t i = 0; i < count; ++i)
        {
            const uint8_t* data = (uint8_t*)mono_array_addr_with_size(a_array, a_elementStride, i);

            memcpy((char*)arrayData + i * a_elementStride, data, a_elementStride);
        }

        DeletionQueue::Push(new VulkanMaterialUserArrayTransfer(a_addr, arrayData, count, a_elementStride), DeletionIndex_Render);

        return;
    }

    IPUSHDELETIONFUNC(Instance->RenderProgramSetUserArray(a_addr, NULL, 0, a_elementStride), DeletionIndex_Render);
}, uint32_t a_addr, uint32_t a_elementStride, MonoArray* a_array)
RUNTIME_FUNCTION(void, Material, SetUserArrayCallback,
{
    // Not normally an issue but code would be fragile and dependant on shutdown order as there would be a period that the pointer would not have an owner
    // Ergo we write a custom deletion object over using the macro
    // Current implementation it does not matter and would always be freed but do not want it fragile
    // And yes I still believe use of smart pointers are from poor architecture
    class VulkanMaterialUserArrayTransferCallback : public DeletionObject
    {
    private:
        void*                     m_data;
        uint32_t                  m_count;
        uint32_t                  m_stride;
        uint32_t                  m_addr;
        uint32_t                  m_callbackAddr;

    protected:

    public:
        VulkanMaterialUserArrayTransferCallback(uint32_t a_addr, void* a_data, uint32_t a_count, uint32_t a_stride, uint32_t a_callbackAddr)
        {
            m_addr = a_addr;
            m_data = a_data;
            m_count = a_count;
            m_stride = a_stride;
            m_callbackAddr = a_callbackAddr;
        }
        virtual ~VulkanMaterialUserArrayTransferCallback()
        {
            if (m_data != NULL)
            {
                // This should not be called but cannot ensure that in future hence this exists
                free(m_data);
            }
        }

        virtual void Destroy()
        {
            Instance->RenderProgramSetUserArray(m_addr, m_data, m_count, m_stride);

            m_data = NULL;

            RuntimeFunction* func = Instance->GetUserArrayCallback();

            void* args[] =
            {
                &m_callbackAddr
            };

            func->Exec(args);
        }
    };

    if (a_elementStride > 0 && a_array != NULL)
    {
        const uint32_t count = (uint32_t)mono_array_length(a_array);
        const size_t size = (size_t)a_elementStride * count;

        void* arrayData = malloc(size);

        for (uint32_t i = 0; i < count; ++i)
        {
            const uint8_t* data = (uint8_t*)mono_array_addr_with_size(a_array, a_elementStride, i);

            memcpy((char*)arrayData + i * a_elementStride, data, a_elementStride);
        }

        DeletionQueue::Push(new VulkanMaterialUserArrayTransferCallback(a_addr, arrayData, count, a_elementStride, a_callbackAddr), DeletionIndex_Render);

        return;
    }

    DeletionQueue::Push(new VulkanMaterialUserArrayTransferCallback(a_addr, NULL, 0, a_elementStride, a_callbackAddr), DeletionIndex_Render);
}, uint32_t a_addr, uint32_t a_elementStride, MonoArray* a_array, uint32_t a_callbackAddr)

RUNTIME_FUNCTION(void, Material, DestroyProgram, 
{
    IPUSHDELETIONFUNC(
    {
        const RenderProgram program = Instance->GetRenderProgram(a_addr);

        IDEFER(
        {
            if (program.VertexAttributes != nullptr)
            {
                delete[] program.VertexAttributes;
            }

            if (program.UBOData != NULL)
            {
                free(program.UBOData);
            }

            if (program.UserArrayData != NULL)
            {
                free(program.UserArrayData);
            }
        });

        Instance->DestroyShaderProgram(a_addr);
    }, DeletionIndex_Render);
}, uint32_t a_addr)

RUNTIME_FUNCTION(uint32_t, Model, GenerateModel,
{
    const uint32_t vertexCount = (uint32_t)mono_array_length(a_vertices);
    const uint32_t indexCount = (uint32_t)mono_array_length(a_indices);

    const uint32_t vertexSize = vertexCount * a_vertexStride;

    uint8_t* vertices = new uint8_t[vertexSize];
    IDEFER(delete[] vertices);
    for (uint32_t i = 0; i < vertexSize; ++i)
    {
        vertices[i] = *mono_array_addr_with_size(a_vertices, 1, i);
    }

    uint32_t* indices = new uint32_t[indexCount];
    IDEFER(delete[] indices);
    for (uint32_t i = 0; i < indexCount; ++i)
    {
        indices[i] = mono_array_get(a_indices, uint32_t, i);
    }

    return Instance->GenerateModel(vertices, vertexCount, indices, indexCount, a_vertexStride, a_radius);
}, MonoArray* a_vertices, MonoArray* a_indices, uint16_t a_vertexStride, float a_radius);
RUNTIME_FUNCTION(uint32_t, Mesh, GenerateFromModel,
{
    // meshopt uses unsigned int so have to verify
    // Will be fine on 99% of platforms but can cause issues
    // That is why this exists
    // If this ever trips we need to port meshopt to use cstdint
    // Luckily things have not been able to change sizes as there is now applications that rely upon int being 32 bit
    // For now this is just code smell to do eventually and not critical
    IVERIFY(sizeof(uint32_t) == sizeof(unsigned int));
    IVERIFY(sizeof(uint8_t) == sizeof(unsigned char));

    const uint32_t vertexCount = (uint32_t)mono_array_length(a_vertices);
    const uint32_t indexCount = (uint32_t)mono_array_length(a_indices);

    const uint32_t vertexSize = vertexCount * a_vertexStride;

    uint8_t* vertices = new uint8_t[vertexSize];
    IDEFER(delete[] vertices);
    for (uint32_t i = 0; i < vertexSize; ++i)
    {
        vertices[i] = *mono_array_addr_with_size(a_vertices, 1, i);
    }

    uint32_t* indices = new uint32_t[indexCount];
    IDEFER(delete[] indices);
    for (uint32_t i = 0; i < indexCount; ++i)
    {
        indices[i] = mono_array_get(a_indices, uint32_t, i);
    }

    // I could probably make an optimize toggle
    // May not always want to optimize
    meshopt_optimizeVertexCache
    (
        indices,
        indices,
        indexCount,
        vertexCount
    );
    // TODO: This needs to change as it is making assumptions about the Vertex layout
    meshopt_optimizeOverdraw
    (
        indices,
        indices,
        indexCount,
        (float*)vertices,
        vertexCount,
        a_vertexStride,
        1.05f
    );
    const size_t newVertexCount = meshopt_optimizeVertexFetch
    (
        vertices,
        indices,
        indexCount,
        vertices,
        vertexCount,
        a_vertexStride
    );

    constexpr uint32_t MeshletTriangleCount = 124;
    constexpr uint32_t MeshletVertexCount = 64;

    const size_t maxMeshletCount = meshopt_buildMeshletsBound(indexCount, MeshletVertexCount, MeshletTriangleCount);

    meshopt_Meshlet* meshoptMeshlets = new meshopt_Meshlet[maxMeshletCount];
    IDEFER(delete[] meshoptMeshlets);

    uint32_t* meshletVertices = new uint32_t[maxMeshletCount * MeshletVertexCount];
    IDEFER(delete[] meshletVertices);
    uint8_t* meshletTriangles = new uint8_t[maxMeshletCount * MeshletTriangleCount * 3];
    IDEFER(delete[] meshletTriangles);

    const size_t meshletCount = meshopt_buildMeshlets
    (
        meshoptMeshlets,
        meshletVertices,
        meshletTriangles,
        indices,
        indexCount,
        (float*)vertices,
        newVertexCount,
        a_vertexStride,
        MeshletVertexCount,
        MeshletTriangleCount,
        0.0f
    );

    IcarianCore::ShaderMeshletBuffer* meshlets = new IcarianCore::ShaderMeshletBuffer[meshletCount];
    IDEFER(delete[] meshlets);

    uint32_t meshletVertexCount = 0;
    uint32_t meshletTriangleCount = 0;
    for (size_t i = 0; i < meshletCount; ++i)
    {
        const meshopt_Meshlet& m = meshoptMeshlets[i];

        uint8_t* mTriangles = meshletTriangles + m.triangle_offset;
        uint32_t* mVertices = meshletVertices + m.vertex_offset;

        meshletVertexCount += m.vertex_count;
        meshletTriangleCount += m.triangle_count;

        meshopt_optimizeMeshlet(mVertices, mTriangles, m.triangle_count, m.vertex_count);

        glm::vec3 max = glm::vec3(std::numeric_limits<float>::min());
        glm::vec3 min = glm::vec3(std::numeric_limits<float>::max());

        for (uint32_t j = 0; j < m.vertex_count; ++j)
        {
            const uint32_t index = (uint32_t)meshletVertices[i];

            // TODO: Again code smell as we are making assumptions about the vertex layout
            const float* vPtr = (float*)(vertices + index * a_vertexStride);

            max.x = glm::max(vPtr[0], max.x);
            max.y = glm::max(vPtr[1], max.y);
            max.z = glm::max(vPtr[2], max.z);

            min.x = glm::min(vPtr[0], min.x);
            min.y = glm::min(vPtr[1], min.y);
            min.z = glm::min(vPtr[2], min.z);
        }

        const glm::vec3 bounds = max - min;
        const glm::vec3 halfBounds = bounds * 0.5f;

        const glm::vec3 center = min + halfBounds;
        const float radius = glm::length(halfBounds);

        meshlets[i].Data = glm::uvec4(m.vertex_offset, m.triangle_offset, m.vertex_count, m.triangle_count);
        meshlets[i].Bounds = glm::vec4(center, radius);
    }

    return Instance->GenerateMeshFromModel
    (
        vertices,
        (uint32_t)newVertexCount,
        a_vertexStride,
        meshletVertices,
        meshletVertexCount,
        meshletTriangles,
        meshletTriangleCount,
        meshlets,
        meshletCount,
        a_radius
    );
}, MonoArray* a_vertices, MonoArray* a_indices, uint16_t a_vertexStride, float a_radius)

RUNTIME_FUNCTION(void, RenderPipeline, SetLightSplits, 
{
    RENDERSCRATCHFRAME;

    const uint32_t lightSplitCount = (uint32_t)mono_array_length(a_lightSplits);

    LightShadowSplit* lightSplits = RenderScratchAlloc::TAllocate<LightShadowSplit>(lightSplitCount);

    for (uint32_t i = 0; i < lightSplitCount; ++i)
    {
        lightSplits[i] = mono_array_get(a_lightSplits, LightShadowSplit, i);
    }

    Instance->SetLightSplits(lightSplits, lightSplitCount);
}, MonoArray* a_lightSplits)

RUNTIME_FUNCTION(void, RenderCommand, PushShadowSplits, 
{
    RENDERSCRATCHFRAME;

    const uint32_t lightSplitCount = (uint32_t)mono_array_length(a_splits);

    LightShadowSplit* lightSplits = RenderScratchAlloc::TAllocate<LightShadowSplit>(lightSplitCount);

    for (uint32_t i = 0; i < lightSplitCount; ++i)
    {
        lightSplits[i] = mono_array_get(a_splits, LightShadowSplit, i);
    }

    Instance->PushLightSplits(a_slot, lightSplits, lightSplitCount);
}, uint32_t a_slot, MonoArray* a_splits)

VulkanGraphicsEngineBindings::VulkanGraphicsEngineBindings(VulkanGraphicsEngine* a_graphicsEngine)
{
    m_graphicsEngine = a_graphicsEngine;

    Instance = this;

    TRACE("Binding Vulkan functions to C#");
    VULKANGRAPHICS_BINDING_FUNCTION_TABLE(RUNTIME_FUNCTION_ATTACH)

    BIND_FUNCTION(IcarianEngine.Rendering.Shaders, ComputeShader, GenerateGraphicsFromFile);
    BIND_FUNCTION(IcarianEngine.Rendering.Shaders, ComputeShader, AddImport);
    BIND_FUNCTION(IcarianEngine.Rendering.Shaders, VertexShader, GenerateFromFile);
    BIND_FUNCTION(IcarianEngine.Rendering.Shaders, VertexShader, AddImport);
    BIND_FUNCTION(IcarianEngine.Rendering.Shaders, MeshShader, GenerateFromFile);
    BIND_FUNCTION(IcarianEngine.Rendering.Shaders, MeshShader, AddImport);
    BIND_FUNCTION(IcarianEngine.Rendering.Shaders, PixelShader, GenerateFromFile);
    BIND_FUNCTION(IcarianEngine.Rendering.Shaders, PixelShader, AddImport);

    BIND_FUNCTION(IcarianEngine.Rendering, Camera, GetProjectionMatrix);
    BIND_FUNCTION(IcarianEngine.Rendering, Camera, GetProjectionMatrixNF);

    BIND_FUNCTION(IcarianEngine.Rendering.Lighting, DirectionalLight, GetShadowMaps);

    BIND_FUNCTION(IcarianEngine.Rendering, Material, GenerateProgram);
    BIND_FUNCTION(IcarianEngine.Rendering, Material, GenerateMeshProgram);
    BIND_FUNCTION(IcarianEngine.Rendering, Material, GenerateComputeProgram);
    BIND_FUNCTION(IcarianEngine.Rendering, Material, SetUserUniform);
    BIND_FUNCTION(IcarianEngine.Rendering, Material, SetUserArray);
    BIND_FUNCTION(IcarianEngine.Rendering, Material, SetUserArrayCallback);
    BIND_FUNCTION(IcarianEngine.Rendering, Material, DestroyProgram);

    BIND_FUNCTION(IcarianEngine.Rendering, Mesh, GenerateFromModel);

    BIND_FUNCTION(IcarianEngine.Rendering, Model, GenerateModel);

    BIND_FUNCTION(IcarianEngine.Rendering, RenderCommand, PushShadowSplits);

    BIND_FUNCTION(IcarianEngine.Rendering, RenderPipeline, SetLightSplits);

    m_userArrayCallback = RuntimeManager::GetFunction("IcarianEngine.Rendering", "Material", ":DispatchUserArrayCallback(uint)");
    IVERIFY(m_userArrayCallback != nullptr);
}
VulkanGraphicsEngineBindings::~VulkanGraphicsEngineBindings()
{
    delete m_userArrayCallback;
}

uint32_t VulkanGraphicsEngineBindings::GenerateFComputeShaderAddr(const std::string_view& a_str) const
{
    return m_graphicsEngine->GenerateFComputeShader(a_str);
}
void VulkanGraphicsEngineBindings::AddComputeShaderImport(const std::string_view& a_key, const std::string_view& a_value) const
{
    IVERIFY(!a_key.empty());
    IVERIFY(!a_value.empty());

    const std::string k = std::string(a_key);
    const std::string v = std::string(a_value);

    const ThreadGuard g = ThreadGuard(m_graphicsEngine->m_importLock);

    auto iter = m_graphicsEngine->m_computeImports.find(k);
    if (iter != m_graphicsEngine->m_computeImports.end())
    {
        iter->second = v;

        return;
    }

    m_graphicsEngine->m_computeImports.emplace(k, v);
}
void VulkanGraphicsEngineBindings::DestroyComputeshader(uint32_t a_addr) const
{
    m_graphicsEngine->DestroyComputeShader(a_addr);
}

uint32_t VulkanGraphicsEngineBindings::GenerateFVertexShaderAddr(const std::string_view& a_str) const
{
    return m_graphicsEngine->GenerateFVertexShader(a_str);
}
void VulkanGraphicsEngineBindings::AddVertexShaderImport(const std::string_view& a_key, const std::string_view& a_value) const
{
    IVERIFY(!a_key.empty());
    IVERIFY(!a_value.empty());

    const std::string k = std::string(a_key);
    const std::string v = std::string(a_value);

    const ThreadGuard g = ThreadGuard(m_graphicsEngine->m_importLock);

    auto iter = m_graphicsEngine->m_vertexImports.find(k);
    if (iter != m_graphicsEngine->m_vertexImports.end())
    {
        iter->second = v;

        return;
    }

    m_graphicsEngine->m_vertexImports.emplace(k, v);
}
void VulkanGraphicsEngineBindings::DestroyVertexShader(uint32_t a_addr) const
{
    m_graphicsEngine->DestroyVertexShader(a_addr);
}

uint32_t VulkanGraphicsEngineBindings::GenerateFMeshShaderAddr(const std::string_view& a_str) const
{
    return m_graphicsEngine->GenerateFMeshShader(a_str);
}
void VulkanGraphicsEngineBindings::AddMeshShaderImport(const std::string_view& a_key, const std::string_view& a_value) const
{
    IVERIFY(!a_key.empty());
    IVERIFY(!a_value.empty());

    const std::string k = std::string(a_key);
    const std::string v = std::string(a_value);

    const ThreadGuard g = ThreadGuard(m_graphicsEngine->m_importLock);

    auto iter = m_graphicsEngine->m_meshImports.find(k);
    if (iter != m_graphicsEngine->m_meshImports.end())
    {
        iter->second = v;

        return;
    }

    m_graphicsEngine->m_meshImports.emplace(k, v);
}
void VulkanGraphicsEngineBindings::DestroyMeshShader(uint32_t a_addr) const
{
    m_graphicsEngine->DestroyMeshShader(a_addr);
}

uint32_t VulkanGraphicsEngineBindings::GenerateFPixelShaderAddr(const std::string_view& a_str) const
{
    return m_graphicsEngine->GenerateFPixelShader(a_str);
}
void VulkanGraphicsEngineBindings::AddPixelShaderImport(const std::string_view& a_key, const std::string_view& a_value) const
{
    IVERIFY(!a_key.empty());
    IVERIFY(!a_value.empty());

    const std::string k = std::string(a_key);
    const std::string v = std::string(a_value);

    const ThreadGuard g = ThreadGuard(m_graphicsEngine->m_importLock);

    auto iter = m_graphicsEngine->m_pixelImports.find(k);
    if (iter != m_graphicsEngine->m_pixelImports.end())
    {
        iter->second = v;

        return;
    }

    m_graphicsEngine->m_pixelImports.emplace(k, v);
}
void VulkanGraphicsEngineBindings::DestroyPixelShader(uint32_t a_addr) const
{
    m_graphicsEngine->DestroyPixelShader(a_addr);
}

uint32_t VulkanGraphicsEngineBindings::GenerateShaderProgram(const RenderProgram& a_program) const
{
    return m_graphicsEngine->GenerateRenderProgram(a_program);
}
void VulkanGraphicsEngineBindings::DestroyShaderProgram(uint32_t a_addr) const
{
    m_graphicsEngine->DestroyRenderProgram(a_addr);
}
void VulkanGraphicsEngineBindings::RenderProgramSetTexture(uint32_t a_addr, uint32_t a_shaderSlot, uint32_t a_samplerAddr) const
{
    IVERIFY(m_graphicsEngine->m_shaderPrograms.Exists(a_addr));
    IVERIFY(m_graphicsEngine->m_textureSampler.Exists(a_samplerAddr));

    TLockArray<RenderProgram> a = m_graphicsEngine->m_shaderPrograms.ToLockArray();

    const RenderProgram& program = a[a_addr];
    IVERIFY(program.Data != nullptr);

    VulkanShaderData* data = (VulkanShaderData*)program.Data;
    data->SetTexture(a_shaderSlot, a_samplerAddr);
}
void VulkanGraphicsEngineBindings::RenderProgramSetUserUBO(uint32_t a_addr, uint32_t a_uboSize, void* a_uboData) const
{
    IVERIFY(m_graphicsEngine->m_shaderPrograms.Exists(a_addr));

    TLockArray<RenderProgram> a = m_graphicsEngine->m_shaderPrograms.ToLockArray();
    
    RenderProgram& program = a[a_addr];

    if (program.UBOData != NULL)
    {
        free(program.UBOData);
    }

    program.UBODataSize = a_uboSize;
    program.UBOData = a_uboData;
}
void VulkanGraphicsEngineBindings::RenderProgramSetUserArray(uint32_t a_addr, void* a_data, uint32_t a_count, uint32_t a_stride) const
{
    IVERIFY(m_graphicsEngine->m_shaderPrograms.Exists(a_addr));

    TLockArray<RenderProgram> a = m_graphicsEngine->m_shaderPrograms.ToLockArray();

    RenderProgram& program = a[a_addr];

    IVERIFY(a_stride == program.UserArrayStride);

    if (program.UserArrayData != NULL)
    {
        free(program.UserArrayData);
    }

    program.UserArrayCount = a_count;
    program.UserArrayData = a_data;
}
RenderProgram VulkanGraphicsEngineBindings::GetRenderProgram(uint32_t a_addr) const
{
    return m_graphicsEngine->GetRenderProgram(a_addr);
}
void VulkanGraphicsEngineBindings::SetRenderProgram(uint32_t a_addr, const RenderProgram& a_program) const
{
    IVERIFY(m_graphicsEngine->m_shaderPrograms.Exists(a_addr));

    m_graphicsEngine->m_shaderPrograms.LockSet(a_addr, a_program);
}

uint32_t VulkanGraphicsEngineBindings::GenerateCameraBuffer(uint32_t a_transformAddr) const
{
    IVERIFY(a_transformAddr != uint32_t(-1));

    const CameraBuffer buff = CameraBuffer(a_transformAddr);

    uint32_t size = 0;
    {
        TRACE("Getting Camera Buffer");
        TLockArray<CameraBuffer> a = m_graphicsEngine->m_cameraBuffers.ToLockArray();

        size = a.Size();
        for (uint32_t i = 0; i < size; ++i)
        {
            if (a[i].TransformAddr == uint32_t(-1))
            {
                a[i] = buff;

                return i;
            }
        }
    }

    TRACE("Allocating Camera Buffer");
    m_graphicsEngine->m_cameraBuffers.Push(buff);

    return size;
}
void VulkanGraphicsEngineBindings::DestroyCameraBuffer(uint32_t a_addr) const
{
    IVERIFY(a_addr < m_graphicsEngine->m_cameraBuffers.Size());

    TLockArray<CameraBuffer> a = m_graphicsEngine->m_cameraBuffers.ToLockArray();
    a[a_addr].TransformAddr = -1;
}
CameraBuffer VulkanGraphicsEngineBindings::GetCameraBuffer(uint32_t a_addr) const
{
    IVERIFY(a_addr < m_graphicsEngine->m_cameraBuffers.Size());
    IVERIFY(m_graphicsEngine->m_cameraBuffers[a_addr].TransformAddr != uint32_t(-1));

    return m_graphicsEngine->m_cameraBuffers[a_addr];
}
void VulkanGraphicsEngineBindings::SetCameraBuffer(uint32_t a_addr, const CameraBuffer& a_buffer) const
{
    IVERIFY(a_addr < m_graphicsEngine->m_cameraBuffers.Size());
    IVERIFY(m_graphicsEngine->m_cameraBuffers[a_addr].TransformAddr != uint32_t(-1));

    m_graphicsEngine->m_cameraBuffers.LockSet(a_addr, a_buffer);
}
glm::vec3 VulkanGraphicsEngineBindings::CameraScreenToWorld(uint32_t a_addr, const glm::vec3& a_screenPos, const glm::vec2& a_screenSize) const
{
    IVERIFY(a_addr < m_graphicsEngine->m_cameraBuffers.Size());

    const CameraBuffer camBuf = m_graphicsEngine->m_cameraBuffers[a_addr];

    IVERIFY(camBuf.TransformAddr != uint32_t(-1));

    const glm::mat4 proj = camBuf.ToProjection(a_screenSize);
    const glm::mat4 invProj = glm::inverse(proj);

    const glm::mat4 invView = ObjectManager::GetGlobalMatrix(camBuf.TransformAddr);

    const glm::vec4 cPos = invProj * glm::vec4(a_screenPos.xy() * 2.0f - 1.0f, a_screenPos.z, 1.0f);
    const glm::vec4 wPos = invView * cPos;

    return wPos.xyz() / wPos.w;
}
glm::mat4 VulkanGraphicsEngineBindings::GetCameraProjectionMatrix(uint32_t a_addr, uint32_t a_width, uint32_t a_height) const
{
    IVERIFY(a_addr < m_graphicsEngine->m_cameraBuffers.Size());
    IVERIFY(m_graphicsEngine->m_cameraBuffers[a_addr].TransformAddr != uint32_t(-1));

    const CameraBuffer camBuf = m_graphicsEngine->m_cameraBuffers[a_addr];

    return camBuf.ToProjection(glm::vec2((float)a_width, (float)a_height));
}
glm::mat4 VulkanGraphicsEngineBindings::GetCameraProjectionMatrix(uint32_t a_addr, uint32_t a_width, uint32_t a_height, float a_near, float a_far) const
{
    IVERIFY(a_addr < m_graphicsEngine->m_cameraBuffers.Size());
    IVERIFY(m_graphicsEngine->m_cameraBuffers[a_addr].TransformAddr != uint32_t(-1));

    const CameraBuffer camBuf = m_graphicsEngine->m_cameraBuffers[a_addr];

    return camBuf.ToProjection(glm::vec2((float)a_width, (float)a_height), a_near, a_far);
}

uint32_t VulkanGraphicsEngineBindings::GenerateModel(const void* a_vertices, uint32_t a_vertexCount, const uint32_t* a_indices, uint32_t a_indexCount, uint16_t a_vertexStride, float a_radius) const
{
    return m_graphicsEngine->GenerateModel(a_vertices, a_vertexCount, a_vertexStride, a_indices, a_indexCount, a_radius);
}
void VulkanGraphicsEngineBindings::DestroyModel(uint32_t a_addr) const
{
    m_graphicsEngine->DestroyModel(a_addr);    
}

uint32_t VulkanGraphicsEngineBindings::GenerateMeshFromModel
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
) const
{
    IVERIFY(a_vertices != nullptr);
    IVERIFY(a_vertexCount > 0);
    IVERIFY(a_vertexStride > 0);
    IVERIFY(a_meshletVertices != nullptr);
    IVERIFY(a_meshletVertexCount > 0);
    IVERIFY(a_meshletTriangles != nullptr);
    IVERIFY(a_meshletTriangleCount > 0);
    IVERIFY(a_meshlets != nullptr);
    IVERIFY(a_meshletCount > 0);

    VulkanRenderEngineBackend* engine = m_graphicsEngine->m_vulkanEngine;

    BlockAllocator* allocator = engine->GetBlockAllocator();

    VulkanMesh* mesh = allocator->Create<VulkanMesh>
    (
        engine,
        a_vertices,
        a_vertexCount,
        a_vertexStride,
        a_meshletVertices,
        a_meshletVertexCount,
        a_meshletTriangles,
        a_meshletTriangleCount,
        a_meshlets,
        a_meshletCount,
        a_radius
    );

    return m_graphicsEngine->m_meshes.PushVal(mesh);
}
void VulkanGraphicsEngineBindings::DestroyMesh(uint32_t a_addr) const
{
    IVERIFY(m_graphicsEngine->m_meshes.Exists(a_addr));

    BlockAllocator* allocator = m_graphicsEngine->m_vulkanEngine->GetBlockAllocator();

    VulkanMesh* mesh = m_graphicsEngine->m_meshes[a_addr];
    IDEFER(allocator->Destroy(mesh));

    m_graphicsEngine->m_meshes.Erase(a_addr);
}

uint32_t VulkanGraphicsEngineBindings::GenerateModelRenderBuffer(uint32_t a_materialAddr, uint32_t a_modelAddr, uint32_t a_transformAddr) const
{
    IVERIFY(m_graphicsEngine->m_shaderPrograms.Exists(a_materialAddr));

    const ModelRenderBuffer buffer = 
    {
        .MaterialAddr = a_materialAddr,
        .ModelAddr = a_modelAddr,
        .TransformAddr = a_transformAddr
    };

    return m_graphicsEngine->m_renderBuffers.PushVal(buffer);
}
void VulkanGraphicsEngineBindings::DestroyModelRenderBuffer(uint32_t a_addr) const
{
    IVERIFY(m_graphicsEngine->m_renderBuffers.Exists(a_addr));

    m_graphicsEngine->m_renderBuffers.Erase(a_addr);
}
void VulkanGraphicsEngineBindings::GenerateModelRenderStack(uint32_t a_modelAddr) const
{
    IVERIFY(m_graphicsEngine->m_renderBuffers.Exists(a_modelAddr));

    BlockAllocator* allocator = m_graphicsEngine->m_vulkanEngine->GetBlockAllocator();

    const ModelRenderBuffer buffer = m_graphicsEngine->m_renderBuffers[a_modelAddr];

    {
        TLockArray<MaterialRenderStack*> a = m_graphicsEngine->m_renderStacks.ToLockArray();
        for (MaterialRenderStack* stack : a)
        {
            if (stack->Add(buffer))
            {
                return;
            }
        }
    }
    
    TRACE("Allocating Model RenderStack");
    m_graphicsEngine->m_renderStacks.Push(allocator->Create<MaterialRenderStack>(allocator, buffer));
}
void VulkanGraphicsEngineBindings::DestroyModelRenderStack(uint32_t a_modelAddr) const
{
    IVERIFY(m_graphicsEngine->m_renderBuffers.Exists(a_modelAddr));

    BlockAllocator* allocator = m_graphicsEngine->m_vulkanEngine->GetBlockAllocator();

    const ModelRenderBuffer buffer = m_graphicsEngine->m_renderBuffers[a_modelAddr];

    TLockArray<MaterialRenderStack*> a = m_graphicsEngine->m_renderStacks.ToLockArray();

    const uint32_t size = a.Size();
    for (uint32_t i = 0; i < size; ++i)
    {
        MaterialRenderStack* stack = a[i];

        if (stack->Remove(buffer)) 
        {
            if (stack->Empty()) 
            {
                IDEFER(allocator->Destroy(stack));

                TRACE("Destroying Model RenderStack");
                m_graphicsEngine->m_renderStacks.UErase(i);
            }

            return;
        }
    }
}

uint32_t VulkanGraphicsEngineBindings::GenerateSkinnedModelRenderBuffer(uint32_t a_materialAddr, uint32_t a_modelAddr, uint32_t a_transformAddr, uint32_t a_skeletonAddr) const
{   
    IVERIFY(m_graphicsEngine->m_shaderPrograms.Exists(a_materialAddr));

    const SkinnedModelRenderBuffer buffer = 
    {
        .SkeletonAddr = a_skeletonAddr,
        .MaterialAddr = a_materialAddr,
        .ModelAddr = a_modelAddr,
        .TransformAddr = a_transformAddr,
    };

    return m_graphicsEngine->m_skinnedRenderBuffers.PushVal(buffer);
}
void VulkanGraphicsEngineBindings::DestroySkinnedModelRenderBuffer(uint32_t a_addr) const
{
    IVERIFY(m_graphicsEngine->m_skinnedRenderBuffers.Exists(a_addr));

    m_graphicsEngine->m_skinnedRenderBuffers.Erase(a_addr);
}
void VulkanGraphicsEngineBindings::GenerateSkinnedModelRenderStack(uint32_t a_addr) const
{
    IVERIFY(m_graphicsEngine->m_skinnedRenderBuffers.Exists(a_addr));

    BlockAllocator* allocator = m_graphicsEngine->m_vulkanEngine->GetBlockAllocator();

    const SkinnedModelRenderBuffer buffer = m_graphicsEngine->m_skinnedRenderBuffers[a_addr];

    {
        TLockArray<MaterialRenderStack*> a = m_graphicsEngine->m_renderStacks.ToLockArray();
        for (MaterialRenderStack* stack : a)
        {
            if (stack->Add(buffer))
            {
                return;
            }
        }
    }

    TRACE("Allocating Skinned RenderStack");
    m_graphicsEngine->m_renderStacks.Push(allocator->Create<MaterialRenderStack>(allocator, buffer));
}
void VulkanGraphicsEngineBindings::DestroySkinnedModelRenderStack(uint32_t a_addr) const
{
    TRACE("Removing Skinned RenderStack");
    IVERIFY(m_graphicsEngine->m_skinnedRenderBuffers.Exists(a_addr));

    BlockAllocator* allocator = m_graphicsEngine->m_vulkanEngine->GetBlockAllocator();

    const SkinnedModelRenderBuffer buffer = m_graphicsEngine->m_skinnedRenderBuffers[a_addr];
    TLockArray<MaterialRenderStack*> a = m_graphicsEngine->m_renderStacks.ToLockArray();

    const uint32_t size = a.Size();
    for (uint32_t i = 0; i < size; ++i)
    {
        if (a[i]->Remove(buffer))
        {
            if (a[i]->Empty())
            {
                MaterialRenderStack* stack = a[i];
                IDEFER(allocator->Destroy(stack));

                TRACE("Destroying Skinned RenderStack");
                m_graphicsEngine->m_renderStacks.UErase(i);
            }

            return;
        }
    }
}

uint32_t VulkanGraphicsEngineBindings::GenerateMeshRenderBuffer(uint32_t a_materialAddr, uint32_t a_meshAddr, uint32_t a_transformAddr, uint32_t a_indexCount) const
{
    IVERIFY(m_graphicsEngine->m_shaderPrograms.Exists(a_materialAddr));

    const MeshRenderBuffer buffer =
    {
        .MaterialAddr = a_materialAddr,
        .MeshAddr = a_meshAddr,
        .TransformAddr = a_transformAddr,
        .IndexCount = a_indexCount,
    };

    return m_graphicsEngine->m_meshRenderBuffers.PushVal(buffer);
}
void VulkanGraphicsEngineBindings::DestroyMeshRenderBuffer(uint32_t a_addr) const
{
    IVERIFY(m_graphicsEngine->m_meshRenderBuffers.Exists(a_addr));

    m_graphicsEngine->m_meshRenderBuffers.Erase(a_addr);
}
void VulkanGraphicsEngineBindings::GenerateMeshRenderStack(uint32_t a_addr) const
{
    IVERIFY(m_graphicsEngine->m_meshRenderBuffers.Exists(a_addr));

    BlockAllocator* allocator = m_graphicsEngine->m_vulkanEngine->GetBlockAllocator();

    const MeshRenderBuffer buffer = m_graphicsEngine->m_meshRenderBuffers[a_addr];

    {
        TLockArray<MaterialRenderStack*> a = m_graphicsEngine->m_renderStacks.ToLockArray();
        for (MaterialRenderStack* stack : a)
        {
            if (stack->Add(buffer))
            {
                return;
            }
        }
    }

    TRACE("Allocating Mesh RenderStack");
    m_graphicsEngine->m_renderStacks.Push(allocator->Create<MaterialRenderStack>(allocator, buffer));
}
void VulkanGraphicsEngineBindings::DestroyMeshRenderStack(uint32_t a_addr) const
{
    IVERIFY(m_graphicsEngine->m_meshRenderBuffers.Exists(a_addr));

    BlockAllocator* allocator = m_graphicsEngine->m_vulkanEngine->GetBlockAllocator();

    const MeshRenderBuffer buffer = m_graphicsEngine->m_meshRenderBuffers[a_addr];
    TLockArray<MaterialRenderStack*> a = m_graphicsEngine->m_renderStacks.ToLockArray();

    const uint32_t size = a.Size();
    for (uint32_t i = 0; i < size; ++i)
    {
        if (a[i]->Remove(buffer))
        {
            if (a[i]->Empty())
            {
                MaterialRenderStack* stack = a[i];
                IDEFER(allocator->Destroy(stack));

                TRACE("Destroying Skinned RenderStack");
                m_graphicsEngine->m_renderStacks.UErase(i);
            }

            return;
        }
    }
}

uint32_t VulkanGraphicsEngineBindings::GenerateGraphicsParticle2D(uint32_t a_computeBufferAddr) const
{
    BlockAllocator* allocator = m_graphicsEngine->m_vulkanEngine->GetBlockAllocator();

    VulkanGraphicsParticle2D* particleSystem = allocator->Create<VulkanGraphicsParticle2D>(m_graphicsEngine->m_vulkanEngine, m_graphicsEngine->m_vulkanEngine->GetComputeEngine(), m_graphicsEngine, a_computeBufferAddr);

    return m_graphicsEngine->m_particleEmitters.PushVal(particleSystem);   
}
void VulkanGraphicsEngineBindings::DestroyGraphicsParticle2D(uint32_t a_addr) const
{
    IVERIFY(m_graphicsEngine->m_particleEmitters.Exists(a_addr));

    BlockAllocator* allocator = m_graphicsEngine->m_vulkanEngine->GetBlockAllocator();

    VulkanGraphicsParticle2D* particleSystem = m_graphicsEngine->m_particleEmitters[a_addr];
    IDEFER(allocator->Destroy(particleSystem));
    m_graphicsEngine->m_particleEmitters.Erase(a_addr);
}

void VulkanGraphicsEngineBindings::DestroyTexture(uint32_t a_addr) const
{
    m_graphicsEngine->DestroyTexture(a_addr);
}

uint32_t VulkanGraphicsEngineBindings::GenerateVideoTexture(uint32_t a_videoAddr) const
{
    BlockAllocator* allocator = m_graphicsEngine->m_vulkanEngine->GetBlockAllocator();

    VulkanVideoTexture* texture = allocator->Create<VulkanVideoTexture>(m_graphicsEngine->m_vulkanEngine, a_videoAddr);

    return m_graphicsEngine->m_videoTextures.PushVal(texture);
}
void VulkanGraphicsEngineBindings::DestroyVideoTexture(uint32_t a_addr) const
{
    IVERIFY(m_graphicsEngine->m_videoTextures.Exists(a_addr));

    BlockAllocator* allocator = m_graphicsEngine->m_vulkanEngine->GetBlockAllocator();

    VulkanVideoTexture* texture = m_graphicsEngine->m_videoTextures[a_addr];
    IDEFER(allocator->Destroy(texture));
    m_graphicsEngine->m_videoTextures.Erase(a_addr);
}

uint32_t VulkanGraphicsEngineBindings::GenerateTextureSampler(uint32_t a_texture, e_TextureFilter a_filter, e_TextureAddress a_addressMode) const
{
    return m_graphicsEngine->GenerateTextureSampler(a_texture, TextureMode_Texture, a_filter, a_addressMode);
}
uint32_t VulkanGraphicsEngineBindings::GenerateRenderTextureSampler(uint32_t a_renderTexture, uint32_t a_textureIndex, e_TextureFilter a_filter, e_TextureAddress a_addressMode) const
{
    return m_graphicsEngine->GenerateTextureSampler(a_renderTexture, TextureMode_RenderTexture, a_filter, a_addressMode, a_textureIndex);
}
uint32_t VulkanGraphicsEngineBindings::GenerateRenderTextureDepthSampler(uint32_t a_renderTexture, e_TextureFilter a_filter, e_TextureAddress a_addressMode) const
{
    return m_graphicsEngine->GenerateTextureSampler(a_renderTexture, TextureMode_RenderTextureDepth, a_filter, a_addressMode);
}
uint32_t VulkanGraphicsEngineBindings::GenerateRenderTextureDepthSamplerDepth(uint32_t a_renderTexture, e_TextureFilter a_filter, e_TextureAddress a_addressMode) const
{
    return m_graphicsEngine->GenerateTextureSampler(a_renderTexture, TextureMode_DepthRenderTexture, a_filter, a_addressMode);
}
void VulkanGraphicsEngineBindings::DestroyTextureSampler(uint32_t a_addr) const
{
    return m_graphicsEngine->DestroyTextureSampler(a_addr);
}

uint32_t VulkanGraphicsEngineBindings::GenerateRenderTexture(uint32_t a_count, uint32_t a_width, uint32_t a_height, bool a_depthTexture, bool a_hdr, uint32_t a_channelCount) const
{
    IVERIFY(a_count > 0);
    IVERIFY(a_width > 0);
    IVERIFY(a_height > 0);
    IVERIFY(a_channelCount > 0);
    IVERIFY(a_channelCount <= 4);

    BlockAllocator* allocator = m_graphicsEngine->m_vulkanEngine->GetBlockAllocator();

    VulkanRenderTexture* texture = allocator->Create<VulkanRenderTexture>(m_graphicsEngine->m_vulkanEngine, m_graphicsEngine, a_count, a_width, a_height, a_depthTexture, a_hdr, a_channelCount);

    return m_graphicsEngine->m_renderTextures.PushVal(texture);
}
uint32_t VulkanGraphicsEngineBindings::GenerateRenderTextureD(uint32_t a_count, uint32_t a_width, uint32_t a_height, uint32_t a_depthHandle, bool a_hdr, uint32_t a_channelCount) const
{
    IVERIFY(a_count > 0);
    IVERIFY(a_width > 0);
    IVERIFY(a_height > 0);
    IVERIFY(a_channelCount > 0);
    IVERIFY(a_channelCount <= 4);

    IVERIFY(m_graphicsEngine->m_depthRenderTextures.Exists(a_depthHandle));

    BlockAllocator* allocator = m_graphicsEngine->m_vulkanEngine->GetBlockAllocator();

    VulkanRenderTexture* texture = allocator->Create<VulkanRenderTexture>(m_graphicsEngine->m_vulkanEngine, m_graphicsEngine, a_count, a_width, a_height, a_depthHandle, a_hdr, a_channelCount);

    return m_graphicsEngine->m_renderTextures.PushVal(texture);
}
void VulkanGraphicsEngineBindings::DestroyRenderTexture(uint32_t a_addr) const
{
    IVERIFY(m_graphicsEngine->m_renderTextures.Exists(a_addr));

    BlockAllocator* allocator = m_graphicsEngine->m_vulkanEngine->GetBlockAllocator();

    VulkanRenderTexture* tex = m_graphicsEngine->m_renderTextures[a_addr];
    IDEFER(allocator->Destroy(tex));

    m_graphicsEngine->m_renderTextures.Erase(a_addr);
}
uint32_t VulkanGraphicsEngineBindings::GetRenderTextureTextureCount(uint32_t a_addr) const
{
    IVERIFY(m_graphicsEngine->m_renderTextures.Exists(a_addr));

    const VulkanRenderTexture* texture = m_graphicsEngine->m_renderTextures[a_addr];

    return texture->GetTextureCount();
}
bool VulkanGraphicsEngineBindings::RenderTextureHasDepth(uint32_t a_addr) const
{
    IVERIFY(m_graphicsEngine->m_renderTextures.Exists(a_addr));

    const VulkanRenderTexture* texture = m_graphicsEngine->m_renderTextures[a_addr];

    return texture->HasDepthTexture();
}
uint32_t VulkanGraphicsEngineBindings::GetRenderTextureWidth(uint32_t a_addr) const
{
    IVERIFY(m_graphicsEngine->m_renderTextures.Exists(a_addr));

    const VulkanRenderTexture* texture = m_graphicsEngine->m_renderTextures[a_addr];

    return texture->GetWidth();
}
uint32_t VulkanGraphicsEngineBindings::GetRenderTextureHeight(uint32_t a_addr) const
{
    IVERIFY(m_graphicsEngine->m_renderTextures.Exists(a_addr));

    const VulkanRenderTexture* texture = m_graphicsEngine->m_renderTextures[a_addr];

    return texture->GetHeight();
}
void VulkanGraphicsEngineBindings::ResizeRenderTexture(uint32_t a_addr, uint32_t a_width, uint32_t a_height) const
{
    IVERIFY(m_graphicsEngine->m_renderTextures.Exists(a_addr));
    IVERIFY(a_width > 0);
    IVERIFY(a_height > 0);

    TLockArray<VulkanRenderTexture*> a = m_graphicsEngine->m_renderTextures.ToLockArray();

    VulkanRenderTexture* texture = a[a_addr];
    texture->Resize(a_width, a_height);
}

uint32_t VulkanGraphicsEngineBindings::GenerateDepthRenderTexture(uint32_t a_width, uint32_t a_height) const
{
    return m_graphicsEngine->GenerateDepthRenderTexture(a_width, a_height);
}
void VulkanGraphicsEngineBindings::DestroyDepthRenderTexture(uint32_t a_addr) const
{
    return m_graphicsEngine->DestroyDepthRenderTexture(a_addr);
}
uint32_t VulkanGraphicsEngineBindings::GetDepthRenderTextureWidth(uint32_t a_addr) const
{
    IVERIFY(m_graphicsEngine->m_depthRenderTextures.Exists(a_addr));

    const VulkanDepthRenderTexture* texture = m_graphicsEngine->m_depthRenderTextures[a_addr];
    
    return texture->GetWidth();
}
uint32_t VulkanGraphicsEngineBindings::GetDepthRenderTextureHeight(uint32_t a_addr) const
{
    IVERIFY(m_graphicsEngine->m_depthRenderTextures.Exists(a_addr));

    const VulkanDepthRenderTexture* texture = m_graphicsEngine->m_depthRenderTextures[a_addr];

    return texture->GetHeight();
}
void VulkanGraphicsEngineBindings::ResizeDepthRenderTexture(uint32_t a_addr, uint32_t a_width, uint32_t a_height) const
{
    IVERIFY(m_graphicsEngine->m_depthRenderTextures.Exists(a_addr));
    IVERIFY(a_width > 0);
    IVERIFY(a_height > 0);

    TLockArray<VulkanDepthRenderTexture*> a = m_graphicsEngine->m_depthRenderTextures.ToLockArray();

    VulkanDepthRenderTexture* texture = a[a_addr];

    texture->Resize(a_width, a_height);
}

uint32_t VulkanGraphicsEngineBindings::GenerateDepthCubeRenderTexture(uint32_t a_width, uint32_t a_height) const
{
    IVERIFY(a_width > 0);
    IVERIFY(a_height > 0);

    BlockAllocator* allocator = m_graphicsEngine->m_vulkanEngine->GetBlockAllocator();

    VulkanDepthCubeRenderTexture* texture = allocator->Create<VulkanDepthCubeRenderTexture>(m_graphicsEngine->m_vulkanEngine, a_width, a_height);

    return m_graphicsEngine->m_depthCubeRenderTextures.PushVal(texture);
}
void VulkanGraphicsEngineBindings::DestroyDepthCubeRenderTexture(uint32_t a_addr) const
{
    IVERIFY(m_graphicsEngine->m_depthCubeRenderTextures.Exists(a_addr));

    BlockAllocator* allocator = m_graphicsEngine->m_vulkanEngine->GetBlockAllocator();

    VulkanDepthCubeRenderTexture* tex = m_graphicsEngine->m_depthCubeRenderTextures[a_addr];
    IDEFER(allocator->Destroy(tex));

    m_graphicsEngine->m_depthCubeRenderTextures.Erase(a_addr);
}
uint32_t VulkanGraphicsEngineBindings::GetDepthCubeRenderTextureWidth(uint32_t a_addr) const
{
    IVERIFY(m_graphicsEngine->m_depthCubeRenderTextures.Exists(a_addr));

    const VulkanDepthCubeRenderTexture* texture = m_graphicsEngine->m_depthCubeRenderTextures[a_addr];

    return texture->GetWidth();
}
uint32_t VulkanGraphicsEngineBindings::GetDepthCubeRenderTextureHeight(uint32_t a_addr) const
{
    IVERIFY(m_graphicsEngine->m_depthCubeRenderTextures.Exists(a_addr));

    const VulkanDepthCubeRenderTexture* texture = m_graphicsEngine->m_depthCubeRenderTextures[a_addr];

    return texture->GetHeight();
}
void VulkanGraphicsEngineBindings::ResizeDepthCubeRenderTexture(uint32_t a_addr, uint32_t a_width, uint32_t a_height) const
{
    IVERIFY(m_graphicsEngine->m_depthCubeRenderTextures.Exists(a_addr));
    IVERIFY(a_width > 0);
    IVERIFY(a_height > 0);

    TLockArray<VulkanDepthCubeRenderTexture*> a = m_graphicsEngine->m_depthCubeRenderTextures.ToLockArray();

    VulkanDepthCubeRenderTexture* texture = a[a_addr];

    texture->Resize(a_width, a_height);
}

uint32_t VulkanGraphicsEngineBindings::GenerateAmbientLightBuffer() const
{
    const AmbientLightBuffer buffer = 
    {
        .RenderLayer = 0b1,
        .Color = glm::vec4(1.0f),
        .Intensity = 1.0f
    };

    return m_graphicsEngine->m_ambientLights.PushVal(buffer);
}
void VulkanGraphicsEngineBindings::SetAmbientLightBuffer(uint32_t a_addr, const AmbientLightBuffer& a_buffer) const
{
    IVERIFY(m_graphicsEngine->m_ambientLights.Exists(a_addr));

    m_graphicsEngine->m_ambientLights.LockSet(a_addr, a_buffer);
}
AmbientLightBuffer VulkanGraphicsEngineBindings::GetAmbientLightBuffer(uint32_t a_addr) const
{
    IVERIFY(m_graphicsEngine->m_ambientLights.Exists(a_addr));

    return m_graphicsEngine->GetAmbientLight(a_addr);
}
void VulkanGraphicsEngineBindings::DestroyAmbientLightBuffer(uint32_t a_addr) const
{
    IVERIFY(m_graphicsEngine->m_ambientLights.Exists(a_addr));

    m_graphicsEngine->m_ambientLights.Erase(a_addr);
}

uint32_t VulkanGraphicsEngineBindings::GenerateDirectionalLightBuffer(uint32_t a_transformAddr) const
{
    IVERIFY(a_transformAddr != uint32_t(-1));

    BlockAllocator* allocator = m_graphicsEngine->m_vulkanEngine->GetBlockAllocator();

    const DirectionalLightBuffer buffer = 
    {
        .TransformAddr = a_transformAddr,
        .RenderLayer = 0b1,
        .Color = glm::vec4(1.0f),
        .Intensity = 1.0f,
        .Data = ILAMBDA(
        {
            VulkanLightBuffer* val = allocator->TAllocate<VulkanLightBuffer>();

            val->LightRenderTextureCount = 0;
            val->LightRenderTextures = nullptr;

            ILRETURN val;
        })
    };

    return m_graphicsEngine->m_directionalLights.PushVal(buffer);
}
void VulkanGraphicsEngineBindings::SetDirectionalLightBuffer(uint32_t a_addr, const DirectionalLightBuffer& a_buffer) const
{
    IVERIFY(m_graphicsEngine->m_directionalLights.Exists(a_addr));

    m_graphicsEngine->m_directionalLights.LockSet(a_addr, a_buffer);
}
DirectionalLightBuffer VulkanGraphicsEngineBindings::GetDirectionalLightBuffer(uint32_t a_addr) const
{
    IVERIFY(m_graphicsEngine->m_directionalLights.Exists(a_addr));

    return m_graphicsEngine->GetDirectionalLight(a_addr);
}
void VulkanGraphicsEngineBindings::DestroyDirectionalLightBuffer(uint32_t a_addr) const
{
    IVERIFY(m_graphicsEngine->m_directionalLights.Exists(a_addr));

    const DirectionalLightBuffer buffer = m_graphicsEngine->m_directionalLights[a_addr];
    IVERIFY(buffer.Data != nullptr);

    BlockAllocator* allocator = m_graphicsEngine->m_vulkanEngine->GetBlockAllocator();

    VulkanLightBuffer* lightBuffer = (VulkanLightBuffer*)buffer.Data;

    IDEFER(
    {
        uint32_t* textures = lightBuffer->LightRenderTextures;
        if (textures != nullptr)
        {
            allocator->Free(textures);
        }

        allocator->Free(lightBuffer);
    });

    m_graphicsEngine->m_directionalLights.Erase(a_addr);
}
void VulkanGraphicsEngineBindings::AddDirectionalLightShadowMap(uint32_t a_addr, uint32_t a_shadowMapAddr) const
{
    IVERIFY(m_graphicsEngine->m_directionalLights.Exists(a_addr));
    IVERIFY(m_graphicsEngine->m_depthRenderTextures.Exists(a_shadowMapAddr));

    BlockAllocator* allocator = m_graphicsEngine->m_vulkanEngine->GetBlockAllocator();

    TLockArray<DirectionalLightBuffer> a = m_graphicsEngine->m_directionalLights.ToLockArray();

    const DirectionalLightBuffer& buffer = a[a_addr];
    VulkanLightBuffer* lightBuffer = (VulkanLightBuffer*)buffer.Data;
    uint32_t* oldRenderTexture = lightBuffer->LightRenderTextures;
    IDEFER(
    if (oldRenderTexture != nullptr)
    {
        allocator->Free(oldRenderTexture);
    });

    uint32_t* renderTextures = allocator->TAllocate<uint32_t>(lightBuffer->LightRenderTextureCount + 1);
    for (uint32_t i = 0; i < lightBuffer->LightRenderTextureCount; ++i)
    {
        renderTextures[i] = lightBuffer->LightRenderTextures[i];
    }

    renderTextures[lightBuffer->LightRenderTextureCount] = a_shadowMapAddr;

    lightBuffer->LightRenderTextures = renderTextures;
    ++lightBuffer->LightRenderTextureCount;
}
void VulkanGraphicsEngineBindings::RemoveDirectionalLightShadowMap(uint32_t a_addr, uint32_t a_shadowMapAddr) const
{
    IVERIFY(m_graphicsEngine->m_directionalLights.Exists(a_addr));
    IVERIFY(m_graphicsEngine->m_depthRenderTextures.Exists(a_shadowMapAddr));

    TLockArray<DirectionalLightBuffer> a = m_graphicsEngine->m_directionalLights.ToLockArray();

    const DirectionalLightBuffer& buffer = a[a_addr];
    VulkanLightBuffer* lightBuffer = (VulkanLightBuffer*)buffer.Data;

    uint32_t index = 0;
    for (uint32_t i = 0; i < lightBuffer->LightRenderTextureCount; ++i)
    {
        if (lightBuffer->LightRenderTextures[i] != a_shadowMapAddr)
        {
            lightBuffer->LightRenderTextures[index++] = lightBuffer->LightRenderTextures[i];
        }
    } 
    --lightBuffer->LightRenderTextureCount;
}

uint32_t VulkanGraphicsEngineBindings::GeneratePointLightBuffer(uint32_t a_transformAddr) const
{
    IVERIFY(a_transformAddr != uint32_t(-1));

    BlockAllocator* allocator = m_graphicsEngine->m_vulkanEngine->GetBlockAllocator();

    const PointLightBuffer buffer =
    {
        .TransformAddr = a_transformAddr,
        .RenderLayer = 0b1,
        .Color = glm::vec4(1.0f),
        .Intensity = 1.0f,
        .Radius = 1.0f,
        .Data = ILAMBDA(
        {
            VulkanLightBuffer* data = allocator->TAllocate<VulkanLightBuffer>();

            data->LightRenderTextureCount = 0;
            data->LightRenderTextures = nullptr;

            ILRETURN data;
        })
    };

    return m_graphicsEngine->m_pointLights.PushVal(buffer);
}
void VulkanGraphicsEngineBindings::SetPointLightBuffer(uint32_t a_addr, const PointLightBuffer& a_buffer) const
{
    IVERIFY(m_graphicsEngine->m_pointLights.Exists(a_addr));

    m_graphicsEngine->m_pointLights.LockSet(a_addr, a_buffer);
}
PointLightBuffer VulkanGraphicsEngineBindings::GetPointLightBuffer(uint32_t a_addr) const
{
    IVERIFY(m_graphicsEngine->m_pointLights.Exists(a_addr));

    return m_graphicsEngine->GetPointLight(a_addr);
}
void VulkanGraphicsEngineBindings::DestroyPointLightBuffer(uint32_t a_addr) const
{
    IVERIFY(m_graphicsEngine->m_pointLights.Exists(a_addr));

    const PointLightBuffer buffer = m_graphicsEngine->m_pointLights[a_addr];
    IVERIFY(buffer.Data != nullptr);

    BlockAllocator* allocator = m_graphicsEngine->m_vulkanEngine->GetBlockAllocator();

    VulkanLightBuffer* data = (VulkanLightBuffer*)buffer.Data;
    IDEFER(
    {
        if (data->LightRenderTextures != nullptr)
        {
            allocator->Free(data->LightRenderTextures);
        }

        allocator->Free(data);
    });

    m_graphicsEngine->m_pointLights.Erase(a_addr);
}
void VulkanGraphicsEngineBindings::SetPointLightShadowMap(uint32_t a_addr, uint32_t a_shadowMapAddr) const
{
    IVERIFY(m_graphicsEngine->m_pointLights.Exists(a_addr));
    
    BlockAllocator* allocator = m_graphicsEngine->m_vulkanEngine->GetBlockAllocator();

    TLockArray<PointLightBuffer> a = m_graphicsEngine->m_pointLights.ToLockArray();

    const PointLightBuffer& buffer = a[a_addr];
    IVERIFY(buffer.Data != nullptr);

    VulkanLightBuffer* lightBuffer = (VulkanLightBuffer*)buffer.Data;

    uint32_t* renderTextures = lightBuffer->LightRenderTextures;

    if (a_shadowMapAddr != uint32_t(-1))
    {
        IVERIFY(m_graphicsEngine->m_depthCubeRenderTextures.Exists(a_shadowMapAddr));

        if (renderTextures == nullptr)
        {
            renderTextures = allocator->TAllocate<uint32_t>(1);
        }

        renderTextures[0] = a_shadowMapAddr;

        lightBuffer->LightRenderTextures = renderTextures;
        lightBuffer->LightRenderTextureCount = 1;
    }
    else if (renderTextures != nullptr)
    {
        lightBuffer->LightRenderTextures = nullptr;
        lightBuffer->LightRenderTextureCount = 0;

        allocator->Free(renderTextures);
    }
}
uint32_t VulkanGraphicsEngineBindings::GetPointLightShadowMap(uint32_t a_addr) const
{
    IVERIFY(m_graphicsEngine->m_pointLights.Exists(a_addr));

    const TReadLockArray<PointLightBuffer> a = m_graphicsEngine->m_pointLights.ToReadLockArray();

    const PointLightBuffer& buffer = a[a_addr];
    IVERIFY(buffer.Data != nullptr);

    const VulkanLightBuffer* lightBuffer = (VulkanLightBuffer*)buffer.Data;
    if (lightBuffer->LightRenderTextureCount == 0)
    {
        return -1;
    }

    return lightBuffer->LightRenderTextures[0];
}

uint32_t VulkanGraphicsEngineBindings::GenerateSpotLightBuffer(uint32_t a_transformAddr) const
{
    IVERIFY(a_transformAddr != uint32_t(-1));
    
    BlockAllocator* allocator = m_graphicsEngine->m_vulkanEngine->GetBlockAllocator();

    const SpotLightBuffer buffer =
    {
        .TransformAddr = a_transformAddr,
        .RenderLayer = 0b1,
        .Color = glm::vec4(1.0f),
        .Intensity = 1.0f,
        .CutoffAngle = glm::vec2(1.0f, 1.5f),
        .Radius = 1.0f,
        .Data = ILAMBDA(
        {
            VulkanLightBuffer* data = allocator->TAllocate<VulkanLightBuffer>();

            data->LightRenderTextureCount = 0;
            data->LightRenderTextures = nullptr;

            ILRETURN data;
        })
    };

    return m_graphicsEngine->m_spotLights.PushVal(buffer);
}
void VulkanGraphicsEngineBindings::SetSpotLightBuffer(uint32_t a_addr, const SpotLightBuffer& a_buffer) const
{
    IVERIFY(m_graphicsEngine->m_spotLights.Exists(a_addr));

    m_graphicsEngine->m_spotLights.LockSet(a_addr, a_buffer);
}
SpotLightBuffer VulkanGraphicsEngineBindings::GetSpotLightBuffer(uint32_t a_addr) const
{
    IVERIFY(m_graphicsEngine->m_spotLights.Exists(a_addr));

    return m_graphicsEngine->GetSpotLight(a_addr);
}
void VulkanGraphicsEngineBindings::DestroySpotLightBuffer(uint32_t a_addr) const
{
    IVERIFY(m_graphicsEngine->m_spotLights.Exists(a_addr));

    BlockAllocator* allocator = m_graphicsEngine->m_vulkanEngine->GetBlockAllocator();

    const SpotLightBuffer buffer = m_graphicsEngine->m_spotLights[a_addr];
    IVERIFY(buffer.Data);

    VulkanLightBuffer* data = (VulkanLightBuffer*)buffer.Data;
    IDEFER(allocator->Free(data));

    m_graphicsEngine->m_spotLights.Erase(a_addr);
}
void VulkanGraphicsEngineBindings::SetSpotLightShadowMap(uint32_t a_addr, uint32_t a_shadowMapAddr) const
{
    IVERIFY(m_graphicsEngine->m_spotLights.Exists(a_addr));

    BlockAllocator* allocator = m_graphicsEngine->m_vulkanEngine->GetBlockAllocator();

    TLockArray<SpotLightBuffer> a = m_graphicsEngine->m_spotLights.ToLockArray();

    const SpotLightBuffer& buffer = a[a_addr];
    IVERIFY(buffer.Data != nullptr);

    VulkanLightBuffer* lightBuffer = (VulkanLightBuffer*)buffer.Data;

    uint32_t* renderTextures = lightBuffer->LightRenderTextures;

    if (a_shadowMapAddr != uint32_t(-1))
    {
        IVERIFY(m_graphicsEngine->m_depthRenderTextures.Exists(a_shadowMapAddr));

        if (renderTextures == nullptr)
        {
            renderTextures = allocator->TAllocate<uint32_t>(1);
        }

        renderTextures[0] = a_shadowMapAddr;

        lightBuffer->LightRenderTextures = renderTextures;
        lightBuffer->LightRenderTextureCount = 1;
    }
    else if (renderTextures != nullptr)
    {
        lightBuffer->LightRenderTextures = nullptr;
        lightBuffer->LightRenderTextureCount = 0;

        allocator->Free(renderTextures);
    }
}
uint32_t VulkanGraphicsEngineBindings::GetSpotLightShadowMap(uint32_t a_addr) const
{
    IVERIFY(m_graphicsEngine->m_spotLights.Exists(a_addr));

    const TReadLockArray<SpotLightBuffer> a = m_graphicsEngine->m_spotLights.ToReadLockArray();

    const SpotLightBuffer& buffer = a[a_addr];
    IVERIFY(buffer.Data != nullptr);

    const VulkanLightBuffer* lightBuffer = (VulkanLightBuffer*)buffer.Data;
    if (lightBuffer->LightRenderTextureCount == 0)
    {
        return -1;
    }

    return lightBuffer->LightRenderTextures[0];
}

uint32_t VulkanGraphicsEngineBindings::GenerateCanvasRenderer() const
{
    const CanvasRendererBuffer canvas = 
    {
        .CanvasAddr = uint32_t(-1),
        .RenderTextureAddr = uint32_t(-1)
    };

    return m_graphicsEngine->m_canvasRenderers.PushVal(canvas);
}
void VulkanGraphicsEngineBindings::DestroyCanvasRenderer(uint32_t a_addr) const
{
    IVERIFY(m_graphicsEngine->m_canvasRenderers.Exists(a_addr));

    m_graphicsEngine->m_canvasRenderers.Erase(a_addr);
}
void VulkanGraphicsEngineBindings::SetCanvasRendererCanvas(uint32_t a_addr, uint32_t a_canvasAddr) const
{
    IVERIFY(m_graphicsEngine->m_canvasRenderers.Exists(a_addr));
    
    TLockArray<CanvasRendererBuffer> a = m_graphicsEngine->m_canvasRenderers.ToLockArray();
    a[a_addr].CanvasAddr = a_canvasAddr;
}
uint32_t VulkanGraphicsEngineBindings::GetCanvasRendererCanvas(uint32_t a_addr) const
{
    IVERIFY(m_graphicsEngine->m_canvasRenderers.Exists(a_addr));

    return m_graphicsEngine->m_canvasRenderers[a_addr].CanvasAddr;
}
void VulkanGraphicsEngineBindings::SetCanvasRendererRenderTexture(uint32_t a_addr, uint32_t a_renderTextureAddr) const
{
    IVERIFY(m_graphicsEngine->m_canvasRenderers.Exists(a_addr));
    if (a_renderTextureAddr != uint32_t(-1))
    {
        IVERIFY(m_graphicsEngine->m_renderTextures.Exists(a_renderTextureAddr));
    }

    TLockArray<CanvasRendererBuffer> a = m_graphicsEngine->m_canvasRenderers.ToLockArray();
    a[a_addr].RenderTextureAddr = a_renderTextureAddr;
}
uint32_t VulkanGraphicsEngineBindings::GetCanvasRendererRenderTexture(uint32_t a_addr) const
{
    IVERIFY(m_graphicsEngine->m_canvasRenderers.Exists(a_addr));

    return m_graphicsEngine->m_canvasRenderers[a_addr].RenderTextureAddr;
}

void VulkanGraphicsEngineBindings::BindMaterial(uint32_t a_addr) const
{
    IVERIFY(m_graphicsEngine->m_renderCommands.Exists());
    if (a_addr != uint32_t(-1))
    {
        IVERIFY(m_graphicsEngine->m_shaderPrograms.Exists(a_addr));
    }

    m_graphicsEngine->m_renderCommands->BindMaterial(a_addr);
}
void VulkanGraphicsEngineBindings::PushTexture(uint32_t a_slot, uint32_t a_samplerAddr) const
{
    IVERIFY(m_graphicsEngine->m_renderCommands.Exists());
    IVERIFY(m_graphicsEngine->m_textureSampler.Exists(a_samplerAddr));

    const TReadLockArray<TextureSamplerBuffer> a = m_graphicsEngine->m_textureSampler.ToReadLockArray();
    m_graphicsEngine->m_renderCommands->PushTexture(a_slot, a[a_samplerAddr]);
}
void VulkanGraphicsEngineBindings::PushLight(uint32_t a_slot, e_LightType a_lightType, uint32_t a_lightAddr) const
{
    IVERIFY(m_graphicsEngine->m_renderCommands.Exists());

    m_graphicsEngine->m_renderCommands->PushLight(a_slot, a_lightType, a_lightAddr);
}
void VulkanGraphicsEngineBindings::PushLightSplits(uint32_t a_slot, const LightShadowSplit* a_splits, uint32_t a_splitCount) const
{
    IVERIFY(m_graphicsEngine->m_renderCommands.Exists());

    m_graphicsEngine->m_renderCommands->PushLightSplits(a_slot, a_splits, a_splitCount);
}
void VulkanGraphicsEngineBindings::PushShadowTextureArray(uint32_t a_slot, uint32_t a_dirLightAddr) const
{
    IVERIFY(m_graphicsEngine->m_renderCommands.Exists());

    m_graphicsEngine->m_renderCommands->PushShadowTextureArray(a_slot, a_dirLightAddr);
}
void VulkanGraphicsEngineBindings::BindRenderTexture(uint32_t a_addr, e_RenderTextureBindMode a_bindMode) const
{
    IVERIFY(m_graphicsEngine->m_renderCommands.Exists());

    if (a_addr != uint32_t(-1))
    {
        IVERIFY(m_graphicsEngine->m_renderTextures.Exists(a_addr));
    }

    m_graphicsEngine->m_renderCommands->BindRenderTexture(a_addr, a_bindMode);
}
void VulkanGraphicsEngineBindings::BlitRTRT(uint32_t a_srcAddr, uint32_t a_dstAddr) const
{
    IVERIFY(m_graphicsEngine->m_renderCommands.Exists());

    IVERIFY(m_graphicsEngine->m_renderTextures.Exists(a_srcAddr));
    const VulkanRenderTexture* srcTex = m_graphicsEngine->m_renderTextures[a_srcAddr];

    VulkanRenderTexture* dstTex = nullptr;
    if (a_dstAddr != uint32_t(-1))
    {
        IVERIFY(m_graphicsEngine->m_renderTextures.Exists(a_dstAddr));
        dstTex = m_graphicsEngine->m_renderTextures[a_dstAddr];
    }

    m_graphicsEngine->m_renderCommands->Blit(srcTex, dstTex);
}
void VulkanGraphicsEngineBindings::BlitMTRT(uint32_t a_srcAddr, uint32_t a_index, uint32_t a_dstAddr) const
{
    IVERIFY(m_graphicsEngine->m_renderCommands.Exists());

    IVERIFY(m_graphicsEngine->m_renderTextures.Exists(a_srcAddr));
    const VulkanRenderTexture* srcTex = m_graphicsEngine->m_renderTextures[a_srcAddr];

    VulkanRenderTexture* dstTex = nullptr;
    if (a_dstAddr != uint32_t(-1))
    {
        IVERIFY(m_graphicsEngine->m_renderTextures.Exists(a_dstAddr));
        dstTex = m_graphicsEngine->m_renderTextures[a_dstAddr];
    }

    m_graphicsEngine->m_renderCommands->Blit(srcTex, a_index, dstTex);
}
void VulkanGraphicsEngineBindings::DrawMaterial() const 
{
    IVERIFY(m_graphicsEngine->m_renderCommands.Exists());

    m_graphicsEngine->m_renderCommands->DrawMaterial();
}
void VulkanGraphicsEngineBindings::DrawModel(const glm::mat4& a_transform, uint32_t a_addr) const
{
    IVERIFY(m_graphicsEngine->m_renderCommands.Exists());

    m_graphicsEngine->m_renderCommands->DrawModel(a_transform, a_addr);
}
void VulkanGraphicsEngineBindings::MarkerStart(const std::string_view& a_name) const
{
    IVERIFY(m_graphicsEngine->m_renderCommands.Exists());

    m_graphicsEngine->m_renderCommands->MarkerStart(a_name);
}
void VulkanGraphicsEngineBindings::MarkerEnd() const
{
    IVERIFY(m_graphicsEngine->m_renderCommands.Exists());

    m_graphicsEngine->m_renderCommands->MarkerEnd();
}

void VulkanGraphicsEngineBindings::SetLightSplits(const LightShadowSplit* a_splits, uint32_t a_splitCount) const
{
    IVERIFY(m_graphicsEngine->m_lightData.Exists());

    m_graphicsEngine->m_lightData->SetLightSplits(a_splits, a_splitCount);
}

#endif

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
