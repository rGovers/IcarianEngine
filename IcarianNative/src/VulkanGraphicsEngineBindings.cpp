#ifdef ICARIANNATIVE_ENABLE_GRAPHICS_VULKAN

#include "Rendering/Vulkan/VulkanGraphicsEngineBindings.h"

#include <fstream>
#include <sstream>

#include "Core/IcarianAssert.h"
#include "Core/IcarianDefer.h"
#include "DeletionQueue.h"
#include "Logger.h"
#include "ObjectManager.h"
#include "Rendering/RenderEngine.h"
#include "Rendering/ShaderTable.h"
#include "Rendering/UI/Font.h"
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
    F(uint32_t, IcarianEngine.Rendering, TextureSampler, GenerateTextureSampler, { return Instance->GenerateTextureSampler(a_texture, (e_TextureFilter)a_filter, (e_TextureAddress)a_addressMode ); }, uint32_t a_texture, uint32_t a_filter, uint32_t a_addressMode) \
    F(uint32_t, IcarianEngine.Rendering, TextureSampler, GenerateRenderTextureSampler, { return Instance->GenerateRenderTextureSampler(a_renderTexture, a_textureIndex, (e_TextureFilter)a_filter, (e_TextureAddress)a_addressMode); }, uint32_t a_renderTexture, uint32_t a_textureIndex, uint32_t a_filter, uint32_t a_addressMode) \
    F(uint32_t, IcarianEngine.Rendering, TextureSampler, GenerateRenderTextureDepthSampler, { return Instance->GenerateRenderTextureDepthSampler(a_renderTexture, (e_TextureFilter)a_filter, (e_TextureAddress)a_addressMode); }, uint32_t a_renderTexture, uint32_t a_filter, uint32_t a_addressMode) \
    F(uint32_t, IcarianEngine.Rendering, TextureSampler, GenerateRenderTextureDepthSamplerDepth, { return Instance->GenerateRenderTextureDepthSamplerDepth(a_renderTexture, (e_TextureFilter)a_filter, (e_TextureAddress)a_addressMode); }, uint32_t a_renderTexture, uint32_t a_filter, uint32_t a_addressMode) \
    F(void, IcarianEngine.Rendering, TextureSampler, DestroySampler, { Instance->DestroyTextureSampler(a_addr); }, uint32_t a_addr) \
    \
    F(uint32_t, IcarianEngine.Rendering, RenderTextureCmd, GenerateRenderTexture, { return Instance->GenerateRenderTexture(a_count, a_width, a_height, (bool)a_depthTexture, (bool)a_hdr); }, uint32_t a_count, uint32_t a_width, uint32_t a_height, uint32_t a_depthTexture, uint32_t a_hdr) \
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
    F(void, IcarianEngine.Rendering, Model, DestroyModel, { Instance->DestroyModel(a_addr); }, uint32_t a_addr) \
    \
    F(uint32_t, IcarianEngine.Rendering, ParticleSystem2D, GenerateGraphicsParticleSystem, { return Instance->GenerateGraphicsParticle2D(a_computeBuffer); }, uint32_t a_computeBuffer) \
    F(void, IcarianEngine.Rendering, ParticleSystem2D, DestroyGraphicsParticleSystem, { Instance->DestroyGraphicsParticle2D(a_bufferAddr); }, uint32_t a_bufferAddr) \
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
    F(uint32_t, IcarianEngine.Rendering.UI, Font, GenerateFont, { char* str = mono_string_to_utf8(a_path); IDEFER(mono_free(str)); return Instance->GenerateFont(str); }, MonoString* a_path) \
    F(void, IcarianEngine.Rendering.UI, Font, DestroyFont, { Instance->DestroyFont(a_addr); }, uint32_t a_addr) \
    \
    F(uint32_t, IcarianEngine.Rendering.UI, CanvasRenderer, GenerateBuffer, { return Instance->GenerateCanvasRenderer(); }) \
    F(void, IcarianEngine.Rendering.UI, CanvasRenderer, DestroyBuffer, { Instance->DestroyCanvasRenderer(a_addr); }, uint32_t a_addr) \
    F(void, IcarianEngine.Rendering.UI, CanvasRenderer, SetCanvas, { Instance->SetCanvasRendererCanvas(a_addr, a_canvasAddr); }, uint32_t a_addr, uint32_t a_canvasAddr) \
    F(uint32_t, IcarianEngine.Rendering.UI, CanvasRenderer, GetCanvas, { return Instance->GetCanvasRendererCanvas(a_addr); }, uint32_t a_addr) \
    F(void, IcarianEngine.Rendering.UI, CanvasRenderer, SetRenderTexture, { Instance->SetCanvasRendererRenderTexture(a_addr, a_renderTextureAddr); }, uint32_t a_addr, uint32_t a_renderTextureAddr) \
    F(uint32_t, IcarianEngine.Rendering.UI, CanvasRenderer, GetRenderTexture, { return Instance->GetCanvasRendererRenderTexture(a_addr); }, uint32_t a_addr) \
    \
    F(void, IcarianEngine.Rendering, RenderCommand, BindMaterial, { Instance->BindMaterial(a_addr); }, uint32_t a_addr) \
    F(void, IcarianEngine.Rendering, RenderCommand, PushTexture, { Instance->PushTexture(a_slot, a_samplerAddr); }, uint32_t a_slot, uint32_t a_samplerAddr) \
    F(void, IcarianEngine.Rendering, RenderCommand, BindRenderTexture, { Instance->BindRenderTexture(a_addr); }, uint32_t a_addr) \
    F(void, IcarianEngine.Rendering, RenderCommand, RTRTBlit, { Instance->BlitRTRT(a_srcAddr, a_dstAddr); }, uint32_t a_srcAddr, uint32_t a_dstAddr) \
    F(void, IcarianEngine.Rendering, RenderCommand, DrawMaterial, { Instance->DrawMaterial(); }) \
    \
    F(void, IcarianEngine.Rendering.Animation, SkeletonAnimator, PushTransform, { }, uint32_t a_addr, MonoString* a_object, MonoArray* a_transform) \

VULKANGRAPHICS_BINDING_FUNCTION_TABLE(RUNTIME_FUNCTION_DEFINITION);

