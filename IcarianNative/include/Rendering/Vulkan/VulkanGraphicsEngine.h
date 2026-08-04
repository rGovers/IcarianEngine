// Icarian Engine - C# Game Engine
//
// License at end of file.

#pragma once

#ifdef ICARIANNATIVE_ENABLE_GRAPHICS_VULKAN

#include "Rendering/Vulkan/IcarianVulkanHeader.h"

#include "DataTypes/Array.h"
#include "DataTypes/COWString.h"
#include "DataTypes/Dictionary.h"
#include "DataTypes/TArray.h"
#include "DataTypes/TNCArray.h"
#include "DataTypes/TStatic.h"
#include "Rendering/CameraBuffer.h"
#include "Rendering/MaterialRenderStack.h"
#include "Rendering/RenderBuffers.h"
#include "Rendering/TextureData.h"
#include "Rendering/UI/CanvasRendererBuffer.h"
#include "Rendering/Vulkan/VulkanCommandBuffer.h"
#include "Rendering/Vulkan/VulkanRenderEngineBackend.h"

#include "EngineAmbientLightInteropStructures.h"
#include "EngineDirectionalLightInteropStructures.h"
#include "EngineMaterialInteropStructures.h"
#include "EnginePointLightInteropStructures.h"
#include "EngineSpotLightInteropStructures.h"
#include "EngineTextureSamplerInteropStructures.h"

struct CanvasBuffer;

class RuntimeFunction;
class VulkanComputeShader;
class VulkanDecalShader;
class VulkanDepthCubeRenderTexture;
class VulkanDepthRenderTexture;
class VulkanGraphicsEngineBindings;
class VulkanGraphicsParticle2D;
class VulkanLightData;
class VulkanMeshShader;
class VulkanMesh;
class VulkanModel;
class VulkanPipeline;
class VulkanPixelShader;
class VulkanRenderCommand;
class VulkanRenderTexture;
class VulkanSwapchain;
class VulkanTaskShader;
class VulkanTexture;
class VulkanUniformBuffer;
class VulkanVertexShader;
class VulkanVideoTexture;

struct VulkanMeshEmulationData
{
    vk::Buffer DrawBuffer;
    vk::Buffer VertexBuffer;
    vk::Buffer IndexBuffer;
    // vk::Buffer TaskBuffer;

    VmaAllocation DrawAlloction;
    VmaAllocation VertexAllocation;
    VmaAllocation IndexAllocation;
    // VmaAllocation TaskAllocation;

    Dictionary<uint64_t, VulkanPipeline*> MeshPipelines;
};

enum e_VulkanShaderInfoType
{
    VulkanShaderInfoType_Null = 0,
    VulkanShaderInfoType_Flare,
    VulkanShaderInfoType_GLSL
};

struct VulkanShaderInfo
{
    COWU8String Data;
    e_VulkanShaderInfoType Type;
};

class VulkanGraphicsEngine
{
private:
    friend class VulkanGraphicsEngineBindings;

    static constexpr uint32_t DrawingPassCount = 7;

    VulkanGraphicsEngineBindings*           m_runtimeBindings;
    VulkanSwapchain*                        m_swapchain;

    RuntimeFunction*                        m_shadowSetupFunc;
    RuntimeFunction*                        m_preShadowFunc;
    RuntimeFunction*                        m_postShadowFunc;
    RuntimeFunction*                        m_preRenderFunc;
    RuntimeFunction*                        m_postRenderFunc;
    RuntimeFunction*                        m_lightSetupFunc;
    RuntimeFunction*                        m_preShadowLightFunc;
    RuntimeFunction*                        m_postShadowLightFunc;
    RuntimeFunction*                        m_preLightFunc;
    RuntimeFunction*                        m_postLightFunc;
    RuntimeFunction*                        m_preForwardFunc;
    RuntimeFunction*                        m_postForwardFunc;
    RuntimeFunction*                        m_postProcessFunc;

    VulkanRenderEngineBackend*              m_vulkanEngine;

    VulkanMeshEmulationData*                m_meshEmulationData;

    SharedSpinLock                          m_pipeLock;
    SharedSpinLock                          m_shadowPipeLock;
    SharedSpinLock                          m_cubeShadowPipeLock;
    SharedSpinLock                          m_importLock;
    SharedSpinLock                          m_meshPipelineLock;

    Dictionary<uint64_t, VulkanPipeline*>   m_pipelines;
    Dictionary<uint64_t, VulkanPipeline*>   m_shadowPipelines;
    Dictionary<uint64_t, VulkanPipeline*>   m_cubeShadowPipelines;
    Dictionary<COWU8String, COWU8String>    m_computeImports;
    Dictionary<COWU8String, COWU8String>    m_vertexImports;
    Dictionary<COWU8String, COWU8String>    m_meshImports;
    Dictionary<COWU8String, COWU8String>    m_pixelImports;

