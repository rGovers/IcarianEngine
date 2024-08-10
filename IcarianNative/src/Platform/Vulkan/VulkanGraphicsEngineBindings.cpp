#ifdef ICARIANNATIVE_ENABLE_GRAPHICS_VULKAN

#include "Rendering/Vulkan/VulkanGraphicsEngineBindings.h"

#include "Core/IcarianAssert.h"
#include "Core/IcarianDefer.h"
#include "Core/StringUtils.h"
#include "DeletionQueue.h"
#include "FileCache.h"
#include "ObjectManager.h"
#include "Rendering/RenderEngine.h"
#include "Rendering/ShaderTable.h"
#include "Rendering/Vulkan/VulkanDepthCubeRenderTexture.h"
#include "Rendering/Vulkan/VulkanDepthRenderTexture.h"
#include "Rendering/Vulkan/VulkanGraphicsEngine.h"
#include "Rendering/Vulkan/VulkanGraphicsParticle2D.h"
#include "Rendering/Vulkan/VulkanLightBuffer.h"
#include "Rendering/Vulkan/VulkanLightData.h"
#include "Rendering/Vulkan/VulkanModel.h"
#include "Rendering/Vulkan/VulkanRenderCommand.h"
#include "Rendering/Vulkan/VulkanRenderEngineBackend.h"
#include "Rendering/Vulkan/VulkanRenderTexture.h"
#include "Rendering/Vulkan/VulkanShaderData.h"
#include "Rendering/Vulkan/VulkanTextureSampler.h"
#include "Rendering/Vulkan/VulkanVideoTexture.h"
#include "Runtime/RuntimeManager.h"
#include "Trace.h"

#ifdef WIN32
#include "Core/WindowsHeaders.h"
#endif

static VulkanGraphicsEngineBindings* Instance = nullptr;

// The lazy part of me won against the part that wants to write clean code
// My apologies to the poor soul that has to decipher this definition
#define VULKANGRAPHICS_BINDING_FUNCTION_TABLE(F) \
    F(void, IcarianEngine.Rendering, VertexShader, DestroyShader, { Instance->DestroyVertexShader(a_addr); }, uint32_t a_addr) \
    F(void, IcarianEngine.Rendering, PixelShader, DestroyShader, { Instance->DestroyPixelShader(a_addr); }, uint32_t a_addr) \
    \
    F(RenderProgram, IcarianEngine.Rendering, Material, GetProgramBuffer, { return Instance->GetRenderProgram(a_addr); }, uint32_t a_addr) \
    F(void, IcarianEngine.Rendering, Material, SetProgramBuffer, { Instance->SetRenderProgram(a_addr, a_program); }, uint32_t a_addr, RenderProgram a_program) \
    F(void, IcarianEngine.Rendering, Material, SetTexture, { Instance->RenderProgramSetTexture(a_addr, a_shaderSlot, a_samplerAddr); }, uint32_t a_addr, uint32_t a_shaderSlot, uint32_t a_samplerAddr) \
    F(void, IcarianEngine.Rendering, Material, SetUserUniform, { Instance->RenderProgramSetUserUBO(a_addr, a_uboSize, a_uboData); }, uint32_t a_addr, uint32_t a_uboSize, void* a_uboData) \
    \
    F(uint32_t, IcarianEngine.Rendering, Camera, GenerateBuffer, { return Instance->GenerateCameraBuffer(a_transformAddr); }, uint32_t a_transformAddr) \
    F(void, IcarianEngine.Rendering, Camera, DestroyBuffer, { Instance->DestroyCameraBuffer(a_addr); }, uint32_t a_addr) \
    F(CameraBuffer, IcarianEngine.Rendering, Camera, GetBuffer, { return Instance->GetCameraBuffer(a_addr); }, uint32_t a_addr) \
    F(void, IcarianEngine.Rendering, Camera, SetBuffer, { Instance->SetCameraBuffer(a_addr, a_buffer); }, uint32_t a_addr, CameraBuffer a_buffer) \
    F(glm::vec3, IcarianEngine.Rendering, Camera, ScreenToWorld, { return Instance->CameraScreenToWorld(a_addr, a_screenPos, a_screenSize); }, uint32_t a_addr, glm::vec3 a_screenPos, glm::vec2 a_screenSize) \
    \
    F(uint32_t, IcarianEngine.Rendering, MeshRenderer, GenerateBuffer, { return Instance->GenerateMeshRenderBuffer(a_materialAddr, a_modelAddr, a_transformAddr); }, uint32_t a_transformAddr, uint32_t a_materialAddr, uint32_t a_modelAddr) \
    F(void, IcarianEngine.Rendering, MeshRenderer, DestroyBuffer, { Instance->DestroyMeshRenderBuffer(a_addr); }, uint32_t a_addr) \
    F(void, IcarianEngine.Rendering, MeshRenderer, GenerateRenderStack, { Instance->GenerateRenderStack(a_addr); }, uint32_t a_addr) \
    F(void, IcarianEngine.Rendering, MeshRenderer, DestroyRenderStack, { Instance->DestroyRenderStack(a_addr); }, uint32_t a_addr) \
    \
    F(uint32_t, IcarianEngine.Rendering.Animation, SkinnedMeshRenderer, GenerateBuffer, { return Instance->GenerateSkinnedMeshRenderBuffer(a_materialAddr, a_modelAddr, a_transformAddr, a_skeletonAddr); }, uint32_t a_transformAddr, uint32_t a_materialAddr, uint32_t a_modelAddr, uint32_t a_skeletonAddr) \
    F(void, IcarianEngine.Rendering.Animation, SkinnedMeshRenderer, DestroyBuffer, { Instance->DestroySkinnedMeshRenderBuffer(a_addr); }, uint32_t a_addr) \
    F(void, IcarianEngine.Rendering.Animation, SkinnedMeshRenderer, GenerateRenderStack, { Instance->GenerateSkinnedRenderStack(a_addr); }, uint32_t a_addr) \
    F(void, IcarianEngine.Rendering.Animation, SkinnedMeshRenderer, DestroyRenderStack, { Instance->DestroySkinnedRenderStack(a_addr); }, uint32_t a_addr) \
    \
    F(void, IcarianEngine.Rendering, Texture, DestroyTexture, { Instance->DestroyTexture(a_addr); }, uint32_t a_addr) \
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
    F(void, IcarianEngine.Rendering, Model, DestroyModel, { IPUSHDELETIONFUNC({ Instance->DestroyModel(a_addr); }, DeletionIndex_Render); }, uint32_t a_addr) \
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
    F(void, IcarianEngine.Rendering.Lighting, DirectionalLight, DestroyBuffer, { Instance->DestroyDirectionalLightBuffer(a_addr); }, uint32_t a_addr) \
    F(DirectionalLightBuffer, IcarianEngine.Rendering.Lighting, DirectionalLight, GetBuffer, { return Instance->GetDirectionalLightBuffer(a_addr); }, uint32_t a_addr) \
    F(void, IcarianEngine.Rendering.Lighting, DirectionalLight, SetBuffer, { Instance->SetDirectionalLightBuffer(a_addr, a_buffer); }, uint32_t a_addr, DirectionalLightBuffer a_buffer) \
    F(void, IcarianEngine.Rendering.Lighting, DirectionalLight, AddShadowMap, { Instance->AddDirectionalLightShadowMap(a_addr, a_shadowMapAddr); }, uint32_t a_addr, uint32_t a_shadowMapAddr) \
    F(void, IcarianEngine.Rendering.Lighting, DirectionalLight, RemoveShadowMap, { Instance->RemoveDirectionalLightShadowMap(a_addr, a_shadowMapAddr); }, uint32_t a_addr, uint32_t a_shadowMapAddr) \
    \
    F(uint32_t, IcarianEngine.Rendering.Lighting, PointLight, GenerateBuffer, { return Instance->GeneratePointLightBuffer(a_transformAddr); }, uint32_t a_transformAddr) \
    F(void, IcarianEngine.Rendering.Lighting, PointLight, DestroyBuffer, { Instance->DestroyPointLightBuffer(a_addr); }, uint32_t a_addr) \
    F(PointLightBuffer, IcarianEngine.Rendering.Lighting, PointLight, GetBuffer, { return Instance->GetPointLightBuffer(a_addr); }, uint32_t a_addr) \
    F(void, IcarianEngine.Rendering.Lighting, PointLight, SetBuffer, { Instance->SetPointLightBuffer(a_addr, a_buffer); }, uint32_t a_addr, PointLightBuffer a_buffer) \
    F(uint32_t, IcarianEngine.Rendering.Lighting, PointLight, GetShadowMap, { return Instance->GetPointLightShadowMap(a_addr); }, uint32_t a_addr) \
    F(void, IcarianEngine.Rendering.Lighting, PointLight, SetShadowMap, { Instance->SetPointLightShadowMap(a_addr, a_shadowMapAddr); }, uint32_t a_addr, uint32_t a_shadowMapAddr) \
    \
    F(uint32_t, IcarianEngine.Rendering.Lighting, SpotLight, GenerateBuffer, { return Instance->GenerateSpotLightBuffer(a_transformAddr); }, uint32_t a_transformAddr) \
    F(void, IcarianEngine.Rendering.Lighting, SpotLight, DestroyBuffer, { Instance->DestroySpotLightBuffer(a_addr); }, uint32_t a_addr) \
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
    F(void, IcarianEngine.Rendering, RenderCommand, DrawMaterial, { Instance->DrawMaterial(); }) \
    F(void, IcarianEngine.Rendering, RenderCommand, DrawModel, { Instance->DrawModel(a_transform, a_addr); }, glm::mat4 a_transform, uint32_t a_addr) \
    \
    F(void, IcarianEngine.Rendering.Animation, SkeletonAnimator, PushTransform, { }, uint32_t a_addr, MonoString* a_object, MonoArray* a_transform) \