RUNTIME_FUNCTION(uint32_t, VertexShader, GenerateFromFile, 
{
    char* str = mono_string_to_utf8(a_path);
    IDEFER(mono_free(str));

    const std::string s = str;
    if (s.find_first_of("[INTERNAL]") == 0)
    {
        const char* shader = GetVertexShaderString(s);

        if (shader != nullptr)
        {
            return Instance->GenerateFVertexShaderAddr(shader);
        }
    }
    else
    {
        const std::filesystem::path p = std::filesystem::path(s);

        if (p.extension() == ".fvert")
        {
            std::ifstream file = std::ifstream(p);
            if (file.good() && file.is_open())
            {
                std::stringstream ss;

                ss << file.rdbuf();

                return Instance->GenerateFVertexShaderAddr(ss.str());
            }
        }
        else if (p.extension() == ".vert")
        {
            std::ifstream file = std::ifstream(p);
            if (file.good() && file.is_open())
            {
                std::stringstream ss;

                ss << file.rdbuf();

                return Instance->GenerateGLSLVertexShaderAddr(ss.str());
            }
        }
    }

    return -1;
}, MonoString* a_path)
RUNTIME_FUNCTION(uint32_t, PixelShader, GenerateFromFile, 
{
    char* str = mono_string_to_utf8(a_path);
    IDEFER(mono_free(str));

    const std::string s = str;

    if (s.find_first_of("[INTERNAL]") == 0)
    {
        const char* shader = GetPixelShaderString(s);

        if (shader != nullptr)
        {
            return Instance->GenerateFPixelShaderAddr(shader);
        }
    }
    else
    {
        const std::filesystem::path p = std::filesystem::path(str);
        const std::filesystem::path ext = p.extension();

        if (ext == ".fpix" || ext == ".ffrag")
        {
            std::ifstream file = std::ifstream(p);
            if (file.good() && file.is_open())
            {
                std::stringstream ss;

                ss << file.rdbuf();

                return Instance->GenerateFPixelShaderAddr(ss.str());
            }
        }
        else if (ext == ".pix" || ext == ".frag")
        {
            std::ifstream file = std::ifstream(p);
            if (file.good() && file.is_open())
            {
                std::stringstream ss;

                ss << file.rdbuf();

                return Instance->GenerateGLSLPixelShaderAddr(ss.str());
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
    RenderProgram program;
    memset(&program, 0, sizeof(RenderProgram));
    program.VertexShader = a_vertexShader;
    program.PixelShader = a_pixelShader;
    program.ShadowVertexShader = a_shadowVertexShader;
    program.VertexStride = a_vertexStride;
    program.CullingMode = (e_CullMode)a_cullMode;
    program.PrimitiveMode = (e_PrimitiveMode)a_primitiveMode;
    program.EnableColorBlending = (uint8_t)a_enableColorBlending;
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

    if (a_shaderInputs != NULL)
    {
        program.ShaderBufferInputCount = (uint16_t)mono_array_length(a_shaderInputs);
        program.ShaderBufferInputs = new ShaderBufferInput[program.ShaderBufferInputCount];

        for (uint32_t i = 0; i < program.ShaderBufferInputCount; ++i)
        {
            program.ShaderBufferInputs[i] = mono_array_get(a_shaderInputs, ShaderBufferInput, i);
        }
    }

    if (a_shadowShaderInputs != NULL)
    {
        program.ShadowShaderBufferInputCount = (uint16_t)mono_array_length(a_shadowShaderInputs);
        program.ShadowShaderBufferInputs = new ShaderBufferInput[program.ShadowShaderBufferInputCount];

        for (uint32_t i = 0; i < program.ShadowShaderBufferInputCount; ++i)
        {
            program.ShadowShaderBufferInputs[i] = mono_array_get(a_shadowShaderInputs, ShaderBufferInput, i);
        }
    }

    if (a_uboData != NULL)
    {
        program.UBODataSize = a_uboSize;
        program.UBOData = malloc((size_t)program.UBODataSize);

        memcpy(program.UBOData, a_uboData, program.UBODataSize);
    }

    return Instance->GenerateShaderProgram(program);
}, uint32_t a_vertexShader, uint32_t a_pixelShader, uint16_t a_vertexStride, MonoArray* a_vertexInputAttribs, MonoArray* a_shaderInputs, uint32_t a_cullMode, uint32_t a_primitiveMode, uint32_t a_enableColorBlending, uint32_t a_renderLayer, uint32_t a_shadowVertexShader, MonoArray* a_shadowShaderInputs, uint32_t a_uboSize, void* a_uboData)
RUNTIME_FUNCTION(void, Material, DestroyProgram, 
{
    const RenderProgram program = Instance->GetRenderProgram(a_addr);

    IDEFER(
    {
        if (program.VertexAttributes != nullptr)
        {
            delete[] program.VertexAttributes;
        }
        
        if (program.ShaderBufferInputs != nullptr)
        {
            delete[] program.ShaderBufferInputs;
        }

        if (program.ShadowShaderBufferInputs != nullptr)
        {
            delete[] program.ShadowShaderBufferInputs;
        }
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

RUNTIME_FUNCTION(void, RenderCommand, DrawModel, 
{
    glm::mat4 transform;

    float* f = (float*)&transform;
    for (int i = 0; i < 16; ++i)
    {
        f[i] = mono_array_get(a_transform, float, i);
    }

    Instance->DrawModel(transform, a_addr);
}, MonoArray* a_transform, uint32_t a_addr)

RUNTIME_FUNCTION(void, RenderPipeline, SetLightLVP,
{
    const uint32_t lightLVPCount = (uint32_t)mono_array_length(a_lightLVP);

    glm::mat4* lightLVP = new glm::mat4[lightLVPCount];
    IDEFER(delete[] lightLVP);

    for (uint32_t i = 0; i < lightLVPCount; ++i)
    {
        MonoArray* lvpArray = mono_array_get(a_lightLVP, MonoArray*, i);

        float* f = (float*)&(lightLVP[i]);
        for (int j = 0; j < 16; ++j)
        {
            f[j] = mono_array_get(lvpArray, float, j);
        }
    }

    Instance->SetLightLVP(lightLVP, lightLVPCount);
}, MonoArray* a_lightLVP)
RUNTIME_FUNCTION(void, RenderPipeline, SetLightSplits, 
{
    const uint32_t lightSplitCount = (uint32_t)mono_array_length(a_lightSplits);

    float* lightSplits = new float[lightSplitCount];
    IDEFER(delete[] lightSplits);

    for (uint32_t i = 0; i < lightSplitCount; ++i)
    {
        lightSplits[i] = mono_array_get(a_lightSplits, float, i);
    }

    Instance->SetLightSplits(lightSplits, lightSplitCount);
}, MonoArray* a_lightSplits)

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

    BIND_FUNCTION(IcarianEngine.Rendering, RenderCommand, DrawModel);

    BIND_FUNCTION(IcarianEngine.Rendering, RenderPipeline, SetLightLVP);
    BIND_FUNCTION(IcarianEngine.Rendering, RenderPipeline, SetLightSplits);
}
VulkanGraphicsEngineBindings::~VulkanGraphicsEngineBindings()
{

}

uint32_t VulkanGraphicsEngineBindings::GenerateFVertexShaderAddr(const std::string_view& a_str) const
{
    return m_graphicsEngine->GenerateFVertexShader(a_str);
}
uint32_t VulkanGraphicsEngineBindings::GenerateGLSLVertexShaderAddr(const std::string_view& a_str) const
{
    return m_graphicsEngine->GenerateGLSLVertexShader(a_str);
}
void VulkanGraphicsEngineBindings::DestroyVertexShader(uint32_t a_addr) const
{
    m_graphicsEngine->DestroyVertexShader(a_addr);
}

uint32_t VulkanGraphicsEngineBindings::GenerateFPixelShaderAddr(const std::string_view& a_str) const
{
    return m_graphicsEngine->GenerateFPixelShader(a_str);
}
uint32_t VulkanGraphicsEngineBindings::GenerateGLSLPixelShaderAddr(const std::string_view& a_str) const
{
    return m_graphicsEngine->GenerateGLSLPixelShader(a_str);
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

    const RenderProgram program = a[a_addr];

    ICARIAN_ASSERT_MSG(program.Data != nullptr, "RenderProgramSetTexture invalid program");

    VulkanShaderData* data = (VulkanShaderData*)program.Data;

    TReadLockArray<TextureSamplerBuffer> b = m_graphicsEngine->m_textureSampler.ToReadLockArray();

    data->SetTexture(a_shaderSlot, b[a_samplerAddr]);
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
    ICARIAN_ASSERT_MSG(a_transformAddr != -1, "GenerateCameraBuffer invalid transform address")

    const CameraBuffer buff = CameraBuffer(a_transformAddr);

    uint32_t size = 0;
    {
        TRACE("Getting Camera Buffer");
        TLockArray<CameraBuffer> a = m_graphicsEngine->m_cameraBuffers.ToLockArray();

        size = a.Size();
        for (uint32_t i = 0; i < size; ++i)
        {
            if (a[i].TransformAddr != -1)
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
    TLockArray<CameraBuffer> a = m_graphicsEngine->m_cameraBuffers.ToLockArray();

    ICARIAN_ASSERT_MSG(a_addr < a.Size(), "DestroyCameraBuffer out of bounds")
    a[a_addr].TransformAddr = -1;
}
CameraBuffer VulkanGraphicsEngineBindings::GetCameraBuffer(uint32_t a_addr) const
{
    ICARIAN_ASSERT_MSG(a_addr < m_graphicsEngine->m_cameraBuffers.Size(), "GetCameraBuffer out of bounds")

    return m_graphicsEngine->m_cameraBuffers[a_addr];
}
void VulkanGraphicsEngineBindings::SetCameraBuffer(uint32_t a_addr, const CameraBuffer& a_buffer) const
{
    ICARIAN_ASSERT_MSG(a_addr < m_graphicsEngine->m_cameraBuffers.Size(), "SetCameraBuffer out of bounds")

    m_graphicsEngine->m_cameraBuffers.LockSet(a_addr, a_buffer);
}
glm::vec3 VulkanGraphicsEngineBindings::CameraScreenToWorld(uint32_t a_addr, const glm::vec3& a_screenPos, const glm::vec2& a_screenSize) const
{
    ICARIAN_ASSERT_MSG(a_addr < m_graphicsEngine->m_cameraBuffers.Size(), "CameraScreenToWorld out of bounds");

    const CameraBuffer camBuf = m_graphicsEngine->m_cameraBuffers[a_addr];

    ICARIAN_ASSERT_MSG(camBuf.TransformAddr != -1, "CameraScreenToWorld invalid transform");

    const glm::mat4 proj = camBuf.ToProjection(a_screenSize);
    const glm::mat4 invProj = glm::inverse(proj);

    const glm::mat4 invView = ObjectManager::GetGlobalMatrix(camBuf.TransformAddr);

    const glm::vec4 cPos = invProj * glm::vec4(a_screenPos.xy() * 2.0f - 1.0f, a_screenPos.z, 1.0f);
    const glm::vec4 wPos = invView * cPos;

    return wPos.xyz() / wPos.w;
}
glm::mat4 VulkanGraphicsEngineBindings::GetCameraProjectionMatrix(uint32_t a_addr, uint32_t a_width, uint32_t a_height) const
{
    ICARIAN_ASSERT_MSG(a_addr < m_graphicsEngine->m_cameraBuffers.Size(), "GetCameraProjectionMatrix out of bounds");
    ICARIAN_ASSERT_MSG(m_graphicsEngine->m_cameraBuffers[a_addr].TransformAddr != -1, "GetCameraProjectionMatrix invalid transform");

    const CameraBuffer camBuf = m_graphicsEngine->m_cameraBuffers[a_addr];

    return camBuf.ToProjection(glm::vec2((float)a_width, (float)a_height));
}
glm::mat4 VulkanGraphicsEngineBindings::GetCameraProjectionMatrix(uint32_t a_addr, uint32_t a_width, uint32_t a_height, float a_near, float a_far) const
{
    ICARIAN_ASSERT_MSG(a_addr < m_graphicsEngine->m_cameraBuffers.Size(), "GetCameraProjectionMatrix out of bounds");
    ICARIAN_ASSERT_MSG(m_graphicsEngine->m_cameraBuffers[a_addr].TransformAddr != -1, "GetCameraProjectionMatrix invalid transform");

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
    ICARIAN_ASSERT_MSG(a_addr < m_graphicsEngine->m_renderBuffers.Size(), "DestroyMeshRenderBuffer out of bounds");

    m_graphicsEngine->m_renderBuffers.Erase(a_addr);
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
    ICARIAN_ASSERT_MSG(a_meshAddr < m_graphicsEngine->m_renderBuffers.Size(), "DestroyRenderStack out of bounds");
    ICARIAN_ASSERT_MSG(m_graphicsEngine->m_renderBuffers.Exists(a_meshAddr), "DestroyRenderStack renderer is destroyed")

    TLockArray<MeshRenderBuffer> aBuffer = m_graphicsEngine->m_renderBuffers.ToLockArray();
    const MeshRenderBuffer& buffer = aBuffer[a_meshAddr];

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
}

uint32_t VulkanGraphicsEngineBindings::GenerateSkinnedMeshRenderBuffer(uint32_t a_materialAddr, uint32_t a_modelAddr, uint32_t a_transformAddr, uint32_t a_skeletonAddr) const
{   
    TRACE("Creating Skinned Render Buffer");
    const SkinnedMeshRenderBuffer buffer = SkinnedMeshRenderBuffer(a_skeletonAddr, a_materialAddr, a_modelAddr, a_transformAddr);

    return m_graphicsEngine->m_skinnedRenderBuffers.PushVal(buffer);
}
void VulkanGraphicsEngineBindings::DestroySkinnedMeshRenderBuffer(uint32_t a_addr) const
{
    ICARIAN_ASSERT_MSG(a_addr < m_graphicsEngine->m_skinnedRenderBuffers.Size(), "DestroySkinnedMeshRenderBuffer out of bounds");

    TRACE("Destroying Skinned Render Buffer");
    m_graphicsEngine->m_skinnedRenderBuffers.Erase(a_addr);
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
    ICARIAN_ASSERT_MSG(a_addr < m_graphicsEngine->m_skinnedRenderBuffers.Size(), "DestroySkinnedRenderStack out of bounds");

    TRACE("Removing Skinned RenderStack");
    const SkinnedMeshRenderBuffer& buffer = m_graphicsEngine->m_skinnedRenderBuffers[a_addr];

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
}

uint32_t VulkanGraphicsEngineBindings::GenerateGraphicsParticle2D(uint32_t a_computeBufferAddr) const
{
    VulkanGraphicsParticle2D* particleSystem = new VulkanGraphicsParticle2D(m_graphicsEngine->m_vulkanEngine, m_graphicsEngine->m_vulkanEngine->GetComputeEngine(), m_graphicsEngine, a_computeBufferAddr);

    return m_graphicsEngine->m_particleEmitters.PushVal(particleSystem);   
}
void VulkanGraphicsEngineBindings::DestroyGraphicsParticle2D(uint32_t a_addr) const
{
    IPUSHDELETIONFUNC(
    {
        ICARIAN_ASSERT_MSG(a_addr < m_graphicsEngine->m_particleEmitters.Size(), "DestroyGraphicsParticle2D out of bounds");
        ICARIAN_ASSERT_MSG(m_graphicsEngine->m_particleEmitters.Exists(a_addr), "DestroyGraphicsParticle2D already destroyed");

        const VulkanGraphicsParticle2D* particleSystem = m_graphicsEngine->m_particleEmitters[a_addr];
        IDEFER(delete particleSystem);
        m_graphicsEngine->m_particleEmitters.Erase(a_addr);
    }, DeletionIndex_Render);
}

void VulkanGraphicsEngineBindings::DestroyTexture(uint32_t a_addr) const
{
    IPUSHDELETIONFUNC(
    {
        m_graphicsEngine->DestroyTexture(a_addr);
    }, DeletionIndex_Render);
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

uint32_t VulkanGraphicsEngineBindings::GenerateRenderTexture(uint32_t a_count, uint32_t a_width, uint32_t a_height, bool a_depthTexture, bool a_hdr) const
{
    ICARIAN_ASSERT_MSG(a_count > 0, "GenerateRenderTexture no textures");
    ICARIAN_ASSERT_MSG(a_width > 0, "GenerateRenderTexture width 0");
    ICARIAN_ASSERT_MSG(a_height > 0, "GenerateRenderTexture height 0");

    VulkanRenderTexture* texture = new VulkanRenderTexture(m_graphicsEngine->m_vulkanEngine, a_count, a_width, a_height, a_depthTexture, a_hdr);

    return m_graphicsEngine->m_renderTextures.PushVal(texture);
}
void VulkanGraphicsEngineBindings::DestroyRenderTexture(uint32_t a_addr) const
{
    ICARIAN_ASSERT_MSG(a_addr < m_graphicsEngine->m_renderTextures.Size(), "DestroyRenderTexture out of bounds");
    ICARIAN_ASSERT_MSG(m_graphicsEngine->m_renderTextures.Exists(a_addr), "DestroyRenderTexture already destroyed");

    const VulkanRenderTexture* tex = m_graphicsEngine->m_renderTextures[a_addr];
    IDEFER(delete tex);
    m_graphicsEngine->m_renderTextures.Erase(a_addr);
}
uint32_t VulkanGraphicsEngineBindings::GetRenderTextureTextureCount(uint32_t a_addr) const
{
    ICARIAN_ASSERT_MSG(a_addr < m_graphicsEngine->m_renderTextures.Size(), "GetRenderTextureCount out of bounds");
    ICARIAN_ASSERT_MSG(m_graphicsEngine->m_renderTextures.Exists(a_addr), "GetRenderTextureCount already destroyed");

    const VulkanRenderTexture* texture = m_graphicsEngine->m_renderTextures[a_addr];

    return texture->GetTextureCount();
}
bool VulkanGraphicsEngineBindings::RenderTextureHasDepth(uint32_t a_addr) const
{
    ICARIAN_ASSERT_MSG(a_addr < m_graphicsEngine->m_renderTextures.Size(), "RenderTextureHasDepth out of bounds");
    ICARIAN_ASSERT_MSG(m_graphicsEngine->m_renderTextures.Exists(a_addr), "RenderTextureHasDepth already destroyed");

    const VulkanRenderTexture* texture = m_graphicsEngine->m_renderTextures[a_addr];

    return texture->HasDepthTexture();
}
uint32_t VulkanGraphicsEngineBindings::GetRenderTextureWidth(uint32_t a_addr) const
{
    ICARIAN_ASSERT_MSG(a_addr < m_graphicsEngine->m_renderTextures.Size(), "GetRenderTextureWidth out of bounds");
    ICARIAN_ASSERT_MSG(m_graphicsEngine->m_renderTextures.Exists(a_addr), "GetRenderTextureWidth already destroyed");

    const VulkanRenderTexture* texture = m_graphicsEngine->m_renderTextures[a_addr];

    return texture->GetWidth();
}
uint32_t VulkanGraphicsEngineBindings::GetRenderTextureHeight(uint32_t a_addr) const
{
    ICARIAN_ASSERT_MSG(a_addr < m_graphicsEngine->m_renderTextures.Size(), "GetRenderTextureHeight out of bounds");
    ICARIAN_ASSERT_MSG(m_graphicsEngine->m_renderTextures.Exists(a_addr), "GetRenderTextureHeight already destroyed");

    const VulkanRenderTexture* texture = m_graphicsEngine->m_renderTextures[a_addr];

    return texture->GetHeight();
}
void VulkanGraphicsEngineBindings::ResizeRenderTexture(uint32_t a_addr, uint32_t a_width, uint32_t a_height) const
{
    ICARIAN_ASSERT_MSG(a_addr < m_graphicsEngine->m_renderTextures.Size(), "ResizeRenderTexture out of bounds");
    ICARIAN_ASSERT_MSG(m_graphicsEngine->m_renderTextures.Exists(a_addr), "ResizeRenderTexture already destroyed");
    ICARIAN_ASSERT_MSG(a_width > 0, "ResizeRenderTexture width 0")
    ICARIAN_ASSERT_MSG(a_height > 0, "ResizeRenderTexture height 0")

    TLockArray<VulkanRenderTexture*> a = m_graphicsEngine->m_renderTextures.ToLockArray();

    VulkanRenderTexture* texture = a[a_addr];
    texture->Resize(a_width, a_height);
}

uint32_t VulkanGraphicsEngineBindings::GenerateDepthRenderTexture(uint32_t a_width, uint32_t a_height) const
{
    ICARIAN_ASSERT_MSG(a_width > 0, "GenerateDepthRenderTexture width 0")
    ICARIAN_ASSERT_MSG(a_height > 0, "GenerateDepthRenderTexture height 0")

    VulkanDepthRenderTexture* texture = new VulkanDepthRenderTexture(m_graphicsEngine->m_vulkanEngine, a_width, a_height);

    return m_graphicsEngine->m_depthRenderTextures.PushVal(texture);
}
void VulkanGraphicsEngineBindings::DestroyDepthRenderTexture(uint32_t a_addr) const
{
    ICARIAN_ASSERT_MSG(a_addr < m_graphicsEngine->m_depthRenderTextures.Size(), "DestroyDepthRenderTexture out of bounds");
    ICARIAN_ASSERT_MSG(m_graphicsEngine->m_depthRenderTextures.Exists(a_addr), "DestroyDepthRenderTexture already destroyed");

    const VulkanDepthRenderTexture* tex = m_graphicsEngine->m_depthRenderTextures[a_addr];
    IDEFER(delete tex);

    m_graphicsEngine->m_depthRenderTextures.Erase(a_addr);
}
uint32_t VulkanGraphicsEngineBindings::GetDepthRenderTextureWidth(uint32_t a_addr) const
{
    ICARIAN_ASSERT_MSG(a_addr < m_graphicsEngine->m_depthRenderTextures.Size(), "GetDepthRenderTextureWidth out of bounds");
    ICARIAN_ASSERT_MSG(m_graphicsEngine->m_depthRenderTextures.Exists(a_addr), "GetDepthRenderTextureWidth already destroyed");

    const VulkanDepthRenderTexture* texture = m_graphicsEngine->m_depthRenderTextures[a_addr];
    
    return texture->GetWidth();
}
uint32_t VulkanGraphicsEngineBindings::GetDepthRenderTextureHeight(uint32_t a_addr) const
{
    ICARIAN_ASSERT_MSG(a_addr < m_graphicsEngine->m_depthRenderTextures.Size(), "GetDepthRenderTextureHeight out of bounds");
    ICARIAN_ASSERT_MSG(m_graphicsEngine->m_depthRenderTextures.Exists(a_addr), "GetDepthRenderTextureHeight already destroyed");

    const VulkanDepthRenderTexture* texture = m_graphicsEngine->m_depthRenderTextures[a_addr];

    return texture->GetHeight();
}
void VulkanGraphicsEngineBindings::ResizeDepthRenderTexture(uint32_t a_addr, uint32_t a_width, uint32_t a_height) const
{
    ICARIAN_ASSERT_MSG(a_addr < m_graphicsEngine->m_depthRenderTextures.Size(), "ResizeDepthRenderTexture out of bounds");
    ICARIAN_ASSERT_MSG(m_graphicsEngine->m_depthRenderTextures.Exists(a_addr), "ResizeDepthRenderTexture already destroyed");
    ICARIAN_ASSERT_MSG(a_width > 0, "ResizeDepthRenderTexture width 0")
    ICARIAN_ASSERT_MSG(a_height > 0, "ResizeDepthRenderTexture height 0")

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
    AmbientLightBuffer buffer;
    buffer.Color = glm::vec4(1.0f);
    buffer.Intensity = 1.0f;
    buffer.RenderLayer = 0b1;
    buffer.Data = nullptr;

    return m_graphicsEngine->m_ambientLights.PushVal(buffer);
}
void VulkanGraphicsEngineBindings::SetAmbientLightBuffer(uint32_t a_addr, const AmbientLightBuffer& a_buffer) const
{
    ICARIAN_ASSERT_MSG(a_addr < m_graphicsEngine->m_ambientLights.Size(), "SetAmbientLightBuffer out of bounds");
    ICARIAN_ASSERT_MSG(m_graphicsEngine->m_ambientLights.Exists(a_addr), "SetAmbientLightBuffer already destroyed");

    m_graphicsEngine->m_ambientLights.LockSet(a_addr, a_buffer);
}
AmbientLightBuffer VulkanGraphicsEngineBindings::GetAmbientLightBuffer(uint32_t a_addr) const
{
    ICARIAN_ASSERT_MSG(a_addr < m_graphicsEngine->m_ambientLights.Size(), "GetAmbientLightBuffer out of bounds");
    ICARIAN_ASSERT_MSG(m_graphicsEngine->m_ambientLights.Exists(a_addr), "GetAmbientLightBuffer already destroyed");

    return m_graphicsEngine->m_ambientLights[a_addr];
}
void VulkanGraphicsEngineBindings::DestroyAmbientLightBuffer(uint32_t a_addr) const
{
    class VulkanAmbientLightDeletionObject : public DeletionObject
    {
    private:
        uint32_t m_addr;

    protected:

    public:
        VulkanAmbientLightDeletionObject(uint32_t a_addr)
        {
            m_addr = a_addr;
        }
        virtual ~VulkanAmbientLightDeletionObject()
        {

        }

        virtual void Destroy()
        {
            ICARIAN_ASSERT_MSG(m_addr < Instance->m_graphicsEngine->m_ambientLights.Size(), "DestroyAmbientLightBuffer out of bounds");
            ICARIAN_ASSERT_MSG(Instance->m_graphicsEngine->m_ambientLights.Exists(m_addr), "DestroyAmbientLightBuffer already destroyed");

            Instance->m_graphicsEngine->m_ambientLights.Erase(m_addr);
        }
    };

    DeletionQueue::Push(new VulkanAmbientLightDeletionObject(a_addr), DeletionIndex_Render);
}

uint32_t VulkanGraphicsEngineBindings::GenerateDirectionalLightBuffer(uint32_t a_transformAddr) const
{
    ICARIAN_ASSERT_MSG(a_transformAddr != -1, "GenerateDirectionalLightBuffer no transform");

    DirectionalLightBuffer buffer;
    buffer.TransformAddr = a_transformAddr;
    buffer.Color = glm::vec4(1.0f);
    buffer.Intensity = 1.0f;
    buffer.RenderLayer = 0b1;
    
    VulkanLightBuffer* lightBuffer = new VulkanLightBuffer();
    lightBuffer->LightRenderTextureCount = 0;
    lightBuffer->LightRenderTextures = nullptr;
    buffer.Data = lightBuffer;

    return m_graphicsEngine->m_directionalLights.PushVal(buffer);
}
void VulkanGraphicsEngineBindings::SetDirectionalLightBuffer(uint32_t a_addr, const DirectionalLightBuffer& a_buffer) const
{
    ICARIAN_ASSERT_MSG(a_addr < m_graphicsEngine->m_directionalLights.Size(), "SetDirectionalLightBuffer out of bounds");

    m_graphicsEngine->m_directionalLights.LockSet(a_addr, a_buffer);
}
DirectionalLightBuffer VulkanGraphicsEngineBindings::GetDirectionalLightBuffer(uint32_t a_addr) const
{
    ICARIAN_ASSERT_MSG(a_addr < m_graphicsEngine->m_directionalLights.Size(), "GetDirectionalLightBuffer out of bounds");

    return m_graphicsEngine->m_directionalLights[a_addr];
}
void VulkanGraphicsEngineBindings::DestroyDirectionalLightBuffer(uint32_t a_addr) const
{
    class VulkanDirectionalLightDeletionObject : public DeletionObject
    {
    private:
        uint32_t m_addr;

    protected:

    public:
        VulkanDirectionalLightDeletionObject(uint32_t a_addr)
        {
            m_addr = a_addr;
        }
        virtual ~VulkanDirectionalLightDeletionObject()
        {

        }

        virtual void Destroy()
        {
            ICARIAN_ASSERT_MSG(m_addr < Instance->m_graphicsEngine->m_directionalLights.Size(), "DestroyDirectionalLightBuffer out of bounds");
            ICARIAN_ASSERT_MSG(Instance->m_graphicsEngine->m_directionalLights.Exists(m_addr), "DestroyDirectionalLightBuffer already destroyed");

            const DirectionalLightBuffer buffer = Instance->m_graphicsEngine->m_directionalLights[m_addr];
            const VulkanLightBuffer* lightBuffer = (VulkanLightBuffer*)buffer.Data;
            IDEFER(delete lightBuffer);
            Instance->m_graphicsEngine->m_directionalLights.Erase(m_addr);
        }
    };

    DeletionQueue::Push(new VulkanDirectionalLightDeletionObject(a_addr), DeletionIndex_Render);
}
void VulkanGraphicsEngineBindings::AddDirectionalLightShadowMap(uint32_t a_addr, uint32_t a_shadowMapAddr) const
{
    ICARIAN_ASSERT_MSG(a_addr < m_graphicsEngine->m_directionalLights.Size(), "AddDirectionalLightShadowMap DirectionalLight out of bounds");
    ICARIAN_ASSERT_MSG(m_graphicsEngine->m_directionalLights.Exists(a_addr), "AddDirectionalLightShadowMap DirectionalLight already destroyed");
    ICARIAN_ASSERT_MSG(a_shadowMapAddr < m_graphicsEngine->m_depthRenderTextures.Size(), "AddDirectionalLightShadowMap DepthShadowMap out of bounds");
    ICARIAN_ASSERT_MSG(m_graphicsEngine->m_depthRenderTextures.Exists(a_shadowMapAddr), "AddDirectionalLightShadowMap DepthShadowMap already destroyed");

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
    ICARIAN_ASSERT_MSG(a_addr < m_graphicsEngine->m_directionalLights.Size(), "RemoveDirectionalLightShadowMap DirectionalLight out of bounds");
    ICARIAN_ASSERT_MSG(m_graphicsEngine->m_directionalLights.Exists(a_addr), "RemoveDirectionalLightShadowMap DirectionalLight already destroyed");
    ICARIAN_ASSERT_MSG(a_shadowMapAddr < m_graphicsEngine->m_depthRenderTextures.Size(), "RemoveDirectionalLightShadowMap DepthShadowMap out of bounds");
    ICARIAN_ASSERT_MSG(m_graphicsEngine->m_depthRenderTextures.Exists(a_shadowMapAddr), "RemoveDirectionalLightShadowMap DepthShadowMap already destroyed");

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
    ICARIAN_ASSERT_MSG(a_transformAddr != -1, "GeneratePointLightBuffer no transform");
    
    PointLightBuffer buffer;
    buffer.TransformAddr = a_transformAddr;
    buffer.Color = glm::vec4(1.0f);
    buffer.Radius = 1.0f;
    buffer.Intensity = 1.0f;
    buffer.RenderLayer = 0b1;

    VulkanLightBuffer* lightBuffer = new VulkanLightBuffer();
    lightBuffer->LightRenderTextureCount = 0;
    lightBuffer->LightRenderTextures = nullptr;
    buffer.Data = lightBuffer;

    return m_graphicsEngine->m_pointLights.PushVal(buffer);
}
void VulkanGraphicsEngineBindings::SetPointLightBuffer(uint32_t a_addr, const PointLightBuffer& a_buffer) const
{
    ICARIAN_ASSERT_MSG(a_addr < m_graphicsEngine->m_pointLights.Size(), "SetPointLightBuffer out of bounds");

    m_graphicsEngine->m_pointLights.LockSet(a_addr, a_buffer);
}
PointLightBuffer VulkanGraphicsEngineBindings::GetPointLightBuffer(uint32_t a_addr) const
{
    ICARIAN_ASSERT_MSG(a_addr < m_graphicsEngine->m_pointLights.Size(), "GetPointLightBuffer out of bounds");

    return m_graphicsEngine->m_pointLights[a_addr];
}
void VulkanGraphicsEngineBindings::DestroyPointLightBuffer(uint32_t a_addr) const
{
    class VulkanPointLightDeletionObject : public DeletionObject
    {
    private:
        uint32_t m_addr;

    protected:

    public:
        VulkanPointLightDeletionObject(uint32_t a_addr)
        {
            m_addr = a_addr;
        }
        virtual ~VulkanPointLightDeletionObject()
        {

        }

        virtual void Destroy()
        {
            ICARIAN_ASSERT_MSG(m_addr < Instance->m_graphicsEngine->m_pointLights.Size(), "DestroyPointLightBuffer out of bounds");
            ICARIAN_ASSERT_MSG(Instance->m_graphicsEngine->m_pointLights.Exists(m_addr), "DestroyPointLightBuffer already destroyed");

            const PointLightBuffer buffer = Instance->m_graphicsEngine->m_pointLights[m_addr];
            const VulkanLightBuffer* data = (VulkanLightBuffer*)buffer.Data;
            IDEFER(delete data);

            Instance->m_graphicsEngine->m_pointLights.Erase(m_addr);
        }
    };

    DeletionQueue::Push(new VulkanPointLightDeletionObject(a_addr), DeletionIndex_Render);
}
void VulkanGraphicsEngineBindings::SetPointLightShadowMap(uint32_t a_addr, uint32_t a_shadowMapAddr) const
{
    ICARIAN_ASSERT_MSG(a_addr < m_graphicsEngine->m_pointLights.Size(), "SetPointLightShadowMap PointLight out of bounds");
    ICARIAN_ASSERT_MSG(m_graphicsEngine->m_pointLights.Exists(a_addr), "SetPointLightShadowMap PointLight already destroyed");
    
    TLockArray<PointLightBuffer> a = m_graphicsEngine->m_pointLights.ToLockArray();

    const PointLightBuffer& buffer = a[a_addr];

    ICARIAN_ASSERT(buffer.Data != nullptr);

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
        ICARIAN_ASSERT_MSG(a_shadowMapAddr < m_graphicsEngine->m_depthCubeRenderTextures.Size(), "SetPointLightShadowMap DepthShadowMap out of bounds");
        ICARIAN_ASSERT_MSG(m_graphicsEngine->m_depthCubeRenderTextures.Exists(a_shadowMapAddr), "SetPointLightShadowMap DepthShadowMap already destroyed");

        uint32_t* renderTextures = new uint32_t[1];

        renderTextures[0] = a_shadowMapAddr;

        lightBuffer->LightRenderTextures = renderTextures;
        lightBuffer->LightRenderTextureCount = 1;
    }
}
uint32_t VulkanGraphicsEngineBindings::GetPointLightShadowMap(uint32_t a_addr) const
{
    ICARIAN_ASSERT_MSG(a_addr < m_graphicsEngine->m_pointLights.Size(), "GetPointLightShadowMap PointLight out of bounds");
    ICARIAN_ASSERT_MSG(m_graphicsEngine->m_pointLights.Exists(a_addr), "GetPointLightShadowMap PointLight already destroyed");

    const PointLightBuffer& buffer = m_graphicsEngine->m_pointLights[a_addr];

    ICARIAN_ASSERT(buffer.Data != nullptr);

    const VulkanLightBuffer* lightBuffer = (VulkanLightBuffer*)buffer.Data;
    if (lightBuffer->LightRenderTextureCount == 0)
    {
        return -1;
    }

    return lightBuffer->LightRenderTextures[0];
}

uint32_t VulkanGraphicsEngineBindings::GenerateSpotLightBuffer(uint32_t a_transformAddr) const
{
    ICARIAN_ASSERT_MSG(a_transformAddr != -1, "GenerateSpotLightBuffer no tranform");
    
    SpotLightBuffer buffer;
    buffer.TransformAddr = a_transformAddr;
    buffer.Color = glm::vec4(1.0f);
    buffer.Radius = 1.0f;
    buffer.Intensity = 1.0f;
    buffer.CutoffAngle = glm::vec2(1.0f, 1.5f);
    buffer.RenderLayer = 0b1;
    
    VulkanLightBuffer* lightBuffer = new VulkanLightBuffer();
    lightBuffer->LightRenderTextureCount = 0;
    lightBuffer->LightRenderTextures = nullptr;
    buffer.Data = lightBuffer;

    return m_graphicsEngine->m_spotLights.PushVal(buffer);
}
void VulkanGraphicsEngineBindings::SetSpotLightBuffer(uint32_t a_addr, const SpotLightBuffer& a_buffer) const
{
    ICARIAN_ASSERT_MSG(a_addr < m_graphicsEngine->m_spotLights.Size(), "SetSpotLightBuffer out of bounds");
    ICARIAN_ASSERT_MSG(m_graphicsEngine->m_spotLights.Exists(a_addr), "SetSpotLightBuffer already destroyed");

    m_graphicsEngine->m_spotLights.LockSet(a_addr, a_buffer);
}
SpotLightBuffer VulkanGraphicsEngineBindings::GetSpotLightBuffer(uint32_t a_addr) const
{
    ICARIAN_ASSERT_MSG(a_addr < m_graphicsEngine->m_spotLights.Size(), "GetSpotLightBuffer out of bounds");
    ICARIAN_ASSERT_MSG(m_graphicsEngine->m_spotLights.Exists(a_addr), "GetSpotLightBuffer already destroyed");

    return m_graphicsEngine->m_spotLights[a_addr];
}
void VulkanGraphicsEngineBindings::DestroySpotLightBuffer(uint32_t a_addr) const
{
    class VulkanSpotLightDeletionObject : public DeletionObject
    {
    private:
        uint32_t m_addr;

    protected:

    public:
        VulkanSpotLightDeletionObject(uint32_t a_addr)
        {
            m_addr = a_addr;
        }
        virtual ~VulkanSpotLightDeletionObject()
        {

        }

        virtual void Destroy()
        {
            ICARIAN_ASSERT_MSG(m_addr < Instance->m_graphicsEngine->m_spotLights.Size(), "DestroySpotLightBuffer out of bounds");
            ICARIAN_ASSERT_MSG(Instance->m_graphicsEngine->m_spotLights.Exists(m_addr), "DestroySpotLightBuffer already destroyed");

            const SpotLightBuffer buffer = Instance->m_graphicsEngine->m_spotLights[m_addr];
            const VulkanLightBuffer* data = (VulkanLightBuffer*)buffer.Data;
            IDEFER(delete data);

            Instance->m_graphicsEngine->m_spotLights.Erase(m_addr);
        }
    };

    DeletionQueue::Push(new VulkanSpotLightDeletionObject(a_addr), DeletionIndex_Render);   
}
void VulkanGraphicsEngineBindings::SetSpotLightShadowMap(uint32_t a_addr, uint32_t a_shadowMapAddr) const
{
    ICARIAN_ASSERT_MSG(a_addr < m_graphicsEngine->m_spotLights.Size(), "SetSpotLightShadowMap SpotLight out of bounds");
    ICARIAN_ASSERT_MSG(m_graphicsEngine->m_spotLights.Exists(a_addr), "SetSpotLightShadowMap SpotLight already destroyed");

    TLockArray<SpotLightBuffer> a = m_graphicsEngine->m_spotLights.ToLockArray();

    const SpotLightBuffer& buffer = a[a_addr];
    ICARIAN_ASSERT(buffer.Data != nullptr);

    VulkanLightBuffer* lightBuffer = (VulkanLightBuffer*)buffer.Data;

    uint32_t* renderTextures = lightBuffer->LightRenderTextures;

    lightBuffer->LightRenderTextures = nullptr;
    lightBuffer->LightRenderTextureCount = 0;

    if (a_shadowMapAddr != -1)
    {
        ICARIAN_ASSERT_MSG(a_shadowMapAddr < m_graphicsEngine->m_depthRenderTextures.Size(), "SetSpotLightShadowMap DepthShadowMap out of bounds");
        ICARIAN_ASSERT_MSG(m_graphicsEngine->m_depthRenderTextures.Exists(a_shadowMapAddr), "SetSpotLightShadowMap DepthShadowMap already destroyed");

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
    ICARIAN_ASSERT_MSG(a_addr < m_graphicsEngine->m_spotLights.Size(), "GetSpotLightShadowMap SpotLight out of bounds");
    ICARIAN_ASSERT_MSG(m_graphicsEngine->m_spotLights.Exists(a_addr), "GetSpotLightShadowMap SpotLight already destroyed");

    const SpotLightBuffer& buffer = m_graphicsEngine->m_spotLights[a_addr];
    ICARIAN_ASSERT(buffer.Data != nullptr);

    const VulkanLightBuffer* lightBuffer = (VulkanLightBuffer*)buffer.Data;
    if (lightBuffer->LightRenderTextureCount == 0)
    {
        return -1;
    }

    return lightBuffer->LightRenderTextures[0];
}

uint32_t VulkanGraphicsEngineBindings::GenerateFont(const std::string_view& a_path) const
{
    Font* font = Font::LoadFont(a_path.data());

    {
        TLockArray<Font*> a = m_graphicsEngine->m_fonts.ToLockArray();

        const uint32_t size = a.Size();
        for (uint32_t i = 0; i < size; ++i)
        {
            if (a[i] == nullptr)
            {
                a[i] = font;

                return i;
            }
        }
    }

    return m_graphicsEngine->m_fonts.PushVal(font);
}
void VulkanGraphicsEngineBindings::DestroyFont(uint32_t a_addr) const
{
    ICARIAN_ASSERT_MSG(a_addr < m_graphicsEngine->m_fonts.Size(), "DestroyFont out of bounds");

    Font* font = m_graphicsEngine->m_fonts[a_addr];
    m_graphicsEngine->m_fonts[a_addr] = nullptr;
    delete font;
}

uint32_t VulkanGraphicsEngineBindings::GenerateCanvasRenderer() const
{
    CanvasRendererBuffer canvas;
    canvas.Flags = 0;
    canvas.CanvasAddr = -1;
    canvas.RenderTextureAddr = -1;

    {
        TLockArray<CanvasRendererBuffer> a = m_graphicsEngine->m_canvasRenderers.ToLockArray();

        const uint32_t size = a.Size();
        for (uint32_t i = 0; i < size; ++i)
        {
            if (a[i].IsDestroyed())
            {
                a[i] = canvas;

                return i;
            }
        }
    }

    return m_graphicsEngine->m_canvasRenderers.PushVal(canvas);
}
void VulkanGraphicsEngineBindings::DestroyCanvasRenderer(uint32_t a_addr) const
{
    ICARIAN_ASSERT_MSG(a_addr < m_graphicsEngine->m_canvasRenderers.Size(), "DestroyCanvasRenderer out of bounds");
    ICARIAN_ASSERT_MSG(!m_graphicsEngine->m_canvasRenderers[a_addr].IsDestroyed(), "DestroyCanvasRenderer canvas renderer destroyed");

    CanvasRendererBuffer nullBuffer;
    nullBuffer.Flags = 0b1 << CanvasRendererBuffer::DestroyedBit;

    m_graphicsEngine->m_canvasRenderers.LockSet(a_addr, nullBuffer);
}
void VulkanGraphicsEngineBindings::SetCanvasRendererCanvas(uint32_t a_addr, uint32_t a_canvasAddr) const
{
    ICARIAN_ASSERT_MSG(a_addr < m_graphicsEngine->m_canvasRenderers.Size(), "SetCanvasRendererCanvas out of bounds");
    ICARIAN_ASSERT_MSG(!m_graphicsEngine->m_canvasRenderers[a_addr].IsDestroyed(), "SetCanvasRenderer canvas renderer destroyed");
    
    CanvasRendererBuffer& buffer = m_graphicsEngine->m_canvasRenderers[a_addr];
    buffer.CanvasAddr = a_canvasAddr;
}
uint32_t VulkanGraphicsEngineBindings::GetCanvasRendererCanvas(uint32_t a_addr) const
{
    ICARIAN_ASSERT_MSG(a_addr < m_graphicsEngine->m_canvasRenderers.Size(), "GetCanvasRenderer out of bounds");
    ICARIAN_ASSERT_MSG(!m_graphicsEngine->m_canvasRenderers[a_addr].IsDestroyed(), "GetCanvasRenderer canvas renderer destroyed");

    return m_graphicsEngine->m_canvasRenderers[a_addr].CanvasAddr;
}
void VulkanGraphicsEngineBindings::SetCanvasRendererRenderTexture(uint32_t a_addr, uint32_t a_renderTextureAddr) const
{
    ICARIAN_ASSERT_MSG(a_addr < m_graphicsEngine->m_canvasRenderers.Size(), "SetCanvasRendererRenderTexture Canvas out of bounds");
    ICARIAN_ASSERT_MSG(!m_graphicsEngine->m_canvasRenderers[a_addr].IsDestroyed(), "SetCanvasRendererRenderTexture canvas renderer destroyed");
    if (a_renderTextureAddr != -1)
    {
        ICARIAN_ASSERT_MSG(a_renderTextureAddr < m_graphicsEngine->m_renderTextures.Size(), "SetCanvasRenderTexture Render Texture out of bounds");
        ICARIAN_ASSERT_MSG(m_graphicsEngine->m_renderTextures[a_renderTextureAddr] != nullptr, "SetCanvasRenderRenderTexture Render Texture destroyer");
    }

    CanvasRendererBuffer& buffer = m_graphicsEngine->m_canvasRenderers[a_addr];
    buffer.RenderTextureAddr = a_renderTextureAddr;
}
uint32_t VulkanGraphicsEngineBindings::GetCanvasRendererRenderTexture(uint32_t a_addr) const
{
    ICARIAN_ASSERT_MSG(a_addr < m_graphicsEngine->m_canvasRenderers.Size(), "GetCanvasRendererRenderTexture out of bounds");
    ICARIAN_ASSERT_MSG(!m_graphicsEngine->m_canvasRenderers[a_addr].IsDestroyed(), "GetCanvasRenderRenderTexuture canvas renderer destroyed");

    return m_graphicsEngine->m_canvasRenderers[a_addr].RenderTextureAddr;
}

void VulkanGraphicsEngineBindings::BindMaterial(uint32_t a_addr) const
{
    ICARIAN_ASSERT_MSG(m_graphicsEngine->m_renderCommands.Exists(), "BindMaterial RenderCommand does not exist");
    if (a_addr != -1)
    {
        ICARIAN_ASSERT_MSG(a_addr < m_graphicsEngine->m_shaderPrograms.Size(), "BindMaterial out of bounds");
    }

    m_graphicsEngine->m_renderCommands->BindMaterial(a_addr);
}
void VulkanGraphicsEngineBindings::PushTexture(uint32_t a_slot, uint32_t a_samplerAddr) const
{
    ICARIAN_ASSERT_MSG_R(a_samplerAddr < m_graphicsEngine->m_textureSampler.Size(), "PushTexture sampler out of bounds");

    m_graphicsEngine->m_renderCommands->PushTexture(a_slot, m_graphicsEngine->m_textureSampler[a_samplerAddr]);
}
void VulkanGraphicsEngineBindings::BindRenderTexture(uint32_t a_addr) const
{
    ICARIAN_ASSERT_MSG(m_graphicsEngine->m_renderCommands.Exists(), "BindRenderTexture RenderCommand does not exist");

    VulkanRenderTexture* tex = nullptr;
    if (a_addr != -1)
    {
        ICARIAN_ASSERT_MSG(a_addr < m_graphicsEngine->m_renderTextures.Size(), "BindRenderTexture out of bounds");

        tex = m_graphicsEngine->m_renderTextures[a_addr];
    }

    m_graphicsEngine->m_renderCommands->BindRenderTexture(a_addr);
}
void VulkanGraphicsEngineBindings::BlitRTRT(uint32_t a_srcAddr, uint32_t a_dstAddr) const
{
    ICARIAN_ASSERT_MSG(m_graphicsEngine->m_renderCommands.Exists(), "BlitRTRT RenderCommand does not exist");
    
    VulkanRenderTexture* srcTex = nullptr;
    VulkanRenderTexture* dstTex = nullptr;

    if (a_srcAddr != -1)
    {
        ICARIAN_ASSERT_MSG(a_srcAddr < m_graphicsEngine->m_renderTextures.Size(), "BlitRTRT source out of bounds");

        srcTex = m_graphicsEngine->m_renderTextures[a_srcAddr];
    }
    if (a_dstAddr != -1)
    {
        ICARIAN_ASSERT_MSG(a_dstAddr < m_graphicsEngine->m_renderTextures.Size(), "BlitRTRT destination out of bounds");

        dstTex = m_graphicsEngine->m_renderTextures[a_dstAddr];
    }

    m_graphicsEngine->m_renderCommands->Blit(srcTex, dstTex);
}
void VulkanGraphicsEngineBindings::DrawMaterial()
{
    ICARIAN_ASSERT_MSG(m_graphicsEngine->m_renderCommands.Exists(), "DrawMaterial RenderCommand does not exist");

    m_graphicsEngine->m_renderCommands->DrawMaterial();
}
void VulkanGraphicsEngineBindings::DrawModel(const glm::mat4& a_transform, uint32_t a_addr)
{
    ICARIAN_ASSERT_MSG(m_graphicsEngine->m_renderCommands.Exists(), "DrawModel RenderCommand does not exist");

    m_graphicsEngine->m_renderCommands->DrawModel(a_transform, a_addr);
}

void VulkanGraphicsEngineBindings::SetLightLVP(const glm::mat4* a_lvp, uint32_t a_lvpCount) const
{
    ICARIAN_ASSERT_MSG(m_graphicsEngine->m_lightData.Exists(), "SetLightLVP LightData does not exist");

    m_graphicsEngine->m_lightData->SetLVP(a_lvp, a_lvpCount);
}
void VulkanGraphicsEngineBindings::SetLightSplits(const float* a_splits, uint32_t a_splitCount) const
{
    ICARIAN_ASSERT_MSG(m_graphicsEngine->m_lightData.Exists(), "SetLightSplits LightData does not exist");

    m_graphicsEngine->m_lightData->SetSplits(a_splits, a_splitCount);
}

#endif