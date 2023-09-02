#ifdef ICARIANNATIVE_ENABLE_GRAPHICS_VULKAN

#include "Rendering/Vulkan/VulkanGraphicsEngineBindings.h"

#include <fstream>
#include <sstream>
#include <stb_image.h>

#include "Flare/ColladaLoader.h"
#include "Flare/IcarianAssert.h"
#include "Flare/IcarianDefer.h"
#include "Flare/OBJLoader.h"
#include "ObjectManager.h"
#include "Rendering/Vulkan/VulkanLightBuffer.h"
#include "Rendering/Vulkan/VulkanLightData.h"
#include "Shaders/DirectionalLightPixel.h"
#include "Shaders/PointLightPixel.h"
#include "Shaders/PostPixel.h"
#include "Shaders/QuadVertex.h"
#include "Shaders/SpotLightPixel.h"
#include "Rendering/RenderEngine.h"
#include "Rendering/UI/Font.h"
#include "Rendering/Vulkan/VulkanGraphicsEngine.h"
#include "Rendering/Vulkan/VulkanModel.h"
#include "Rendering/Vulkan/VulkanPixelShader.h"
#include "Rendering/Vulkan/VulkanRenderCommand.h"
#include "Rendering/Vulkan/VulkanRenderEngineBackend.h"
#include "Rendering/Vulkan/VulkanRenderTexture.h"
#include "Rendering/Vulkan/VulkanShaderData.h"
#include "Rendering/Vulkan/VulkanTexture.h"
#include "Rendering/Vulkan/VulkanTextureSampler.h"
#include "Rendering/Vulkan/VulkanVertexShader.h"
#include "Runtime/RuntimeManager.h"
#include "Trace.h"

static VulkanGraphicsEngineBindings* Engine = nullptr;

#define VULKANGRAPHICS_RUNTIME_ATTACH(ret, namespace, klass, name, code, ...) a_runtime->BindFunction(RUNTIME_FUNCTION_STRING(namespace, klass, name), (void*)RUNTIME_FUNCTION_NAME(klass, name));