    TStatic<VulkanRenderCommand>            m_renderCommands;
    TStatic<VulkanLightData>                m_lightData;

    TNCArray<RenderProgram>                 m_shaderPrograms;

    TNCArray<VulkanShaderInfo>              m_vertexShaders;
    TNCArray<VulkanShaderInfo>              m_taskShaders;
    TNCArray<VulkanShaderInfo>              m_meshShaders;
    TNCArray<VulkanShaderInfo>              m_pixelShaders;
    TNCArray<VulkanShaderInfo>              m_computeShaders;

    TNCArray<TextureSamplerBuffer>          m_textureSampler;

    TNCArray<VulkanModel*>                  m_models;
    TNCArray<VulkanMesh*>                   m_meshes;
    TNCArray<VulkanTexture*>                m_textures;

    TNCArray<VulkanRenderTexture*>          m_renderTextures;
    TNCArray<VulkanDepthCubeRenderTexture*> m_depthCubeRenderTextures;
    TNCArray<VulkanDepthRenderTexture*>     m_depthRenderTextures;

    TNCArray<ModelRenderBuffer>             m_renderBuffers;
    TNCArray<SkinnedModelRenderBuffer>      m_skinnedRenderBuffers;
    TNCArray<MeshRenderBuffer>              m_meshRenderBuffers;
    TArray<MaterialRenderStack*>            m_renderStacks;

    TNCArray<VulkanGraphicsParticle2D*>     m_particleEmitters;

    TNCArray<AmbientLightBuffer>            m_ambientLights;
    TNCArray<DirectionalLightBuffer>        m_directionalLights;
    TNCArray<PointLightBuffer>              m_pointLights;
    TNCArray<SpotLightBuffer>               m_spotLights;

    TArray<CameraBuffer>                    m_cameraBuffers;
    Array<VulkanUniformBuffer*>             m_cameraUniforms;

    VulkanUniformBuffer*                    m_timeUniform;

    Array<vk::CommandPool>*                 m_commandPool[VulkanFlightPoolSize];
    Array<vk::CommandBuffer>*               m_commandBuffers[VulkanFlightPoolSize];

    TNCArray<CanvasRendererBuffer>          m_canvasRenderers;

    uint32_t                                m_textUIPipelineAddr;
    uint32_t                                m_imageUIPipelineAddr;

    vk::CommandBuffer StartCommandBuffer(uint32_t a_bufferIndex, uint32_t a_index) const;

    void Draw(bool a_forward, const CameraBuffer& a_camBuffer, const Frustum& a_frustum, VulkanRenderCommand* a_renderCommand, uint32_t a_frameIndex);
    void DrawShadow
    (
        const glm::mat4& a_lvp,
        float a_split,
        const glm::vec2& a_bias,
        uint32_t a_renderLayer,
        uint32_t a_renderTexture,
        bool a_cube,
        vk::CommandBuffer a_commandBuffer,
        uint32_t a_index
    );

    VulkanCommandBuffer DirectionalShadowPass(uint32_t a_camIndex, uint32_t a_bufferIndex, uint32_t a_frameIndex);
    VulkanCommandBuffer PointShadowPass(uint32_t a_camIndex, uint32_t a_bufferIndex, uint32_t a_frameIndex);
    VulkanCommandBuffer SpotShadowPass(uint32_t a_camIndex, uint32_t a_bufferIndex, uint32_t a_frameIndex);
    VulkanCommandBuffer DrawPass(uint32_t a_camIndex, uint32_t a_bufferIndex, uint32_t a_frameIndex);
    VulkanCommandBuffer LightPass(uint32_t a_camIndex, uint32_t a_bufferIndex, uint32_t a_frameIndex);
    VulkanCommandBuffer ForwardPass(uint32_t a_camIndex, uint32_t a_bufferIndex, uint32_t a_frameIndex);
    VulkanCommandBuffer PostPass(uint32_t a_camIndex, uint32_t a_bufferIndex, uint32_t a_frameIndex);

    void DrawUIElement(vk::CommandBuffer a_commandBuffer, uint32_t a_addr, const CanvasBuffer& a_canvas, const glm::vec2& a_screenSize, uint32_t a_index);

protected:

public:
    VulkanGraphicsEngine(VulkanRenderEngineBackend* a_vulkanEngine, VulkanSwapchain* a_swapchain);
    ~VulkanGraphicsEngine();

    // Code reeks but cannot be fucked
    // Later me problem
    void Cleanup();

    inline VulkanMeshEmulationData* GetMeshEmulationData() const
    {
        return m_meshEmulationData;
    }

    Array<VulkanCommandBuffer> Update(double a_delta, double a_time, uint32_t a_index);