VULKANGRAPHICS_BINDING_FUNCTION_TABLE(RUNTIME_FUNCTION_DEFINITION);

RUNTIME_FUNCTION(uint32_t, VertexShader, GenerateFromFile, 
{
    char* str = mono_string_to_utf8(a_path);
    IDEFER(mono_free(str));

    // Faster to do the comparison if it is a single comparison
    if (strncmp(str, INTERNALSHADERPATHSTR, InternalShaderStringSize) == 0)
    {
        const char* shader = GetVertexShaderString(str + InternalShaderStringSize);

        if (shader != nullptr)
        {
            return Instance->GenerateFVertexShaderAddr(shader);
        }
        else
        {
            IWARN(std::string("Failed to find internal VertexShader: ") + str);
        }
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
            FileHandle* handle = FileCache::LoadFile(p);
            if (handle != nullptr)
            {
                IDEFER(delete handle);

                const uint64_t size = handle->GetSize();

                char* str = new char[size];
                IDEFER(delete[] str);

                handle->Read(str, size);

                return Instance->GenerateFVertexShaderAddr(std::string_view(str, size));
            }
            
            IWARN(std::string("VertexShader failed to open: ") + str);

            break;
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
RUNTIME_FUNCTION(uint32_t, PixelShader, GenerateFromFile, 
{
    char* str = mono_string_to_utf8(a_path);
    IDEFER(mono_free(str));

    // Faster to do the comparison if it is a single comparison
    if (strncmp(str, INTERNALSHADERPATHSTR, InternalShaderStringSize) == 0)
    {
        const char* shader = GetPixelShaderString(str + InternalShaderStringSize);

        if (shader != nullptr)
        {
            return Instance->GenerateFPixelShaderAddr(shader);
        }
        else
        {
            IWARN(std::string("Failed to find internal PixelShader: ") + str);
        }
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
            FileHandle* handle = FileCache::LoadFile(p);
            if (handle != nullptr)
            {
                IDEFER(delete handle);

                const uint64_t size = handle->GetSize();

                char* str = new char[size];
                IDEFER(delete[] str);

                handle->Read(str, size);

                return Instance->GenerateFPixelShaderAddr(std::string_view(str, size));
            }

            IWARN(std::string("PixelShader failed to open: ") + str);

            break;
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

    return Instance->GenerateShaderProgram(program);
}, uint32_t a_vertexShader, uint32_t a_pixelShader, uint16_t a_vertexStride, MonoArray* a_vertexInputAttribs, uint32_t a_cullMode, uint32_t a_primitiveMode, uint32_t a_colorBlendMode, uint32_t a_renderLayer, uint32_t a_shadowVertexShader, uint32_t a_uboSize, void* a_uboData)
RUNTIME_FUNCTION(void, Material, DestroyProgram, 
{
    const RenderProgram program = Instance->GetRenderProgram(a_addr);

    IDEFER(
    if (program.VertexAttributes != nullptr)
    {
        delete[] program.VertexAttributes;
    }
    
    if (program.UBOData != NULL)
    {
        free(program.UBOData);
    });

    Instance->DestroyShaderProgram(a_addr);
}, uint32_t a_addr)

// MSVC workaround
static uint32_t M_Model_GenerateModel(MonoArray* a_vertices, MonoArray* a_indices, uint16_t a_vertexStride, float a_radius)
{
    const uint32_t vertexCount = (uint32_t)mono_array_length(a_vertices);
    const uint32_t indexCount = (uint32_t)mono_array_length(a_indices);

    const uint32_t vertexSize = vertexCount * a_vertexStride;

    char* vertices = new char[vertexSize];
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
}
RUNTIME_FUNCTION(uint32_t, Model, GenerateModel,
{
    return M_Model_GenerateModel(a_vertices, a_indices, a_vertexStride, a_radius);
}, MonoArray* a_vertices, MonoArray* a_indices, uint16_t a_vertexStride, float a_radius);

RUNTIME_FUNCTION(void, RenderPipeline, SetLightSplits, 
{
    const uint32_t lightSplitCount = (uint32_t)mono_array_length(a_lightSplits);

    LightShadowSplit* lightSplits = new LightShadowSplit[lightSplitCount];
    IDEFER(delete[] lightSplits);

    for (uint32_t i = 0; i < lightSplitCount; ++i)
    {
        lightSplits[i] = mono_array_get(a_lightSplits, LightShadowSplit, i);
    }

    Instance->SetLightSplits(lightSplits, lightSplitCount);
}, MonoArray* a_lightSplits)

RUNTIME_FUNCTION(void, RenderCommand, PushShadowSplits, 
{
    const uint32_t lightSplitCount = (uint32_t)mono_array_length(a_splits);

    LightShadowSplit* lightSplits = new LightShadowSplit[lightSplitCount];
    IDEFER(delete[] lightSplits);

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

    BIND_FUNCTION(IcarianEngine.Rendering, VertexShader, GenerateFromFile);
    BIND_FUNCTION(IcarianEngine.Rendering, PixelShader, GenerateFromFile);

    BIND_FUNCTION(IcarianEngine.Rendering, Camera, GetProjectionMatrix);
    BIND_FUNCTION(IcarianEngine.Rendering, Camera, GetProjectionMatrixNF);

    BIND_FUNCTION(IcarianEngine.Rendering.Lighting, DirectionalLight, GetShadowMaps);

    BIND_FUNCTION(IcarianEngine.Rendering, Material, GenerateProgram);
    BIND_FUNCTION(IcarianEngine.Rendering, Material, DestroyProgram);

    BIND_FUNCTION(IcarianEngine.Rendering, Model, GenerateModel);

    BIND_FUNCTION(IcarianEngine.Rendering, RenderCommand, PushShadowSplits);

    BIND_FUNCTION(IcarianEngine.Rendering, RenderPipeline, SetLightSplits);
}
VulkanGraphicsEngineBindings::~VulkanGraphicsEngineBindings()
{

}

uint32_t VulkanGraphicsEngineBindings::GenerateFVertexShaderAddr(const std::string_view& a_str) const
{
    return m_graphicsEngine->GenerateFVertexShader(a_str);
}
void VulkanGraphicsEngineBindings::DestroyVertexShader(uint32_t a_addr) const
{
    m_graphicsEngine->DestroyVertexShader(a_addr);
}

uint32_t VulkanGraphicsEngineBindings::GenerateFPixelShaderAddr(const std::string_view& a_str) const
{
    return m_graphicsEngine->GenerateFPixelShader(a_str);
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
    ICARIAN_ASSERT_MSG(a_addr < m_graphicsEngine->m_shaderPrograms.Size(), "RenderProgramSetTexture material out of bounds");
    ICARIAN_ASSERT_MSG(a_samplerAddr < m_graphicsEngine->m_textureSampler.Size(), "RenderProgramSetTexture sampler out of bounds");

    TLockArray<RenderProgram> a = m_graphicsEngine->m_shaderPrograms.ToLockArray();

    const RenderProgram& program = a[a_addr];
    IVERIFY(program.Data != nullptr);

    VulkanShaderData* data = (VulkanShaderData*)program.Data;
    data->SetTexture(a_shaderSlot, a_samplerAddr);
}
void VulkanGraphicsEngineBindings::RenderProgramSetUserUBO(uint32_t a_addr, uint32_t a_uboSize, const void* a_uboData) const
{
    ICARIAN_ASSERT_MSG(a_addr < m_graphicsEngine->m_shaderPrograms.Size(), "RenderProgramSetUserUBO material out of bounds");

    TLockArray<RenderProgram> a = m_graphicsEngine->m_shaderPrograms.ToLockArray();
    
    RenderProgram& program = a[a_addr];

    if (program.UBOData != NULL)
    {
        ICARIAN_ASSERT(program.UBODataSize == a_uboSize);
        memcpy(program.UBOData, a_uboData, a_uboSize);

        return;
    }

    program.UBODataSize = a_uboSize;

    if (a_uboData != NULL && a_uboSize > 0)
    {
        program.UBOData = malloc((size_t)a_uboSize);

        memcpy(program.UBOData, a_uboData, (size_t)a_uboSize);
    }
    else
    {
        program.UBOData = NULL;
    }
}
RenderProgram VulkanGraphicsEngineBindings::GetRenderProgram(uint32_t a_addr) const
{
    return m_graphicsEngine->GetRenderProgram(a_addr);
}
void VulkanGraphicsEngineBindings::SetRenderProgram(uint32_t a_addr, const RenderProgram& a_program) const
{
    ICARIAN_ASSERT_MSG(a_addr < m_graphicsEngine->m_shaderPrograms.Size(), "SetRenderProgram out of bounds");
    ICARIAN_ASSERT_MSG(m_graphicsEngine->m_shaderPrograms.Exists(a_addr), "SetRenderProgram invalid address");

    m_graphicsEngine->m_shaderPrograms.LockSet(a_addr, a_program);
}

uint32_t VulkanGraphicsEngineBindings::GenerateCameraBuffer(uint32_t a_transformAddr) const
{
    IVERIFY(a_transformAddr != -1);

    const CameraBuffer buff = CameraBuffer(a_transformAddr);

    uint32_t size = 0;
    {
        TRACE("Getting Camera Buffer");
        TLockArray<CameraBuffer> a = m_graphicsEngine->m_cameraBuffers.ToLockArray();

        size = a.Size();
        for (uint32_t i = 0; i < size; ++i)
        {
            if (a[i].TransformAddr == -1)
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
    IVERIFY(m_graphicsEngine->m_cameraBuffers[a_addr].TransformAddr != -1);

    return m_graphicsEngine->m_cameraBuffers[a_addr];
}
void VulkanGraphicsEngineBindings::SetCameraBuffer(uint32_t a_addr, const CameraBuffer& a_buffer) const
{
    IVERIFY(a_addr < m_graphicsEngine->m_cameraBuffers.Size());
    IVERIFY(m_graphicsEngine->m_cameraBuffers[a_addr].TransformAddr != -1);

    m_graphicsEngine->m_cameraBuffers.LockSet(a_addr, a_buffer);
}
glm::vec3 VulkanGraphicsEngineBindings::CameraScreenToWorld(uint32_t a_addr, const glm::vec3& a_screenPos, const glm::vec2& a_screenSize) const
{
    IVERIFY(a_addr < m_graphicsEngine->m_cameraBuffers.Size());

    const CameraBuffer camBuf = m_graphicsEngine->m_cameraBuffers[a_addr];

    IVERIFY(camBuf.TransformAddr != -1);

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
    IVERIFY(m_graphicsEngine->m_cameraBuffers[a_addr].TransformAddr != -1);

    const CameraBuffer camBuf = m_graphicsEngine->m_cameraBuffers[a_addr];

    return camBuf.ToProjection(glm::vec2((float)a_width, (float)a_height));
}
glm::mat4 VulkanGraphicsEngineBindings::GetCameraProjectionMatrix(uint32_t a_addr, uint32_t a_width, uint32_t a_height, float a_near, float a_far) const
{
    IVERIFY(a_addr < m_graphicsEngine->m_cameraBuffers.Size());
    IVERIFY(m_graphicsEngine->m_cameraBuffers[a_addr].TransformAddr != -1);

    const CameraBuffer camBuf = m_graphicsEngine->m_cameraBuffers[a_addr];

    return camBuf.ToProjection(glm::vec2((float)a_width, (float)a_height), a_near, a_far);
}

uint32_t VulkanGraphicsEngineBindings::GenerateModel(const void* a_vertices, uint32_t a_vertexCount, const uint32_t* a_indices, uint32_t a_indexCount, uint16_t a_vertexStride, float a_radius) const
{
    return m_graphicsEngine->GenerateModel(a_vertices, a_vertexCount, a_vertexStride, a_indices, a_indexCount, a_radius);
}
void VulkanGraphicsEngineBindings::DestroyModel(uint32_t a_addr) const
{
    IPUSHDELETIONFUNC(
    {
        m_graphicsEngine->DestroyModel(a_addr);    
    }, DeletionIndex_Render);
}

uint32_t VulkanGraphicsEngineBindings::GenerateMeshRenderBuffer(uint32_t a_materialAddr, uint32_t a_modelAddr, uint32_t a_transformAddr) const
{
    const MeshRenderBuffer buffer = MeshRenderBuffer(a_materialAddr, a_modelAddr, a_transformAddr);

    return m_graphicsEngine->m_renderBuffers.PushVal(buffer);
}
void VulkanGraphicsEngineBindings::DestroyMeshRenderBuffer(uint32_t a_addr) const
{
    IPUSHDELETIONFUNC(
    {
        IVERIFY(a_addr < m_graphicsEngine->m_renderBuffers.Size());
        IVERIFY(m_graphicsEngine->m_renderBuffers.Exists(a_addr));

        m_graphicsEngine->m_renderBuffers.Erase(a_addr);
    }, DeletionIndex_Render);
}
void VulkanGraphicsEngineBindings::GenerateRenderStack(uint32_t a_meshAddr) const
{
    ICARIAN_ASSERT_MSG(a_meshAddr < m_graphicsEngine->m_renderBuffers.Size(), "GenerateRenderStack out of bounds");
    ICARIAN_ASSERT_MSG(m_graphicsEngine->m_renderBuffers.Exists(a_meshAddr), "GenerateRenderStack renderer is destroyed");

    TLockArray<MeshRenderBuffer> aBuffer = m_graphicsEngine->m_renderBuffers.ToLockArray();
    const MeshRenderBuffer& buffer = aBuffer[a_meshAddr];

    {
        TLockArray<MaterialRenderStack*> a = m_graphicsEngine->m_renderStacks.ToLockArray();

        const uint32_t size = a.Size();
        for (uint32_t i = 0; i < size; ++i)
        {
            if (a[i]->Add(buffer))
            {
                return;
            }
        }
    }
    
    TRACE("Allocating RenderStack");
    m_graphicsEngine->m_renderStacks.Push(new MaterialRenderStack(buffer));
}
void VulkanGraphicsEngineBindings::DestroyRenderStack(uint32_t a_meshAddr) const
{
    IPUSHDELETIONFUNC(
    {
        IVERIFY(a_meshAddr < m_graphicsEngine->m_renderBuffers.Size());
        IVERIFY(m_graphicsEngine->m_renderBuffers.Exists(a_meshAddr));

        const MeshRenderBuffer buffer = m_graphicsEngine->m_renderBuffers[a_meshAddr];

        TLockArray<MaterialRenderStack*> a = m_graphicsEngine->m_renderStacks.ToLockArray();

        const uint32_t size = a.Size();
        for (uint32_t i = 0; i < size; ++i)
        {
            MaterialRenderStack* stack = a[i];

            if (stack->Remove(buffer)) 
            {
                if (stack->Empty()) 
                {
                    IDEFER(delete stack);

                    TRACE("Destroying RenderStack");
                    m_graphicsEngine->m_renderStacks.UErase(i);
                }

                return;
            }
        }
    }, DeletionIndex_Render);
}

uint32_t VulkanGraphicsEngineBindings::GenerateSkinnedMeshRenderBuffer(uint32_t a_materialAddr, uint32_t a_modelAddr, uint32_t a_transformAddr, uint32_t a_skeletonAddr) const
{   
    TRACE("Creating Skinned Render Buffer");
    const SkinnedMeshRenderBuffer buffer = SkinnedMeshRenderBuffer(a_skeletonAddr, a_materialAddr, a_modelAddr, a_transformAddr);

    return m_graphicsEngine->m_skinnedRenderBuffers.PushVal(buffer);
}
void VulkanGraphicsEngineBindings::DestroySkinnedMeshRenderBuffer(uint32_t a_addr) const
{
    IPUSHDELETIONFUNC(
    {
        TRACE("Destroying Skinned Render Buffer");
        IVERIFY(a_addr < m_graphicsEngine->m_skinnedRenderBuffers.Size());
        IVERIFY(m_graphicsEngine->m_skinnedRenderBuffers.Exists(a_addr));

        m_graphicsEngine->m_skinnedRenderBuffers.Erase(a_addr);
    }, DeletionIndex_Render);
}
void VulkanGraphicsEngineBindings::GenerateSkinnedRenderStack(uint32_t a_addr) const
{
    ICARIAN_ASSERT_MSG(a_addr < m_graphicsEngine->m_skinnedRenderBuffers.Size(), "GenerateSkinnedRenderStack out of bounds");

    TRACE("Pushing Skinned RenderStack");
    const SkinnedMeshRenderBuffer& buffer = m_graphicsEngine->m_skinnedRenderBuffers[a_addr];

    {
        TLockArray<MaterialRenderStack*> a = m_graphicsEngine->m_renderStacks.ToLockArray();

        const uint32_t size = a.Size();

        for (uint32_t i = 0; i < size; ++i)
        {
            if (a[i]->Add(buffer))
            {
                return;
            }
        }
    }

    TRACE("Allocating Skinned RenderStack");
    m_graphicsEngine->m_renderStacks.Push(new MaterialRenderStack(buffer));
}
void VulkanGraphicsEngineBindings::DestroySkinnedRenderStack(uint32_t a_addr) const
{
    IPUSHDELETIONFUNC(
    {   
        TRACE("Removing Skinned RenderStack");
        IVERIFY(a_addr < m_graphicsEngine->m_skinnedRenderBuffers.Size());
        IVERIFY(m_graphicsEngine->m_skinnedRenderBuffers.Exists(a_addr));

        const SkinnedMeshRenderBuffer buffer = m_graphicsEngine->m_skinnedRenderBuffers[a_addr];

        TLockArray<MaterialRenderStack*> a = m_graphicsEngine->m_renderStacks.ToLockArray();

        const uint32_t size = a.Size();
        for (uint32_t i = 0; i < size; ++i)
        {
            if (a[i]->Remove(buffer))
            {
                if (a[i]->Empty())
                {
                    const MaterialRenderStack* stack = a[i];
                    IDEFER(delete stack);

                    TRACE("Destroying Skinned RenderStack");
                    m_graphicsEngine->m_renderStacks.UErase(i);
                }

                return;
            }
        }
    }, DeletionIndex_Render);
}

uint32_t VulkanGraphicsEngineBindings::GenerateGraphicsParticle2D(uint32_t a_computeBufferAddr) const
{
    VulkanGraphicsParticle2D* particleSystem = new VulkanGraphicsParticle2D(m_graphicsEngine->m_vulkanEngine, m_graphicsEngine->m_vulkanEngine->GetComputeEngine(), m_graphicsEngine, a_computeBufferAddr);

    return m_graphicsEngine->m_particleEmitters.PushVal(particleSystem);   
}
void VulkanGraphicsEngineBindings::DestroyGraphicsParticle2D(uint32_t a_addr) const
{
    IVERIFY(a_addr < m_graphicsEngine->m_particleEmitters.Size());
    IVERIFY(m_graphicsEngine->m_particleEmitters.Exists(a_addr));

    const VulkanGraphicsParticle2D* particleSystem = m_graphicsEngine->m_particleEmitters[a_addr];
    IDEFER(delete particleSystem);
    m_graphicsEngine->m_particleEmitters.Erase(a_addr);
}

void VulkanGraphicsEngineBindings::DestroyTexture(uint32_t a_addr) const
{
    IPUSHDELETIONFUNC(
    {
        m_graphicsEngine->DestroyTexture(a_addr);
    }, DeletionIndex_Render);
}

uint32_t VulkanGraphicsEngineBindings::GenerateVideoTexture(uint32_t a_videoAddr) const
{
    VulkanVideoTexture* texture = new VulkanVideoTexture(m_graphicsEngine->m_vulkanEngine, a_videoAddr);

    return m_graphicsEngine->m_videoTextures.PushVal(texture);
}
void VulkanGraphicsEngineBindings::DestroyVideoTexture(uint32_t a_addr) const
{
    IVERIFY(a_addr < m_graphicsEngine->m_videoTextures.Size());
    IVERIFY(m_graphicsEngine->m_videoTextures.Exists(a_addr));

    const VulkanVideoTexture* texture = m_graphicsEngine->m_videoTextures[a_addr];
    IDEFER(delete texture);
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

    VulkanRenderTexture* texture = new VulkanRenderTexture(m_graphicsEngine->m_vulkanEngine, m_graphicsEngine, a_count, a_width, a_height, a_depthTexture, a_hdr, a_channelCount);

    return m_graphicsEngine->m_renderTextures.PushVal(texture);
}
uint32_t VulkanGraphicsEngineBindings::GenerateRenderTextureD(uint32_t a_count, uint32_t a_width, uint32_t a_height, uint32_t a_depthHandle, bool a_hdr, uint32_t a_channelCount) const
{
    IVERIFY(a_count > 0);
    IVERIFY(a_width > 0);
    IVERIFY(a_height > 0);
    IVERIFY(a_channelCount > 0);
    IVERIFY(a_channelCount <= 4);

    IVERIFY(a_depthHandle < m_graphicsEngine->m_depthRenderTextures.Size());
    IVERIFY(m_graphicsEngine->m_depthRenderTextures.Exists(a_depthHandle));

    VulkanRenderTexture* texture = new VulkanRenderTexture(m_graphicsEngine->m_vulkanEngine, m_graphicsEngine, a_count, a_width, a_height, a_depthHandle, a_hdr, a_channelCount);

    return m_graphicsEngine->m_renderTextures.PushVal(texture);
}
void VulkanGraphicsEngineBindings::DestroyRenderTexture(uint32_t a_addr) const
{
    IVERIFY(a_addr < m_graphicsEngine->m_renderTextures.Size());
    IVERIFY(m_graphicsEngine->m_renderTextures.Exists(a_addr));

    const VulkanRenderTexture* tex = m_graphicsEngine->m_renderTextures[a_addr];
    IDEFER(delete tex);

    m_graphicsEngine->m_renderTextures.Erase(a_addr);
}
uint32_t VulkanGraphicsEngineBindings::GetRenderTextureTextureCount(uint32_t a_addr) const
{
    IVERIFY(a_addr < m_graphicsEngine->m_renderTextures.Size());
    IVERIFY(m_graphicsEngine->m_renderTextures.Exists(a_addr));

    const VulkanRenderTexture* texture = m_graphicsEngine->m_renderTextures[a_addr];

    return texture->GetTextureCount();
}
bool VulkanGraphicsEngineBindings::RenderTextureHasDepth(uint32_t a_addr) const
{
    IVERIFY(a_addr < m_graphicsEngine->m_renderTextures.Size());
    IVERIFY(m_graphicsEngine->m_renderTextures.Exists(a_addr));

    const VulkanRenderTexture* texture = m_graphicsEngine->m_renderTextures[a_addr];

    return texture->HasDepthTexture();
}
uint32_t VulkanGraphicsEngineBindings::GetRenderTextureWidth(uint32_t a_addr) const
{
    IVERIFY(a_addr < m_graphicsEngine->m_renderTextures.Size());
    IVERIFY(m_graphicsEngine->m_renderTextures.Exists(a_addr));

    const VulkanRenderTexture* texture = m_graphicsEngine->m_renderTextures[a_addr];

    return texture->GetWidth();
}
uint32_t VulkanGraphicsEngineBindings::GetRenderTextureHeight(uint32_t a_addr) const
{
    IVERIFY(a_addr < m_graphicsEngine->m_renderTextures.Size());
    IVERIFY(m_graphicsEngine->m_renderTextures.Exists(a_addr));

    const VulkanRenderTexture* texture = m_graphicsEngine->m_renderTextures[a_addr];

    return texture->GetHeight();
}
void VulkanGraphicsEngineBindings::ResizeRenderTexture(uint32_t a_addr, uint32_t a_width, uint32_t a_height) const
{
    IVERIFY(a_addr < m_graphicsEngine->m_renderTextures.Size());
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
    IVERIFY(a_addr < m_graphicsEngine->m_depthRenderTextures.Size());
    IVERIFY(m_graphicsEngine->m_depthRenderTextures.Exists(a_addr));

    const VulkanDepthRenderTexture* texture = m_graphicsEngine->m_depthRenderTextures[a_addr];
    
    return texture->GetWidth();
}
uint32_t VulkanGraphicsEngineBindings::GetDepthRenderTextureHeight(uint32_t a_addr) const
{
    IVERIFY(a_addr < m_graphicsEngine->m_depthRenderTextures.Size());
    IVERIFY(m_graphicsEngine->m_depthRenderTextures.Exists(a_addr));

    const VulkanDepthRenderTexture* texture = m_graphicsEngine->m_depthRenderTextures[a_addr];

    return texture->GetHeight();
}
void VulkanGraphicsEngineBindings::ResizeDepthRenderTexture(uint32_t a_addr, uint32_t a_width, uint32_t a_height) const
{
    IVERIFY(a_addr < m_graphicsEngine->m_depthRenderTextures.Size());
    IVERIFY(m_graphicsEngine->m_depthRenderTextures.Exists(a_addr));
    IVERIFY(a_width > 0);
    IVERIFY(a_height > 0);

    TLockArray<VulkanDepthRenderTexture*> a = m_graphicsEngine->m_depthRenderTextures.ToLockArray();

    VulkanDepthRenderTexture* texture = a[a_addr];

    texture->Resize(a_width, a_height);
}

uint32_t VulkanGraphicsEngineBindings::GenerateDepthCubeRenderTexture(uint32_t a_width, uint32_t a_height) const
{
    ICARIAN_ASSERT_MSG(a_width > 0, "GenerateDepthCubeRenderTexture width 0")
    ICARIAN_ASSERT_MSG(a_height > 0, "GenerateDepthCubeRenderTexture height 0")

    VulkanDepthCubeRenderTexture* texture = new VulkanDepthCubeRenderTexture(m_graphicsEngine->m_vulkanEngine, a_width, a_height);

    return m_graphicsEngine->m_depthCubeRenderTextures.PushVal(texture);
}
void VulkanGraphicsEngineBindings::DestroyDepthCubeRenderTexture(uint32_t a_addr) const
{
    ICARIAN_ASSERT_MSG(a_addr < m_graphicsEngine->m_depthCubeRenderTextures.Size(), "DestroyDepthCubeRenderTexture out of bounds");
    ICARIAN_ASSERT_MSG(m_graphicsEngine->m_depthCubeRenderTextures.Exists(a_addr), "DestroyDepthCubeRenderTexture already destroyed");

    const VulkanDepthCubeRenderTexture* tex = m_graphicsEngine->m_depthCubeRenderTextures[a_addr];
    IDEFER(delete tex);

    m_graphicsEngine->m_depthCubeRenderTextures.Erase(a_addr);
}
uint32_t VulkanGraphicsEngineBindings::GetDepthCubeRenderTextureWidth(uint32_t a_addr) const
{
    ICARIAN_ASSERT_MSG(a_addr < m_graphicsEngine->m_depthCubeRenderTextures.Size(), "GetDepthCubeRenderTextureWidth out of bounds");
    ICARIAN_ASSERT_MSG(m_graphicsEngine->m_depthCubeRenderTextures.Exists(a_addr), "GetDepthCubeRenderTextureWidth already destroyed");

    const VulkanDepthCubeRenderTexture* texture = m_graphicsEngine->m_depthCubeRenderTextures[a_addr];

    return texture->GetWidth();
}
uint32_t VulkanGraphicsEngineBindings::GetDepthCubeRenderTextureHeight(uint32_t a_addr) const
{
    ICARIAN_ASSERT_MSG(a_addr < m_graphicsEngine->m_depthCubeRenderTextures.Size(), "GetDepthCubeRenderTextureHeight out of bounds");
    ICARIAN_ASSERT_MSG(m_graphicsEngine->m_depthCubeRenderTextures.Exists(a_addr), "GetDepthCubeRenderTextureHeight already destroyed");

    const VulkanDepthCubeRenderTexture* texture = m_graphicsEngine->m_depthCubeRenderTextures[a_addr];

    return texture->GetHeight();
}
void VulkanGraphicsEngineBindings::ResizeDepthCubeRenderTexture(uint32_t a_addr, uint32_t a_width, uint32_t a_height) const
{
    ICARIAN_ASSERT_MSG(a_addr < m_graphicsEngine->m_depthCubeRenderTextures.Size(), "ResizeDepthCubeRenderTexture out of bounds");
    ICARIAN_ASSERT_MSG(m_graphicsEngine->m_depthCubeRenderTextures.Exists(a_addr), "ResizeDepthCubeRenderTexture already destroyed");
    ICARIAN_ASSERT_MSG(a_width > 0, "ResizeDepthCubeRenderTexture width 0")
    ICARIAN_ASSERT_MSG(a_height > 0, "ResizeDepthCubeRenderTexture height 0")

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
    IVERIFY(a_addr < m_graphicsEngine->m_ambientLights.Size());
    IVERIFY(m_graphicsEngine->m_ambientLights.Exists(a_addr));

    m_graphicsEngine->m_ambientLights.LockSet(a_addr, a_buffer);
}
AmbientLightBuffer VulkanGraphicsEngineBindings::GetAmbientLightBuffer(uint32_t a_addr) const
{
    return m_graphicsEngine->GetAmbientLight(a_addr);
}
void VulkanGraphicsEngineBindings::DestroyAmbientLightBuffer(uint32_t a_addr) const
{
    IPUSHDELETIONFUNC(
    {   
        IVERIFY(a_addr < m_graphicsEngine->m_ambientLights.Size());
        IVERIFY(m_graphicsEngine->m_ambientLights.Exists(a_addr));

        m_graphicsEngine->m_ambientLights.Erase(a_addr);
    }, DeletionIndex_Render);
}

uint32_t VulkanGraphicsEngineBindings::GenerateDirectionalLightBuffer(uint32_t a_transformAddr) const
{
    IVERIFY(a_transformAddr != -1);

    DirectionalLightBuffer buffer = 
    {
        .TransformAddr = a_transformAddr,
        .RenderLayer = 0b1,
        .Color = glm::vec4(1.0f),
        .Intensity = 1.0f,
    };
    
    VulkanLightBuffer* lightBuffer = new VulkanLightBuffer();
    lightBuffer->LightRenderTextureCount = 0;
    lightBuffer->LightRenderTextures = nullptr;
    buffer.Data = lightBuffer;

    return m_graphicsEngine->m_directionalLights.PushVal(buffer);
}
void VulkanGraphicsEngineBindings::SetDirectionalLightBuffer(uint32_t a_addr, const DirectionalLightBuffer& a_buffer) const
{
    IVERIFY(a_addr < m_graphicsEngine->m_directionalLights.Size());
    IVERIFY(m_graphicsEngine->m_directionalLights.Exists(a_addr));

    m_graphicsEngine->m_directionalLights.LockSet(a_addr, a_buffer);
}
DirectionalLightBuffer VulkanGraphicsEngineBindings::GetDirectionalLightBuffer(uint32_t a_addr) const
{
    return m_graphicsEngine->GetDirectionalLight(a_addr);
}
void VulkanGraphicsEngineBindings::DestroyDirectionalLightBuffer(uint32_t a_addr) const
{
    IPUSHDELETIONFUNC(
    {
        IVERIFY(a_addr < m_graphicsEngine->m_directionalLights.Size());
        IVERIFY(m_graphicsEngine->m_directionalLights.Exists(a_addr));

        const DirectionalLightBuffer buffer = m_graphicsEngine->m_directionalLights[a_addr];
        IVERIFY(buffer.Data != nullptr);

        const VulkanLightBuffer* lightBuffer = (VulkanLightBuffer*)buffer.Data;
        IDEFER(delete lightBuffer);

        m_graphicsEngine->m_directionalLights.Erase(a_addr);
    }, DeletionIndex_Render);
}
void VulkanGraphicsEngineBindings::AddDirectionalLightShadowMap(uint32_t a_addr, uint32_t a_shadowMapAddr) const
{
    IVERIFY(a_addr < m_graphicsEngine->m_directionalLights.Size());
    IVERIFY(m_graphicsEngine->m_directionalLights.Exists(a_addr));
    IVERIFY(a_shadowMapAddr < m_graphicsEngine->m_depthRenderTextures.Size());
    IVERIFY(m_graphicsEngine->m_depthRenderTextures.Exists(a_shadowMapAddr));

    TLockArray<DirectionalLightBuffer> a = m_graphicsEngine->m_directionalLights.ToLockArray();

    const DirectionalLightBuffer& buffer = a[a_addr];
    VulkanLightBuffer* lightBuffer = (VulkanLightBuffer*)buffer.Data;
    const uint32_t* oldRenderTexture = lightBuffer->LightRenderTextures;
    IDEFER(
    if (oldRenderTexture != nullptr)
    {
        delete[] oldRenderTexture;
    });

    uint32_t* renderTextures = new uint32_t[lightBuffer->LightRenderTextureCount + 1];
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
    IVERIFY(a_addr < m_graphicsEngine->m_directionalLights.Size());
    IVERIFY(m_graphicsEngine->m_directionalLights.Exists(a_addr));
    IVERIFY(a_shadowMapAddr < m_graphicsEngine->m_depthRenderTextures.Size());
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
    IVERIFY(a_transformAddr != -1);
    
    PointLightBuffer buffer = 
    {
        .TransformAddr = a_transformAddr,
        .RenderLayer = 0b1,
        .Color = glm::vec4(1.0f),
        .Intensity = 1.0f,
        .Radius = 1.0f
    };

    VulkanLightBuffer* lightBuffer = new VulkanLightBuffer();
    lightBuffer->LightRenderTextureCount = 0;
    lightBuffer->LightRenderTextures = nullptr;
    buffer.Data = lightBuffer;

    return m_graphicsEngine->m_pointLights.PushVal(buffer);
}
void VulkanGraphicsEngineBindings::SetPointLightBuffer(uint32_t a_addr, const PointLightBuffer& a_buffer) const
{
    IVERIFY(a_addr < m_graphicsEngine->m_pointLights.Size());
    IVERIFY(m_graphicsEngine->m_pointLights.Exists(a_addr));

    m_graphicsEngine->m_pointLights.LockSet(a_addr, a_buffer);
}
PointLightBuffer VulkanGraphicsEngineBindings::GetPointLightBuffer(uint32_t a_addr) const
{
    return m_graphicsEngine->GetPointLight(a_addr);
}
void VulkanGraphicsEngineBindings::DestroyPointLightBuffer(uint32_t a_addr) const
{
    IPUSHDELETIONFUNC(
    {
        IVERIFY(a_addr < m_graphicsEngine->m_pointLights.Size());
        IVERIFY(m_graphicsEngine->m_pointLights.Exists(a_addr));

        const PointLightBuffer buffer = m_graphicsEngine->m_pointLights[a_addr];
        IVERIFY(buffer.Data != nullptr);

        const VulkanLightBuffer* data = (VulkanLightBuffer*)buffer.Data;
        IDEFER(delete data);

        m_graphicsEngine->m_pointLights.Erase(a_addr);
    }, DeletionIndex_Render);
}
void VulkanGraphicsEngineBindings::SetPointLightShadowMap(uint32_t a_addr, uint32_t a_shadowMapAddr) const
{
    IVERIFY(a_addr < m_graphicsEngine->m_pointLights.Size());
    IVERIFY(m_graphicsEngine->m_pointLights.Exists(a_addr));
    
    TLockArray<PointLightBuffer> a = m_graphicsEngine->m_pointLights.ToLockArray();

    const PointLightBuffer& buffer = a[a_addr];
    IVERIFY(buffer.Data != nullptr);

    VulkanLightBuffer* lightBuffer = (VulkanLightBuffer*)buffer.Data;

    const uint32_t* oldRenderTexture = lightBuffer->LightRenderTextures;
    if (oldRenderTexture != nullptr)
    {
        delete[] oldRenderTexture;
    }

    lightBuffer->LightRenderTextures = nullptr;
    lightBuffer->LightRenderTextureCount = 0;

    if (a_shadowMapAddr != -1)
    {
        IVERIFY(a_shadowMapAddr < m_graphicsEngine->m_depthCubeRenderTextures.Size());
        IVERIFY(m_graphicsEngine->m_depthCubeRenderTextures.Exists(a_shadowMapAddr));

        lightBuffer->LightRenderTextures = new uint32_t[1];

        lightBuffer->LightRenderTextures[0] = a_shadowMapAddr;

        lightBuffer->LightRenderTextureCount = 1;
    }
}
uint32_t VulkanGraphicsEngineBindings::GetPointLightShadowMap(uint32_t a_addr) const
{
    IVERIFY(a_addr < m_graphicsEngine->m_pointLights.Size());
    IVERIFY(m_graphicsEngine->m_pointLights.Exists(a_addr));

    const PointLightBuffer& buffer = m_graphicsEngine->m_pointLights[a_addr];
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
    IVERIFY(a_transformAddr != -1);
    
    SpotLightBuffer buffer =
    {
        .TransformAddr = a_transformAddr,
        .RenderLayer = 0b1,
        .Color = glm::vec4(1.0f),
        .Intensity = 1.0f,
        .CutoffAngle = glm::vec2(1.0f, 1.5f),
        .Radius = 1.0f
    };
    
    VulkanLightBuffer* lightBuffer = new VulkanLightBuffer();
    lightBuffer->LightRenderTextureCount = 0;
    lightBuffer->LightRenderTextures = nullptr;
    buffer.Data = lightBuffer;

    return m_graphicsEngine->m_spotLights.PushVal(buffer);
}
void VulkanGraphicsEngineBindings::SetSpotLightBuffer(uint32_t a_addr, const SpotLightBuffer& a_buffer) const
{
    IVERIFY(a_addr < m_graphicsEngine->m_spotLights.Size());
    IVERIFY(m_graphicsEngine->m_spotLights.Exists(a_addr));

    m_graphicsEngine->m_spotLights.LockSet(a_addr, a_buffer);
}
SpotLightBuffer VulkanGraphicsEngineBindings::GetSpotLightBuffer(uint32_t a_addr) const
{
    return m_graphicsEngine->GetSpotLight(a_addr);
}
void VulkanGraphicsEngineBindings::DestroySpotLightBuffer(uint32_t a_addr) const
{
    IPUSHDELETIONFUNC(
    {
        IVERIFY(a_addr < m_graphicsEngine->m_spotLights.Size());
        IVERIFY(m_graphicsEngine->m_spotLights.Exists(a_addr));

        const SpotLightBuffer buffer = m_graphicsEngine->m_spotLights[a_addr];
        IVERIFY(buffer.Data);

        const VulkanLightBuffer* data = (VulkanLightBuffer*)buffer.Data;
        IDEFER(delete data);

        m_graphicsEngine->m_spotLights.Erase(a_addr);
    }, DeletionIndex_Render);
}
void VulkanGraphicsEngineBindings::SetSpotLightShadowMap(uint32_t a_addr, uint32_t a_shadowMapAddr) const
{
    IVERIFY(a_addr < m_graphicsEngine->m_spotLights.Size());
    IVERIFY(m_graphicsEngine->m_spotLights.Exists(a_addr));

    TLockArray<SpotLightBuffer> a = m_graphicsEngine->m_spotLights.ToLockArray();

    const SpotLightBuffer& buffer = a[a_addr];
    IVERIFY(buffer.Data != nullptr);

    VulkanLightBuffer* lightBuffer = (VulkanLightBuffer*)buffer.Data;

    uint32_t* renderTextures = lightBuffer->LightRenderTextures;

    lightBuffer->LightRenderTextures = nullptr;
    lightBuffer->LightRenderTextureCount = 0;

    if (a_shadowMapAddr != -1)
    {
        IVERIFY(a_shadowMapAddr < m_graphicsEngine->m_depthRenderTextures.Size());
        IVERIFY(m_graphicsEngine->m_depthRenderTextures.Exists(a_shadowMapAddr));

        if (renderTextures == nullptr)
        {
            renderTextures = new uint32_t[1];
        }

        renderTextures[0] = a_shadowMapAddr;

        lightBuffer->LightRenderTextures = renderTextures;
        lightBuffer->LightRenderTextureCount = 1;
    }
    else if (renderTextures != nullptr)
    {
        delete[] renderTextures;
    }
}
uint32_t VulkanGraphicsEngineBindings::GetSpotLightShadowMap(uint32_t a_addr) const
{
    IVERIFY(a_addr < m_graphicsEngine->m_spotLights.Size());
    IVERIFY(m_graphicsEngine->m_spotLights.Exists(a_addr));

    const SpotLightBuffer& buffer = m_graphicsEngine->m_spotLights[a_addr];
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
    IVERIFY(a_addr < m_graphicsEngine->m_canvasRenderers.Size());
    IVERIFY(m_graphicsEngine->m_canvasRenderers.Exists(a_addr));

    m_graphicsEngine->m_canvasRenderers.Erase(a_addr);
}
void VulkanGraphicsEngineBindings::SetCanvasRendererCanvas(uint32_t a_addr, uint32_t a_canvasAddr) const
{
    IVERIFY(a_addr < m_graphicsEngine->m_canvasRenderers.Size());
    IVERIFY(m_graphicsEngine->m_canvasRenderers.Exists(a_addr));
    
    TLockArray<CanvasRendererBuffer> a = m_graphicsEngine->m_canvasRenderers.ToLockArray();
    a[a_addr].CanvasAddr = a_canvasAddr;
}
uint32_t VulkanGraphicsEngineBindings::GetCanvasRendererCanvas(uint32_t a_addr) const
{
    IVERIFY(a_addr < m_graphicsEngine->m_canvasRenderers.Size());
    IVERIFY(m_graphicsEngine->m_canvasRenderers.Exists(a_addr));

    return m_graphicsEngine->m_canvasRenderers[a_addr].CanvasAddr;
}
void VulkanGraphicsEngineBindings::SetCanvasRendererRenderTexture(uint32_t a_addr, uint32_t a_renderTextureAddr) const
{
    IVERIFY(a_addr < m_graphicsEngine->m_canvasRenderers.Size());
    IVERIFY(m_graphicsEngine->m_canvasRenderers.Exists(a_addr));
    if (a_renderTextureAddr != -1)
    {
        IVERIFY(a_renderTextureAddr < m_graphicsEngine->m_renderTextures.Size());
        IVERIFY(m_graphicsEngine->m_renderTextures.Exists(a_renderTextureAddr));
    }

    TLockArray<CanvasRendererBuffer> a = m_graphicsEngine->m_canvasRenderers.ToLockArray();
    a[a_addr].RenderTextureAddr = a_renderTextureAddr;
}
uint32_t VulkanGraphicsEngineBindings::GetCanvasRendererRenderTexture(uint32_t a_addr) const
{
    IVERIFY(a_addr < m_graphicsEngine->m_canvasRenderers.Size());
    IVERIFY(m_graphicsEngine->m_canvasRenderers.Exists(a_addr));

    return m_graphicsEngine->m_canvasRenderers[a_addr].RenderTextureAddr;
}

void VulkanGraphicsEngineBindings::BindMaterial(uint32_t a_addr) const
{
    IVERIFY(m_graphicsEngine->m_renderCommands.Exists());
    if (a_addr != -1)
    {
        IVERIFY(a_addr < m_graphicsEngine->m_shaderPrograms.Size());
    }

    m_graphicsEngine->m_renderCommands->BindMaterial(a_addr);
}
void VulkanGraphicsEngineBindings::PushTexture(uint32_t a_slot, uint32_t a_samplerAddr) const
{
    IVERIFY(m_graphicsEngine->m_renderCommands.Exists());

    IVERIFY(a_samplerAddr < m_graphicsEngine->m_textureSampler.Size());
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

    VulkanRenderTexture* tex = nullptr;
    if (a_addr != -1)
    {
        IVERIFY(a_addr < m_graphicsEngine->m_renderTextures.Size());
        IVERIFY(m_graphicsEngine->m_renderTextures.Exists(a_addr));

        tex = m_graphicsEngine->m_renderTextures[a_addr];
    }

    m_graphicsEngine->m_renderCommands->BindRenderTexture(a_addr, a_bindMode);
}
void VulkanGraphicsEngineBindings::BlitRTRT(uint32_t a_srcAddr, uint32_t a_dstAddr) const
{
    IVERIFY(m_graphicsEngine->m_renderCommands.Exists());
    
    VulkanRenderTexture* srcTex = nullptr;
    VulkanRenderTexture* dstTex = nullptr;

    if (a_srcAddr != -1)
    {
        IVERIFY(a_srcAddr < m_graphicsEngine->m_renderTextures.Size());

        srcTex = m_graphicsEngine->m_renderTextures[a_srcAddr];
    }
    if (a_dstAddr != -1)
    {
        IVERIFY(a_dstAddr < m_graphicsEngine->m_renderTextures.Size());

        dstTex = m_graphicsEngine->m_renderTextures[a_dstAddr];
    }

    m_graphicsEngine->m_renderCommands->Blit(srcTex, dstTex);
}
void VulkanGraphicsEngineBindings::DrawMaterial()
{
    IVERIFY(m_graphicsEngine->m_renderCommands.Exists());

    m_graphicsEngine->m_renderCommands->DrawMaterial();
}
void VulkanGraphicsEngineBindings::DrawModel(const glm::mat4& a_transform, uint32_t a_addr)
{
    IVERIFY(m_graphicsEngine->m_renderCommands.Exists());

    m_graphicsEngine->m_renderCommands->DrawModel(a_transform, a_addr);
}

void VulkanGraphicsEngineBindings::SetLightSplits(const LightShadowSplit* a_splits, uint32_t a_splitCount) const
{
    IVERIFY(m_graphicsEngine->m_lightData.Exists());

    m_graphicsEngine->m_lightData->SetLightSplits(a_splits, a_splitCount);
}

#endif