// The lazy part of me won against the part that wants to write clean code
// My apologies to the poor soul that has to decipher this definition
#define VULKANGRAPHICS_BINDING_FUNCTION_TABLE(F) \
    F(void, IcarianEngine.Rendering, VertexShader, DestroyShader, { Engine->DestroyVertexShader(a_addr); }, uint32_t a_addr) \
    F(void, IcarianEngine.Rendering, PixelShader, DestroyShader, { Engine->DestroyPixelShader(a_addr); }, uint32_t a_addr) \
    \
    F(uint32_t, IcarianEngine.Rendering, Material, GenerateInternalProgram, { return Engine->GenerateInternalShaderProgram(a_renderProgram); }, FlareBase::e_InternalRenderProgram a_renderProgram) \
    F(FlareBase::RenderProgram, IcarianEngine.Rendering, Material, GetProgramBuffer, { return Engine->GetRenderProgram(a_addr); }, uint32_t a_addr) \
    F(void, IcarianEngine.Rendering, Material, SetProgramBuffer, { Engine->SetRenderProgram(a_addr, a_program); }, uint32_t a_addr, FlareBase::RenderProgram a_program) \
    F(void, IcarianEngine.Rendering, Material, SetTexture, { Engine->RenderProgramSetTexture(a_addr, a_shaderSlot, a_samplerAddr); }, uint32_t a_addr, uint32_t a_shaderSlot, uint32_t a_samplerAddr) \
    \
    F(uint32_t, IcarianEngine.Rendering, Camera, GenerateBuffer, { return Engine->GenerateCameraBuffer(a_transformAddr); }, uint32_t a_transformAddr) \
    F(void, IcarianEngine.Rendering, Camera, DestroyBuffer, { Engine->DestroyCameraBuffer(a_addr); }, uint32_t a_addr) \
    F(CameraBuffer, IcarianEngine.Rendering, Camera, GetBuffer, { return Engine->GetCameraBuffer(a_addr); }, uint32_t a_addr) \
    F(void, IcarianEngine.Rendering, Camera, SetBuffer, { Engine->SetCameraBuffer(a_addr, a_buffer); }, uint32_t a_addr, CameraBuffer a_buffer) \
    F(glm::vec3, IcarianEngine.Rendering, Camera, ScreenToWorld, { return Engine->CameraScreenToWorld(a_addr, a_screenPos, a_screenSize); }, uint32_t a_addr, glm::vec3 a_screenPos, glm::vec2 a_screenSize) \
    \
    F(uint32_t, IcarianEngine.Rendering, MeshRenderer, GenerateBuffer, { return Engine->GenerateMeshRenderBuffer(a_materialAddr, a_modelAddr, a_transformAddr); }, uint32_t a_transformAddr, uint32_t a_materialAddr, uint32_t a_modelAddr) \
    F(void, IcarianEngine.Rendering, MeshRenderer, DestroyBuffer, { Engine->DestroyMeshRenderBuffer(a_addr); }, uint32_t a_addr) \
    F(void, IcarianEngine.Rendering, MeshRenderer, GenerateRenderStack, { Engine->GenerateRenderStack(a_addr); }, uint32_t a_addr) \
    F(void, IcarianEngine.Rendering, MeshRenderer, DestroyRenderStack, { Engine->DestroyRenderStack(a_addr); }, uint32_t a_addr) \
    \
    F(uint32_t, IcarianEngine.Rendering.Animation, SkinnedMeshRenderer, GenerateBuffer, { return Engine->GenerateSkinnedMeshRenderBuffer(a_materialAddr, a_modelAddr, a_transformAddr, a_skeletonAddr); }, uint32_t a_transformAddr, uint32_t a_materialAddr, uint32_t a_modelAddr, uint32_t a_skeletonAddr) \
    F(void, IcarianEngine.Rendering.Animation, SkinnedMeshRenderer, DestroyBuffer, { Engine->DestroySkinnedMeshRenderBuffer(a_addr); }, uint32_t a_addr) \
    F(void, IcarianEngine.Rendering.Animation, SkinnedMeshRenderer, GenerateRenderStack, { Engine->GenerateSkinnedRenderStack(a_addr); }, uint32_t a_addr) \
    F(void, IcarianEngine.Rendering.Animation, SkinnedMeshRenderer, DestroyRenderStack, { Engine->DestroySkinnedRenderStack(a_addr); }, uint32_t a_addr) \
    \
    F(void, IcarianEngine.Rendering, Texture, DestroyTexture, { Engine->DestroyTexture(a_addr); }, uint32_t a_addr) \
    \
    F(uint32_t, IcarianEngine.Rendering, TextureSampler, GenerateTextureSampler, { return Engine->GenerateTextureSampler(a_texture, (FlareBase::e_TextureFilter)a_filter, (FlareBase::e_TextureAddress)a_addressMode ); }, uint32_t a_texture, uint32_t a_filter, uint32_t a_addressMode) \
    F(uint32_t, IcarianEngine.Rendering, TextureSampler, GenerateRenderTextureSampler, { return Engine->GenerateRenderTextureSampler(a_renderTexture, a_textureIndex, (FlareBase::e_TextureFilter)a_filter, (FlareBase::e_TextureAddress)a_addressMode); }, uint32_t a_renderTexture, uint32_t a_textureIndex, uint32_t a_filter, uint32_t a_addressMode) \
    F(uint32_t, IcarianEngine.Rendering, TextureSampler, GenerateRenderTextureDepthSampler, { return Engine->GenerateRenderTextureDepthSampler(a_renderTexture, (FlareBase::e_TextureFilter)a_filter, (FlareBase::e_TextureAddress)a_addressMode); }, uint32_t a_renderTexture, uint32_t a_filter, uint32_t a_addressMode) \
    F(uint32_t, IcarianEngine.Rendering, TextureSampler, GenerateRenderTextureDepthSamplerDepth, { return Engine->GenerateRenderTextureDepthSamplerDepth(a_renderTexture, (FlareBase::e_TextureFilter)a_filter, (FlareBase::e_TextureAddress)a_addressMode); }, uint32_t a_renderTexture, uint32_t a_filter, uint32_t a_addressMode) \
    F(void, IcarianEngine.Rendering, TextureSampler, DestroySampler, { Engine->DestroyTextureSampler(a_addr); }, uint32_t a_addr) \
    \
    F(uint32_t, IcarianEngine.Rendering, RenderTextureCmd, GenerateRenderTexture, { return Engine->GenerateRenderTexture(a_count, a_width, a_height, (bool)a_depthTexture, (bool)a_hdr); }, uint32_t a_count, uint32_t a_width, uint32_t a_height, uint32_t a_depthTexture, uint32_t a_hdr) \
    F(void, IcarianEngine.Rendering, RenderTextureCmd, DestroyRenderTexture, { return Engine->DestroyRenderTexture(a_addr); }, uint32_t a_addr) \
    F(uint32_t, IcarianEngine.Rendering, RenderTextureCmd, HasDepth, { return (uint32_t)Engine->RenderTextureHasDepth(a_addr); }, uint32_t a_addr) \
    F(uint32_t, IcarianEngine.Rendering, RenderTextureCmd, GetWidth, { return Engine->GetRenderTextureWidth(a_addr); }, uint32_t a_addr) \
    F(uint32_t, IcarianEngine.Rendering, RenderTextureCmd, GetHeight, { return Engine->GetRenderTextureHeight(a_addr); }, uint32_t a_addr) \
    F(void, IcarianEngine.Rendering, RenderTextureCmd, Resize, { return Engine->ResizeRenderTexture(a_addr, a_width, a_height); }, uint32_t a_addr, uint32_t a_width, uint32_t a_height) \
    \
    F(uint32_t, IcarianEngine.Rendering, DepthRenderTexture, GenerateRenderTexture, {  return Engine->GenerateDepthRenderTexture(a_width, a_height); }, uint32_t a_width, uint32_t a_height) \
    F(void, IcarianEngine.Rendering, DepthRenderTexture, DestroyRenderTexture, { Engine->DestroyDepthRenderTexture(a_addr); }, uint32_t a_addr) \
    \
    F(uint32_t, IcarianEngine.Renddering, MultiRenderTexture, GetTextureCount, { return Engine->GetRenderTextureTextureCount(a_addr); }, uint32_t a_addr) \
    \
    F(void, IcarianEngine.Rendering, Model, DestroyModel, { Engine->DestroyModel(a_addr); }, uint32_t a_addr) \
    \
    F(uint32_t, IcarianEngine.Rendering.Lighting, DirectionalLight, GenerateBuffer, { return Engine->GenerateDirectionalLightBuffer(a_transformAddr); }, uint32_t a_transformAddr) \
    F(void, IcarianEngine.Rendering.Lighting, DirectionalLight, DestroyBuffer, { Engine->DestroyDirectionalLightBuffer(a_addr); }, uint32_t a_addr) \
    F(DirectionalLightBuffer, IcarianEngine.Rendering.Lighting, DirectionalLight, GetBuffer, { return Engine->GetDirectionalLightBuffer(a_addr); }, uint32_t a_addr) \
    F(void, IcarianEngine.Rendering.Lighting, DirectionalLight, SetBuffer, { Engine->SetDirectionalLightBuffer(a_addr, a_buffer); }, uint32_t a_addr, DirectionalLightBuffer a_buffer) \
    F(void, IcarianEngine.Rendering.Lighting, DirectionalLight, AddShadowMap, { Engine->AddDirectionalLightShadowMap(a_addr, a_shadowMapAddr); }, uint32_t a_addr, uint32_t a_shadowMapAddr) \
    F(void, IcarianEngine.Rendering.Lighting, DirectionalLight, RemoveShadowMap, { Engine->RemoveDirectionalLightShadowMap(a_addr, a_shadowMapAddr); }, uint32_t a_addr, uint32_t a_shadowMapAddr) \
    \
    F(uint32_t, IcarianEngine.Rendering.Lighting, PointLight, GenerateBuffer, { return Engine->GeneratePointLightBuffer(a_transformAddr); }, uint32_t a_transformAddr) \
    F(void, IcarianEngine.Rendering.Lighting, PointLight, DestroyBuffer, { Engine->DestroyPointLightBuffer(a_addr); }, uint32_t a_addr) \
    F(PointLightBuffer, IcarianEngine.Rendering.Lighting, PointLight, GetBuffer, { return Engine->GetPointLightBuffer(a_addr); }, uint32_t a_addr) \
    F(void, IcarianEngine.Rendering.Lighting, PointLight, SetBuffer, { Engine->SetPointLightBuffer(a_addr, a_buffer); }, uint32_t a_addr, PointLightBuffer a_buffer) \
    \
    F(uint32_t, IcarianEngine.Rendering.Lighting, SpotLight, GenerateBuffer, { return Engine->GenerateSpotLightBuffer(a_transformAddr); }, uint32_t a_transformAddr) \
    F(void, IcarianEngine.Rendering.Lighting, SpotLight, DestroyBuffer, { Engine->DestroySpotLightBuffer(a_addr); }, uint32_t a_addr) \
    F(SpotLightBuffer, IcarianEngine.Rendering.Lighting, SpotLight, GetBuffer, { return Engine->GetSpotLightBuffer(a_addr); }, uint32_t a_addr) \
    F(void, IcarianEngine.Rendering.Lighting, SpotLight, SetBuffer, { Engine->SetSpotLightBuffer(a_addr, a_buffer); }, uint32_t a_addr, SpotLightBuffer a_buffer) \
    \
    F(uint32_t, IcarianEngine.Rendering.UI, Font, GenerateFont, { char* str = mono_string_to_utf8(a_path); IDEFER(mono_free(str)); return Engine->GenerateFont(str); }, MonoString* a_path) \
    F(void, IcarianEngine.Rendering.UI, Font, DestroyFont, { Engine->DestroyFont(a_addr); }, uint32_t a_addr) \
    \
    F(uint32_t, IcarianEngine.Rendering.UI, CanvasRenderer, GenerateBuffer, { return Engine->GenerateCanvasRenderer(); }) \
    F(void, IcarianEngine.Rendering.UI, CanvasRenderer, DestroyBuffer, { Engine->DestroyCanvasRenderer(a_addr); }, uint32_t a_addr) \
    F(void, IcarianEngine.Rendering.UI, CanvasRenderer, SetCanvas, { Engine->SetCanvasRendererCanvas(a_addr, a_canvasAddr); }, uint32_t a_addr, uint32_t a_canvasAddr) \
    F(uint32_t, IcarianEngine.Rendering.UI, CanvasRenderer, GetCanvas, { return Engine->GetCanvasRendererCanvas(a_addr); }, uint32_t a_addr) \
    F(void, IcarianEngine.Rendering.UI, CanvasRenderer, SetRenderTexture, { Engine->SetCanvasRendererRenderTexture(a_addr, a_renderTextureAddr); }, uint32_t a_addr, uint32_t a_renderTextureAddr) \
    F(uint32_t, IcarianEngine.Rendering.UI, CanvasRenderer, GetRenderTexture, { return Engine->GetCanvasRendererRenderTexture(a_addr); }, uint32_t a_addr) \
    \
    F(void, IcarianEngine.Rendering, RenderCommand, BindMaterial, { Engine->BindMaterial(a_addr); }, uint32_t a_addr) \
    F(void, IcarianEngine.Rendering, RenderCommand, PushTexture, { Engine->PushTexture(a_slot, a_samplerAddr); }, uint32_t a_slot, uint32_t a_samplerAddr) \
    F(void, IcarianEngine.Rendering, RenderCommand, BindRenderTexture, { Engine->BindRenderTexture(a_addr); }, uint32_t a_addr) \
    F(void, IcarianEngine.Rendering, RenderCommand, RTRTBlit, { Engine->BlitRTRT(a_srcAddr, a_dstAddr); }, uint32_t a_srcAddr, uint32_t a_dstAddr) \
    F(void, IcarianEngine.Rendering, RenderCommand, DrawMaterial, { Engine->DrawMaterial(); }) \
    \
    F(void, IcarianEngine.Rendering.Animation, SkeletonAnimator, PushTransform, { }, uint32_t a_addr, MonoString* a_object, MonoArray* a_transform) \