    [[nodiscard]] uint32_t GenerateFVertexShader(const COWU8String& a_source);
    void DestroyVertexShader(uint32_t a_addr);
    VulkanShaderInfo GetVertexShaderInfo(uint32_t a_addr);
    Dictionary<COWU8String, COWU8String> GetVertexShaderImports();

    [[nodiscard]] uint32_t GenerateFTaskShader(const COWU8String& a_source);
    void DestroyTaskShader(uint32_t a_addr);
    VulkanShaderInfo GetTaskShaderInfo(uint32_t a_addr);
    [[nodiscard]] uint32_t GenerateFMeshShader(const COWU8String& a_source);
    void DestroyMeshShader(uint32_t a_addr);
    VulkanShaderInfo GetMeshShaderInfo(uint32_t a_addr);
    Dictionary<COWU8String, COWU8String> GetMeshShaderImports();

    [[nodiscard]] uint32_t GenerateFPixelShader(const COWU8String& a_source);
    void DestroyPixelShader(uint32_t a_addr);
    VulkanShaderInfo GetPixelShaderInfo(uint32_t a_addr);
    Dictionary<COWU8String, COWU8String> GetPixelShaderImports();

    [[nodiscard]] uint32_t GenerateFComputeShader(const COWU8String& a_source);
    void DestroyComputeShader(uint32_t a_addr);
    VulkanShaderInfo GetComputeShaderInfo(uint32_t a_addr);
    Dictionary<COWU8String, COWU8String> GetComputeShaderImports();

   [[nodiscard]]  uint32_t GenerateRenderProgram(const RenderProgram& a_program, Allocator* a_tempAllocator);
    void DestroyRenderProgram(uint32_t a_addr);
    RenderProgram GetRenderProgram(uint32_t a_addr);

    VulkanPipeline* GetShadowPipeline(uint32_t a_renderTexture, uint32_t a_pipeline);
    VulkanPipeline* GetCubeShadowPipeline(uint32_t a_renderTexture, uint32_t a_pipeline);
    VulkanPipeline* GetPipeline(uint32_t a_renderTexture, uint32_t a_pipeline);
    VulkanPipeline* GetComputeMeshPipeline(uint32_t a_pipeline);

    CameraBuffer GetCameraBuffer(uint32_t a_addr);
    inline VulkanUniformBuffer* GetCameraUniformBuffer(uint32_t a_addr) const
    {
        return m_cameraUniforms[a_addr];
    }

    inline VulkanUniformBuffer* GetTimeUniformBuffer() const
    {
        return m_timeUniform;
    }

    [[nodiscard]] uint32_t GenerateMesh
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
    );
    void DestroyMesh(uint32_t a_addr);
    VulkanMesh* GetMesh(uint32_t a_addr);

    [[nodiscard]] uint32_t GenerateModel
    (
        const void* a_vertices,
        uint32_t a_vertexCount,
        uint16_t a_vertexStride,
        const uint32_t* a_indices,
        uint32_t a_indexCount,
        float a_radius
    );
    void DestroyModel(uint32_t a_addr);
    VulkanModel* GetModel(uint32_t a_addr);

    [[nodiscard]] uint32_t GenerateTexture(uint32_t a_width, uint32_t a_height, e_TextureFormat a_format, const void* a_data);
    [[nodiscard]] uint32_t GenerateMipMappedTexture
    (
        uint32_t a_width,
        uint32_t a_height,
        uint32_t a_levels,
        const uint64_t* a_offsets,
        e_TextureFormat a_format,
        const void* a_data,
        uint64_t a_dataSize
    );
    void DestroyTexture(uint32_t a_addr);
    VulkanTexture* GetTexture(uint32_t a_addr);

    [[nodiscard]] uint32_t GenerateDepthRenderTexture(uint32_t a_width, uint32_t a_height);
    void DestroyDepthRenderTexture(uint32_t a_addr);

    VulkanRenderTexture* GetRenderTexture(uint32_t a_addr);
    VulkanDepthRenderTexture* GetDepthRenderTexture(uint32_t a_addr);
    VulkanDepthCubeRenderTexture* GetDepthCubeRenderTexture(uint32_t a_addr);

    AmbientLightBuffer GetAmbientLight(uint32_t a_addr);
    DirectionalLightBuffer GetDirectionalLight(uint32_t a_addr);
    PointLightBuffer GetPointLight(uint32_t a_addr);
    SpotLightBuffer GetSpotLight(uint32_t a_addr);

    [[nodiscard]] uint32_t GenerateTextureSampler
    (
        uint32_t a_textureAddr,
        e_TextureMode a_textureMode,
        e_TextureFilter a_filterMode,
        e_TextureAddress a_addressMode,
        uint32_t a_slot = 0
    );
    void DestroyTextureSampler(uint32_t a_addr);
    TextureSamplerBuffer GetTextureSampler(uint32_t a_addr);
};

#endif

// MIT License
//
// Copyright (c) 2026 River Govers
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