VULKANGRAPHICS_BINDING_FUNCTION_TABLE(RUNTIME_FUNCTION_DEFINITION)

FLARE_MONO_EXPORT(uint32_t, RUNTIME_FUNCTION_NAME(VertexShader, GenerateFromFile), MonoString* a_path)
{
    char* str = mono_string_to_utf8(a_path);

    const std::filesystem::path p = std::filesystem::path(str);

    mono_free(str);  

    if (p.extension() == ".fvert")
    {
        std::ifstream file = std::ifstream(p);
        if (file.good() && file.is_open())
        {
            std::stringstream ss;

            ss << file.rdbuf();

            file.close();

            return Engine->GenerateFVertexShaderAddr(ss.str());
        }
    }
    else if (p.extension() == ".vert")
    {
        std::ifstream file = std::ifstream(p);
        if (file.good() && file.is_open())
        {
            std::stringstream ss;

            ss << file.rdbuf();

            file.close();

            return Engine->GenerateGLSLVertexShaderAddr(ss.str());
        }
    }

    return -1;
}
FLARE_MONO_EXPORT(uint32_t, RUNTIME_FUNCTION_NAME(PixelShader, GenerateFromFile), MonoString* a_path)
{
    char* str = mono_string_to_utf8(a_path);

    const std::filesystem::path p = std::filesystem::path(str);

    mono_free(str);

    if (p.extension() == ".fpix" || p.extension() == ".ffrag")
    {
        std::ifstream file = std::ifstream(p);
        if (file.good() && file.is_open())
        {
            std::stringstream ss;

            ss << file.rdbuf();

            file.close();

            return Engine->GenerateFPixelShaderAddr(ss.str());
        }
    }
    else if (p.extension() == ".pix" || p.extension() == ".frag")
    {
        std::ifstream file = std::ifstream(p);
        if (file.good() && file.is_open())
        {
            std::stringstream ss;

            ss << file.rdbuf();

            file.close();

            return Engine->GenerateGLSLPixelShaderAddr(ss.str());
        }
    }

    return -1;
}

RUNTIME_FUNCTION(MonoArray*, Camera, GetProjectionMatrix, 
{
    const glm::mat4 proj = Engine->GetCameraProjectionMatrix(a_addr, a_width, a_height);

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
    const glm::mat4 proj = Engine->GetCameraProjectionMatrix(a_addr, a_width, a_height, a_near, a_far);

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
    const DirectionalLightBuffer buffer = Engine->GetDirectionalLightBuffer(a_addr);

    const VulkanLightBuffer* lightBuffer = (VulkanLightBuffer*)buffer.Data;
    
    MonoArray* arr = mono_array_new(mono_domain_get(), mono_get_uint32_class(), lightBuffer->LightRenderTextureCount);

    for (uint32_t i = 0; i < lightBuffer->LightRenderTextureCount; ++i)
    {
        mono_array_set(arr, uint32_t, i, lightBuffer->LightRenderTextures[i].TextureAddr);
    }

    return arr;
}, uint32_t a_addr)

// Gonna leave theses functions seperate as there is a bit to it
FLARE_MONO_EXPORT(uint32_t, RUNTIME_FUNCTION_NAME(Material, GenerateProgram), uint32_t a_vertexShader, uint32_t a_pixelShader, uint16_t a_vertexStride, MonoArray* a_vertexInputAttribs, MonoArray* a_shaderInputs, uint32_t a_cullingMode, uint32_t a_primitiveMode, uint32_t a_colorBlendingEnabled)
{
    FlareBase::RenderProgram program;
    program.VertexShader = a_vertexShader;
    program.PixelShader = a_pixelShader;
    program.VertexStride = a_vertexStride;
    program.CullingMode = (FlareBase::e_CullMode)a_cullingMode;
    program.PrimitiveMode = (FlareBase::e_PrimitiveMode)a_primitiveMode;
    program.EnableColorBlending = (uint8_t)a_colorBlendingEnabled;
    program.Flags = 0;

    // Need to recreate the array
    // Because it is a managed array may not be contiguous and is controlled by the GC 
    // Need a reliable lifetime and memory layout
    if (a_vertexInputAttribs != nullptr)
    {
        program.VertexInputCount = (uint16_t)mono_array_length(a_vertexInputAttribs);
        program.VertexAttribs = new FlareBase::VertexInputAttrib[program.VertexInputCount];

        for (uint16_t i = 0; i < program.VertexInputCount; ++i)
        {
            program.VertexAttribs[i] = mono_array_get(a_vertexInputAttribs, FlareBase::VertexInputAttrib, i);
        }
    }
    else
    {
        program.VertexInputCount = 0;
        program.VertexAttribs = nullptr;
    }
    
    if (a_shaderInputs != nullptr)
    {
        program.ShaderBufferInputCount = (uint16_t)mono_array_length(a_shaderInputs);
        program.ShaderBufferInputs = new FlareBase::ShaderBufferInput[program.ShaderBufferInputCount];

        for (uint16_t i = 0; i < program.ShaderBufferInputCount; ++i)
        {
            program.ShaderBufferInputs[i] = mono_array_get(a_shaderInputs, FlareBase::ShaderBufferInput, i);
        }
    }
    else
    {
        program.ShaderBufferInputCount = 0;
        program.ShaderBufferInputs = nullptr;
    }

    return Engine->GenerateShaderProgram(program);
}
FLARE_MONO_EXPORT(void, RUNTIME_FUNCTION_NAME(Material, DestroyProgram), uint32_t a_addr)
{
    const FlareBase::RenderProgram program = Engine->GetRenderProgram(a_addr);

    IDEFER(
    {
        if (program.VertexAttribs != nullptr)
        {
            delete[] program.VertexAttribs;
        }
        
        if (program.ShaderBufferInputs != nullptr)
        {
            delete[] program.ShaderBufferInputs;
        }
    });

    Engine->DestroyShaderProgram(a_addr);
}

FLARE_MONO_EXPORT(uint32_t, RUNTIME_FUNCTION_NAME(Texture, GenerateFromFile), MonoString* a_path)
{
    char* str = mono_string_to_utf8(a_path);
    IDEFER(mono_free(str));
    const std::filesystem::path p = std::filesystem::path(str);

    if (p.extension() == ".png")
    {
		int width;
		int height;
		int channels;

		const std::string str = p.string();
		stbi_uc* pixels = stbi_load(str.c_str(), &width, &height, &channels, STBI_rgb_alpha);
		if (pixels != nullptr)
		{
            IDEFER(stbi_image_free(pixels));

			return Engine->GenerateTexture((uint32_t)width, (uint32_t)height, pixels);
		}
    }
    else 
    {
        ICARIAN_ASSERT_MSG_R(0, "GenerateFromFile invalid file type");
    }

    return -1;
}

RUNTIME_FUNCTION(uint32_t, Model, GenerateModel, 
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

    return Engine->GenerateModel(vertices, vertexCount, indices, indexCount, a_vertexStride, a_radius);
}, MonoArray* a_vertices, MonoArray* a_indices, uint16_t a_vertexStride, float a_radius);
RUNTIME_FUNCTION(uint32_t, Model, GenerateFromFile,
{
    char* str = mono_string_to_utf8(a_path);
    IDEFER(mono_free(str));

    std::vector<FlareBase::Vertex> vertices;
    std::vector<uint32_t> indices;
    float radius;
    const std::filesystem::path p = std::filesystem::path(str);
    const std::filesystem::path ext = p.extension();

    if (ext == ".obj")
    {
        if (FlareBase::OBJLoader_LoadFile(p, &vertices, &indices, &radius))
        {
            return Engine->GenerateModel((const char*)vertices.data(), (uint32_t)vertices.size(), indices.data(), (uint32_t)indices.size(), sizeof(FlareBase::Vertex), radius);
        }
    }
    else if (ext == ".dae")
    {
        if (FlareBase::ColladaLoader_LoadFile(p, &vertices, &indices, &radius))
        {
            return Engine->GenerateModel((const char*)vertices.data(), (uint32_t)vertices.size(), indices.data(), (uint32_t)indices.size(), sizeof(FlareBase::Vertex), radius);
        }
    }
    else
    {
        ICARIAN_ASSERT_MSG_R(0, "GenerateFromFile invalid file extension");
    }

    return -1;
}, MonoString* a_path)
RUNTIME_FUNCTION(uint32_t, Model, GenerateSkinnedFromFile, 
{
    char* str = mono_string_to_utf8(a_path);
    IDEFER(mono_free(str));

    std::vector<FlareBase::SkinnedVertex> vertices;
    std::vector<uint32_t> indices;
    float radius;
    const std::filesystem::path p = std::filesystem::path(str);
    const std::filesystem::path ext = p.extension();

    if (ext == ".dae")
    {
        if (FlareBase::ColladaLoader_LoadSkinnedFile(p, &vertices, &indices, &radius))
        {
            return Engine->GenerateModel((const char*)vertices.data(), (uint32_t)vertices.size(), indices.data(), (uint32_t)indices.size(), sizeof(FlareBase::SkinnedVertex), radius);
        }
    }
    else 
    {
        ICARIAN_ASSERT_MSG_R(0, "GenerateSkinnedFromFile invalid file extension");
    }

    return -1;
}, MonoString* a_path)

FLARE_MONO_EXPORT(void, RUNTIME_FUNCTION_NAME(RenderCommand, DrawModel), MonoArray* a_transform, uint32_t a_addr)
{
    glm::mat4 transform;

    float* f = (float*)&transform;
    for (int i = 0; i < 16; ++i)
    {
        f[i] = mono_array_get(a_transform, float, i);
    }

    Engine->DrawModel(transform, a_addr);
}

RUNTIME_FUNCTION(void, RenderPipeline, SetLightLVP,
{
    glm::mat4 lightLVP = glm::mat4(1.0f);

    float* f = (float*)&lightLVP;
    for (int i = 0; i < 16; ++i)
    {
        f[i] = mono_array_get(a_lightLVP, float, i);
    }

    Engine->SetLightLVP(lightLVP);
}, MonoArray* a_lightLVP)

VulkanGraphicsEngineBindings::VulkanGraphicsEngineBindings(RuntimeManager* a_runtime, VulkanGraphicsEngine* a_graphicsEngine)
{
    m_graphicsEngine = a_graphicsEngine;

    Engine = this;

    TRACE("Binding Vulkan functions to C#");
    VULKANGRAPHICS_BINDING_FUNCTION_TABLE(VULKANGRAPHICS_RUNTIME_ATTACH)

    BIND_FUNCTION(a_runtime, IcarianEngine.Rendering, VertexShader, GenerateFromFile);
    BIND_FUNCTION(a_runtime, IcarianEngine.Rendering, PixelShader, GenerateFromFile);

    BIND_FUNCTION(a_runtime, IcarianEngine.Rendering, Camera, GetProjectionMatrix);
    BIND_FUNCTION(a_runtime, IcarianEngine.Rendering, Camera, GetProjectionMatrixNF);

    BIND_FUNCTION(a_runtime, IcarianEngine.Rendering.Lighting, DirectionalLight, GetShadowMaps);

    BIND_FUNCTION(a_runtime, IcarianEngine.Rendering, Material, GenerateProgram);
    BIND_FUNCTION(a_runtime, IcarianEngine.Rendering, Material, DestroyProgram);
    
    BIND_FUNCTION(a_runtime, IcarianEngine.Rendering, Texture, GenerateFromFile);

    BIND_FUNCTION(a_runtime, IcarianEngine.Rendering, Model, GenerateModel);
    BIND_FUNCTION(a_runtime, IcarianEngine.Rendering, Model, GenerateFromFile);
    BIND_FUNCTION(a_runtime, IcarianEngine.Rendering, Model, GenerateSkinnedFromFile);

    BIND_FUNCTION(a_runtime, IcarianEngine.Rendering, RenderCommand, DrawModel);

    BIND_FUNCTION(a_runtime, IcarianEngine.Rendering, RenderPipeline, SetLightLVP);
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

uint32_t VulkanGraphicsEngineBindings::GenerateInternalShaderProgram(FlareBase::e_InternalRenderProgram a_program) const
{
    FlareBase::RenderProgram program;
    program.VertexStride = 0;
    program.VertexInputCount = 0;
    program.VertexAttribs = nullptr;
    program.Flags |= 0b1 << FlareBase::RenderProgram::DestroyFlag;

    switch (a_program)
    {
    case FlareBase::InternalRenderProgram_DirectionalLight:
    {
        TRACE("Creating Directional Light Shader");
        program.VertexShader = GenerateGLSLVertexShaderAddr(QUADVERTEX);
        program.PixelShader = GenerateFPixelShaderAddr(DIRECTIONALLIGHTPIXEL);
        program.CullingMode = FlareBase::CullMode_None;
        program.PrimitiveMode = FlareBase::PrimitiveMode_TriangleStrip;
        program.EnableColorBlending = 1;

        constexpr uint32_t TextureCount = 5;
        constexpr uint32_t BufferCount = TextureCount + 2;

        program.ShaderBufferInputCount = BufferCount;
        program.ShaderBufferInputs = new FlareBase::ShaderBufferInput[BufferCount];
        for (uint32_t i = 0; i < TextureCount; ++i)
        {
            program.ShaderBufferInputs[i] = FlareBase::ShaderBufferInput(i, FlareBase::ShaderBufferType_Texture, FlareBase::ShaderSlot_Pixel);
        }

        program.ShaderBufferInputs[TextureCount + 0] = FlareBase::ShaderBufferInput(TextureCount + 0, FlareBase::ShaderBufferType_DirectionalLightBuffer, FlareBase::ShaderSlot_Pixel, 1);
        program.ShaderBufferInputs[TextureCount + 1] = FlareBase::ShaderBufferInput(TextureCount + 1, FlareBase::ShaderBufferType_CameraBuffer, FlareBase::ShaderSlot_Pixel, 2);

        break;
    }
    case FlareBase::InternalRenderProgram_PointLight:
    {
        TRACE("Creating Point Light Shader");
        program.VertexShader = GenerateGLSLVertexShaderAddr(QUADVERTEX);
        program.PixelShader = GenerateFPixelShaderAddr(POINTLIGHTPIXEL);
        program.CullingMode = FlareBase::CullMode_None;
        program.PrimitiveMode = FlareBase::PrimitiveMode_TriangleStrip;
        program.EnableColorBlending = 1;

        constexpr uint32_t TextureCount = 5;
        constexpr uint32_t BufferCount = TextureCount + 2;
        
        program.ShaderBufferInputCount = BufferCount;
        program.ShaderBufferInputs = new FlareBase::ShaderBufferInput[BufferCount];
        for (uint32_t i = 0; i < TextureCount; ++i)
        {
            program.ShaderBufferInputs[i] = FlareBase::ShaderBufferInput(i, FlareBase::ShaderBufferType_Texture, FlareBase::ShaderSlot_Pixel);
        }

        program.ShaderBufferInputs[TextureCount + 0] = FlareBase::ShaderBufferInput(TextureCount + 0, FlareBase::ShaderBufferType_PointLightBuffer, FlareBase::ShaderSlot_Pixel, 1);
        program.ShaderBufferInputs[TextureCount + 1] = FlareBase::ShaderBufferInput(TextureCount + 1, FlareBase::ShaderBufferType_CameraBuffer, FlareBase::ShaderSlot_Pixel, 2);

        break;
    }
    case FlareBase::InternalRenderProgram_SpotLight:
    {
        TRACE("Creating Spot Light Shader");
        program.VertexShader = GenerateGLSLVertexShaderAddr(QUADVERTEX);
        program.PixelShader = GenerateFPixelShaderAddr(SPOTLIGHTPIXEL);
        program.CullingMode = FlareBase::CullMode_None;
        program.PrimitiveMode = FlareBase::PrimitiveMode_TriangleStrip;
        program.EnableColorBlending = 1;

        constexpr uint32_t TextureCount = 5;
        constexpr uint32_t BufferCount = TextureCount + 2;

        program.ShaderBufferInputCount = BufferCount;
        program.ShaderBufferInputs = new FlareBase::ShaderBufferInput[BufferCount];
        for (uint32_t i = 0; i < TextureCount; ++i)
        {
            program.ShaderBufferInputs[i] = FlareBase::ShaderBufferInput(i, FlareBase::ShaderBufferType_Texture, FlareBase::ShaderSlot_Pixel);
        }

        program.ShaderBufferInputs[TextureCount + 0] = FlareBase::ShaderBufferInput(TextureCount + 0, FlareBase::ShaderBufferType_SpotLightBuffer, FlareBase::ShaderSlot_Pixel, 1);
        program.ShaderBufferInputs[TextureCount + 1] = FlareBase::ShaderBufferInput(TextureCount + 1, FlareBase::ShaderBufferType_CameraBuffer, FlareBase::ShaderSlot_Pixel, 2);

        break;
    }
    case FlareBase::InternalRenderProgram_Post:
    {
        TRACE("Creating Post Shader");
        program.VertexShader = GenerateGLSLVertexShaderAddr(QUADVERTEX);
        program.PixelShader = GenerateFPixelShaderAddr(POSTPIXEL);
        program.CullingMode = FlareBase::CullMode_None;
        program.PrimitiveMode = FlareBase::PrimitiveMode_TriangleStrip;
        program.EnableColorBlending = 1;

        constexpr uint32_t TextureCount = 4;
        constexpr uint32_t BufferCount = TextureCount + 1;

        program.ShaderBufferInputCount = BufferCount;
        program.ShaderBufferInputs = new FlareBase::ShaderBufferInput[BufferCount];
        for (uint32_t i = 0; i < TextureCount; ++i)
        {
            program.ShaderBufferInputs[i] = FlareBase::ShaderBufferInput(i, FlareBase::ShaderBufferType_Texture, FlareBase::ShaderSlot_Pixel);
        }

        program.ShaderBufferInputs[TextureCount + 0] = FlareBase::ShaderBufferInput(TextureCount + 0, FlareBase::ShaderBufferType_CameraBuffer, FlareBase::ShaderSlot_Pixel, 1);

        break;
    }
    default:
    {
        ICARIAN_ASSERT_MSG(0, "Invalid Internal Render Program");
    }
    }

    return GenerateShaderProgram(program);
}
uint32_t VulkanGraphicsEngineBindings::GenerateShaderProgram(const FlareBase::RenderProgram& a_program) const
{
    return m_graphicsEngine->GenerateRenderProgram(a_program);
}
void VulkanGraphicsEngineBindings::DestroyShaderProgram(uint32_t a_addr) const
{
    m_graphicsEngine->DestroyRenderProgram(a_addr);
}
void VulkanGraphicsEngineBindings::RenderProgramSetTexture(uint32_t a_addr, uint32_t a_shaderSlot, uint32_t a_samplerAddr)
{
    TLockArray<FlareBase::RenderProgram> a = m_graphicsEngine->m_shaderPrograms.ToLockArray();

    ICARIAN_ASSERT_MSG(a_addr < a.Size(), "RenderProgramSetTexture material out of bounds");

    const FlareBase::RenderProgram& program = a[a_addr];

    ICARIAN_ASSERT_MSG(program.Data != nullptr, "RenderProgramSetTexture invalid program");

    VulkanShaderData* data = (VulkanShaderData*)program.Data;
    ICARIAN_ASSERT_MSG(a_samplerAddr < m_graphicsEngine->m_textureSampler.Size(), "RenderProgramSetTexture sampler out of bounds");
    data->SetTexture(a_shaderSlot, m_graphicsEngine->m_textureSampler[a_samplerAddr]);
}
FlareBase::RenderProgram VulkanGraphicsEngineBindings::GetRenderProgram(uint32_t a_addr) const
{
    ICARIAN_ASSERT_MSG(a_addr < m_graphicsEngine->m_shaderPrograms.Size(), "GetRenderProgram out of bounds")

    return m_graphicsEngine->m_shaderPrograms[a_addr];
}
void VulkanGraphicsEngineBindings::SetRenderProgram(uint32_t a_addr, const FlareBase::RenderProgram& a_program) const
{
    ICARIAN_ASSERT_MSG(a_addr < m_graphicsEngine->m_shaderPrograms.Size(), "SetRenderProgram out of bounds")

    m_graphicsEngine->m_shaderPrograms[a_addr] = a_program;
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

    ObjectManager* objManager = m_graphicsEngine->m_vulkanEngine->GetRenderEngine()->GetObjectManager();
    const glm::mat4 invView = objManager->GetGlobalMatrix(camBuf.TransformAddr);

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

uint32_t VulkanGraphicsEngineBindings::GenerateModel(const char* a_vertices, uint32_t a_vertexCount, const uint32_t* a_indices, uint32_t a_indexCount, uint16_t a_vertexStride, float a_radius) const
{
    ICARIAN_ASSERT_MSG(a_vertices != nullptr, "GenerateModel vertices null")
    ICARIAN_ASSERT_MSG(a_vertexCount > 0, "GenerateModel no vertices")
    ICARIAN_ASSERT_MSG(a_indices != nullptr, "GenerateModel indices null")
    ICARIAN_ASSERT_MSG(a_indexCount > 0, "GenerateModel no indices")
    ICARIAN_ASSERT_MSG(a_vertexStride > 0, "GenerateModel vertex stride 0")

    VulkanModel* model = new VulkanModel(m_graphicsEngine->m_vulkanEngine, a_vertexCount, a_vertices, a_vertexStride, a_indexCount, a_indices, a_radius);

    return m_graphicsEngine->m_models.PushVal(model);
}
void VulkanGraphicsEngineBindings::DestroyModel(uint32_t a_addr) const
{
    ICARIAN_ASSERT_MSG(a_addr < m_graphicsEngine->m_models.Size(), "DestroyModel out of bounds")
    ICARIAN_ASSERT_MSG(m_graphicsEngine->m_models.Exists(a_addr), "DestroyModel already destroyed");

    const VulkanModel* model = m_graphicsEngine->m_models[a_addr];
    IDEFER(delete model);
    m_graphicsEngine->m_models.Erase(a_addr);
}

uint32_t VulkanGraphicsEngineBindings::GenerateMeshRenderBuffer(uint32_t a_materialAddr, uint32_t a_modelAddr, uint32_t a_transformAddr) const
{
    TRACE("Creating Render Buffer");
    const MeshRenderBuffer buffer = MeshRenderBuffer(a_materialAddr, a_modelAddr, a_transformAddr);

    return m_graphicsEngine->m_renderBuffers.PushVal(buffer);
}
void VulkanGraphicsEngineBindings::DestroyMeshRenderBuffer(uint32_t a_addr) const
{
    ICARIAN_ASSERT_MSG(a_addr < m_graphicsEngine->m_renderBuffers.Size(), "DestroyMeshRenderBuffer out of bounds");

    TRACE("Destroying Render Buffer");
    m_graphicsEngine->m_renderBuffers.Erase(a_addr);
}
void VulkanGraphicsEngineBindings::GenerateRenderStack(uint32_t a_meshAddr) const
{
    ICARIAN_ASSERT_MSG(a_meshAddr < m_graphicsEngine->m_renderBuffers.Size(), "GenerateRenderStack out of bounds");

    TRACE("Pushing RenderStack");
    const MeshRenderBuffer& buffer = m_graphicsEngine->m_renderBuffers[a_meshAddr];

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

    TRACE("Removing RenderStack");
    const MeshRenderBuffer& buffer = m_graphicsEngine->m_renderBuffers[a_meshAddr];

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

uint32_t VulkanGraphicsEngineBindings::GenerateTexture(uint32_t a_width, uint32_t a_height, const void* a_data)
{
    VulkanTexture* texture = VulkanTexture::CreateRGBA(m_graphicsEngine->m_vulkanEngine, a_width, a_height, a_data);

    {
        TLockArray<VulkanTexture*> a = m_graphicsEngine->m_textures.ToLockArray();

        const uint32_t size = a.Size();
        for (uint32_t i = 0; i < size; ++i)
        {
            if (a[i] == nullptr)
            {
                a[i] = texture;

                return i;
            }
        }
    }

    return m_graphicsEngine->m_textures.PushVal(texture);
}
void VulkanGraphicsEngineBindings::DestroyTexture(uint32_t a_addr) const
{
    m_graphicsEngine->DestroyTexture(a_addr);
}

uint32_t VulkanGraphicsEngineBindings::GenerateTextureSampler(uint32_t a_texture, FlareBase::e_TextureFilter a_filter, FlareBase::e_TextureAddress a_addressMode) const
{
    return m_graphicsEngine->GenerateTextureSampler(a_texture, FlareBase::TextureMode_Texture, a_filter, a_addressMode);
}
uint32_t VulkanGraphicsEngineBindings::GenerateRenderTextureSampler(uint32_t a_renderTexture, uint32_t a_textureIndex, FlareBase::e_TextureFilter a_filter, FlareBase::e_TextureAddress a_addressMode) const
{
    return m_graphicsEngine->GenerateTextureSampler(a_renderTexture, FlareBase::TextureMode_RenderTexture, a_filter, a_addressMode, a_textureIndex);
}
uint32_t VulkanGraphicsEngineBindings::GenerateRenderTextureDepthSampler(uint32_t a_renderTexture, FlareBase::e_TextureFilter a_filter, FlareBase::e_TextureAddress a_addressMode) const
{
    return m_graphicsEngine->GenerateTextureSampler(a_renderTexture, FlareBase::TextureMode_RenderTextureDepth, a_filter, a_addressMode);
}
uint32_t VulkanGraphicsEngineBindings::GenerateRenderTextureDepthSamplerDepth(uint32_t a_renderTexture, FlareBase::e_TextureFilter a_filter, FlareBase::e_TextureAddress a_addressMode) const
{
    return m_graphicsEngine->GenerateTextureSampler(a_renderTexture, FlareBase::TextureMode_DepthRenderTexture, a_filter, a_addressMode);
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

    const VulkanRenderTexture* tex = m_graphicsEngine->m_renderTextures[a_addr];
    IDEFER(delete tex);
    m_graphicsEngine->m_renderTextures.Erase(a_addr);
}
uint32_t VulkanGraphicsEngineBindings::GetRenderTextureTextureCount(uint32_t a_addr) const
{
    ICARIAN_ASSERT_MSG(a_addr < m_graphicsEngine->m_renderTextures.Size(), "GetRenderTextureCount out of bounds");

    const VulkanRenderTexture* texture = m_graphicsEngine->m_renderTextures[a_addr];

    return texture->GetTextureCount();
}
bool VulkanGraphicsEngineBindings::RenderTextureHasDepth(uint32_t a_addr) const
{
    ICARIAN_ASSERT_MSG(a_addr < m_graphicsEngine->m_renderTextures.Size(), "RenderTextureHasDepth out of bounds");

    const VulkanRenderTexture* texture = m_graphicsEngine->m_renderTextures[a_addr];

    return texture->HasDepthTexture();
}
uint32_t VulkanGraphicsEngineBindings::GetRenderTextureWidth(uint32_t a_addr) const
{
    ICARIAN_ASSERT_MSG(a_addr < m_graphicsEngine->m_renderTextures.Size(), "GetRenderTextureWidth out of bounds");

    const VulkanRenderTexture* texture = m_graphicsEngine->m_renderTextures[a_addr];

    return texture->GetWidth();
}
uint32_t VulkanGraphicsEngineBindings::GetRenderTextureHeight(uint32_t a_addr) const
{
    ICARIAN_ASSERT_MSG(a_addr < m_graphicsEngine->m_renderTextures.Size(), "GetRenderTextureHeight out of bounds");

    const VulkanRenderTexture* texture = m_graphicsEngine->m_renderTextures[a_addr];

    return texture->GetHeight();
}
void VulkanGraphicsEngineBindings::ResizeRenderTexture(uint32_t a_addr, uint32_t a_width, uint32_t a_height) const
{
    ICARIAN_ASSERT_MSG(a_addr < m_graphicsEngine->m_renderTextures.Size(), "ResizeRenderTexture out of bounds");
    ICARIAN_ASSERT_MSG(a_width > 0, "ResizeRenderTexture width 0")
    ICARIAN_ASSERT_MSG(a_height > 0, "ResizeRenderTexture height 0")

    VulkanRenderTexture* texture = m_graphicsEngine->m_renderTextures[a_addr];

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

    const VulkanDepthRenderTexture* tex = m_graphicsEngine->m_depthRenderTextures[a_addr];
    IDEFER(delete tex);
    m_graphicsEngine->m_depthRenderTextures.Erase(a_addr);
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
    ICARIAN_ASSERT_MSG(a_addr < m_graphicsEngine->m_directionalLights.Size(), "DestroyDirectionalLightBuffer out of bounds");

    const DirectionalLightBuffer buffer = m_graphicsEngine->m_directionalLights[a_addr];
    const VulkanLightBuffer* lightBuffer = (VulkanLightBuffer*)buffer.Data;
    IDEFER(delete lightBuffer);
    m_graphicsEngine->m_directionalLights.Erase(a_addr);
}
void VulkanGraphicsEngineBindings::AddDirectionalLightShadowMap(uint32_t a_addr, uint32_t a_shadowMapAddr) const
{
    ICARIAN_ASSERT_MSG(a_addr < m_graphicsEngine->m_directionalLights.Size(), "AddDirectionalLightShadowMap DirectionalLight out of bounds");
    ICARIAN_ASSERT_MSG(a_shadowMapAddr < m_graphicsEngine->m_depthRenderTextures.Size(), "AddDirectionalLightShadowMap DepthShadowMap out of bounds");

    const DirectionalLightBuffer& buffer = m_graphicsEngine->m_directionalLights[a_addr];
    VulkanLightBuffer* lightBuffer = (VulkanLightBuffer*)buffer.Data;

    VulkanLightRenderTexture* renderTexture = new VulkanLightRenderTexture[lightBuffer->LightRenderTextureCount + 1];
    for (uint32_t i = 0; i < lightBuffer->LightRenderTextureCount; ++i)
    {
        renderTexture[i] = lightBuffer->LightRenderTextures[i];
    }
    renderTexture[lightBuffer->LightRenderTextureCount].TextureAddr = a_shadowMapAddr;
    renderTexture[lightBuffer->LightRenderTextureCount].Type = VulkanLightRenderTextureType_DepthRenderTexture;

    // Not the best way to do this, but it works for now
    const std::unique_lock g = std::unique_lock(m_graphicsEngine->m_directionalLights.Lock());
    const VulkanLightRenderTexture* renderTextures = lightBuffer->LightRenderTextures;
    IDEFER(
    if (renderTextures != nullptr)
    {
        delete[] renderTextures;
    });
    lightBuffer->LightRenderTextures = renderTexture;
    ++lightBuffer->LightRenderTextureCount;
}
void VulkanGraphicsEngineBindings::RemoveDirectionalLightShadowMap(uint32_t a_addr, uint32_t a_shadowMapAddr) const
{
    ICARIAN_ASSERT_MSG(a_addr < m_graphicsEngine->m_directionalLights.Size(), "RemoveDirectionalLightShadowMap DirectionalLight out of bounds");
    ICARIAN_ASSERT_MSG(a_shadowMapAddr < m_graphicsEngine->m_depthRenderTextures.Size(), "RemoveDirectionalLightShadowMap DepthShadowMap out of bounds");

    const DirectionalLightBuffer& buffer = m_graphicsEngine->m_directionalLights[a_addr];
    VulkanLightBuffer* lightBuffer = (VulkanLightBuffer*)buffer.Data;

    // Not the best way to do this, but it works for now
    uint32_t index = 0;
    const std::unique_lock g = std::unique_lock(m_graphicsEngine->m_directionalLights.Lock());
    for (uint32_t i = 0; i < lightBuffer->LightRenderTextureCount; ++i)
    {
        if (lightBuffer->LightRenderTextures[i].TextureAddr != a_shadowMapAddr)
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
    buffer.Data = nullptr;

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
    ICARIAN_ASSERT_MSG(a_addr < m_graphicsEngine->m_pointLights.Size(), "DestroyPointLightBuffer out of bounds");

    m_graphicsEngine->m_pointLights.Erase(a_addr);
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
    buffer.Data = nullptr;

    return m_graphicsEngine->m_spotLights.PushVal(buffer);
}
void VulkanGraphicsEngineBindings::SetSpotLightBuffer(uint32_t a_addr, const SpotLightBuffer& a_buffer) const
{
    ICARIAN_ASSERT_MSG(a_addr < m_graphicsEngine->m_spotLights.Size(), "SetSpotLightBuffer out of bounds");

    m_graphicsEngine->m_spotLights.LockSet(a_addr, a_buffer);
}
SpotLightBuffer VulkanGraphicsEngineBindings::GetSpotLightBuffer(uint32_t a_addr) const
{
    ICARIAN_ASSERT_MSG(a_addr < m_graphicsEngine->m_spotLights.Size(), "GetSpotLightBuffer out of bounds");

    return m_graphicsEngine->m_spotLights[a_addr];
}
void VulkanGraphicsEngineBindings::DestroySpotLightBuffer(uint32_t a_addr) const
{
    ICARIAN_ASSERT_MSG(a_addr < m_graphicsEngine->m_spotLights.Size(), "DestroySpotLightBuffer out of bounds");

    m_graphicsEngine->m_spotLights.Erase(a_addr);
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

void VulkanGraphicsEngineBindings::SetLightLVP(const glm::mat4 &a_lvp) const
{
    ICARIAN_ASSERT_MSG(m_graphicsEngine->m_lightData.Exists(), "SetLightLVP LightData does not exist");

    m_graphicsEngine->m_lightData->SetLVP(a_lvp);
}
#endif