#pragma once

#ifdef ICARIANNATIVE_ENABLE_GRAPHICS_VULKAN
#include "Rendering/Vulkan/IcarianVulkanHeader.h"

#include <unordered_map>
#include <vector>

struct CanvasBuffer;

class Font;
class RuntimeFunction;
class VulkanDepthCubeRenderTexture;
class VulkanDepthRenderTexture;
class VulkanGraphicsEngineBindings;
class VulkanGraphicsParticle2D;
class VulkanLightData;
class VulkanModel;
class VulkanPipeline;
class VulkanPixelShader;
class VulkanRenderCommand;
class VulkanRenderEngineBackend;
class VulkanRenderTexture;
class VulkanSwapchain;
class VulkanTexture;
class VulkanUniformBuffer;
class VulkanVertexShader;

#include "DataTypes/Array.h"
#include "DataTypes/TArray.h"
#include "DataTypes/TNCArray.h"
#include "DataTypes/TStatic.h"
#include "Rendering/CameraBuffer.h"
#include "Rendering/MaterialRenderStack.h"
#include "Rendering/MeshRenderBuffer.h"
#include "Rendering/SkinnedMeshRenderBuffer.h"
#include "Rendering/TextureData.h"
#include "Rendering/UI/CanvasRendererBuffer.h"

#include "EngineAmbientLightInteropStructures.h"
#include "EngineDirectionalLightInteropStructures.h"
#include "EngineMaterialInteropStructures.h"
#include "EnginePointLightInteropStructures.h"
#include "EngineSpotLightInteropStructures.h"
#include "EngineTextureSamplerInteropStructures.h"

class VulkanGraphicsEngine
{
private:
    friend class VulkanGraphicsEngineBindings;

    static constexpr uint32_t DrawingPassCount = 6;

    VulkanGraphicsEngineBindings*                 m_runtimeBindings;
    VulkanSwapchain*                              m_swapchain;

    RuntimeFunction*                              m_shadowSetupFunc;
    RuntimeFunction*                              m_preShadowFunc;
    RuntimeFunction*                              m_postShadowFunc;
    RuntimeFunction*                              m_preRenderFunc;
    RuntimeFunction*                              m_postRenderFunc;
    RuntimeFunction*                              m_lightSetupFunc;
    RuntimeFunction*                              m_preShadowLightFunc;
    RuntimeFunction*                              m_postShadowLightFunc;
    RuntimeFunction*                              m_preLightFunc;
    RuntimeFunction*                              m_postLightFunc;
    RuntimeFunction*                              m_postProcessFunc;

    VulkanRenderEngineBackend*                    m_vulkanEngine;

    SharedSpinLock                                m_pipeLock;
    std::unordered_map<uint64_t, VulkanPipeline*> m_pipelines;

    SharedSpinLock                                m_shadowPipeLock;
    std::unordered_map<uint64_t, VulkanPipeline*> m_shadowPipelines;

    SharedSpinLock                                m_cubeShadowPipeLock;
    std::unordered_map<uint64_t, VulkanPipeline*> m_cubeShadowPipelines;

    TStatic<VulkanRenderCommand>                  m_renderCommands;
    TStatic<VulkanLightData>                      m_lightData;

    TNCArray<RenderProgram>                       m_shaderPrograms;
     
    TNCArray<VulkanVertexShader*>                 m_vertexShaders;
    TNCArray<VulkanPixelShader*>                  m_pixelShaders;
     
    TNCArray<TextureSamplerBuffer>                m_textureSampler;

    TNCArray<VulkanModel*>                        m_models;
    TNCArray<VulkanTexture*>                      m_textures;

    TNCArray<VulkanRenderTexture*>                m_renderTextures;
    TNCArray<VulkanDepthCubeRenderTexture*>       m_depthCubeRenderTextures;
    TNCArray<VulkanDepthRenderTexture*>           m_depthRenderTextures;

    TNCArray<MeshRenderBuffer>                    m_renderBuffers;
    TNCArray<SkinnedMeshRenderBuffer>             m_skinnedRenderBuffers;
    TArray<MaterialRenderStack*>                  m_renderStacks;

    TNCArray<VulkanGraphicsParticle2D*>           m_particleEmitters;

    TNCArray<AmbientLightBuffer>                  m_ambientLights;
    TNCArray<DirectionalLightBuffer>              m_directionalLights;
    TNCArray<PointLightBuffer>                    m_pointLights;
    TNCArray<SpotLightBuffer>                     m_spotLights;

    TArray<Font*>                                 m_fonts;

    TArray<CameraBuffer>                          m_cameraBuffers;
    Array<VulkanUniformBuffer*>                   m_cameraUniforms;

    VulkanUniformBuffer*                          m_timeUniform;

    Array<vk::CommandPool>                        m_commandPool[VulkanFlightPoolSize];
    Array<vk::CommandBuffer>                      m_commandBuffers[VulkanFlightPoolSize];
    
    TArray<CanvasRendererBuffer>                  m_canvasRenderers;

    uint32_t                                      m_textUIPipelineAddr;
    uint32_t                                      m_imageUIPipelineAddr;

    vk::CommandBuffer StartCommandBuffer(uint32_t a_bufferIndex, uint32_t a_index) const;

    void DrawShadow(const glm::mat4& a_lvp, float a_split, uint32_t a_renderLayer, uint32_t a_renderTexture, bool a_cube, vk::CommandBuffer a_commandBuffer, uint32_t a_index);

    vk::CommandBuffer DirectionalShadowPass(uint32_t a_camIndex, uint32_t a_bufferIndex, uint32_t a_index);
    vk::CommandBuffer PointShadowPass(uint32_t a_camIndex, uint32_t a_bufferIndex, uint32_t a_index);
    vk::CommandBuffer SpotShadowPass(uint32_t a_camIndex, uint32_t a_bufferIndex, uint32_t a_index);
    vk::CommandBuffer DrawPass(uint32_t a_camIndex, uint32_t a_bufferIndex, uint32_t a_index);
    vk::CommandBuffer LightPass(uint32_t a_camIndex, uint32_t a_bufferIndex, uint32_t a_index);
    vk::CommandBuffer PostPass(uint32_t a_camIndex, uint32_t a_bufferIndex, uint32_t a_index);

    void DrawUIElement(vk::CommandBuffer a_commandBuffer, uint32_t a_addr, const CanvasBuffer& a_canvas, const glm::vec2& a_screenSize, uint32_t a_index);
    
protected:

public:
    VulkanGraphicsEngine(VulkanRenderEngineBackend* a_vulkanEngine);
    ~VulkanGraphicsEngine();

    // Code reeks but cannot be fucked
    // Later me problem
    void Cleanup();

    inline void SetSwapchain(VulkanSwapchain* a_swapchaing)
    {
        m_swapchain = a_swapchaing;
    }

    Array<vk::CommandBuffer> Update(double a_delta, double a_time, uint32_t a_index);

    uint32_t GenerateFVertexShader(const std::string_view& a_source);
    void DestroyVertexShader(uint32_t a_addr);
    VulkanVertexShader* GetVertexShader(uint32_t a_addr);

    uint32_t GenerateFPixelShader(const std::string_view& a_source);
    void DestroyPixelShader(uint32_t a_addr);
    VulkanPixelShader* GetPixelShader(uint32_t a_addr);

    uint32_t GenerateRenderProgram(const RenderProgram& a_program);
    void DestroyRenderProgram(uint32_t a_addr);
    RenderProgram GetRenderProgram(uint32_t a_addr);

    VulkanPipeline* GetShadowPipeline(uint32_t a_renderTexture, uint32_t a_pipeline);
    VulkanPipeline* GetCubeShadowPipeline(uint32_t a_renderTexture, uint32_t a_pipeline);
    VulkanPipeline* GetPipeline(uint32_t a_renderTexture, uint32_t a_pipeline);
    
    CameraBuffer GetCameraBuffer(uint32_t a_addr);
    inline VulkanUniformBuffer* GetCameraUniformBuffer(uint32_t a_addr) const
    {
        return m_cameraUniforms[a_addr];
    }

    inline VulkanUniformBuffer* GetTimeUniformBuffer() const
    {
        return m_timeUniform;
    }

    uint32_t GenerateModel(const void* a_vertices, uint32_t a_vertexCount, uint16_t a_vertexStride, const uint32_t* a_indices, uint32_t a_indexCount, float a_radius);
    void DestroyModel(uint32_t a_addr);
    VulkanModel* GetModel(uint32_t a_addr);

    uint32_t GenerateTexture(uint32_t a_width, uint32_t a_height, e_TextureFormat a_format, const void* a_data);
    uint32_t GenerateMipMappedTexture(uint32_t a_width, uint32_t a_height, uint32_t a_levels, const uint64_t* a_offsets, e_TextureFormat a_format, const void* a_data, uint64_t a_dataSize);
    void DestroyTexture(uint32_t a_addr);
    VulkanTexture* GetTexture(uint32_t a_addr);

    VulkanRenderTexture* GetRenderTexture(uint32_t a_addr);
    VulkanDepthRenderTexture* GetDepthRenderTexture(uint32_t a_addr);
    VulkanDepthCubeRenderTexture* GetDepthCubeRenderTexture(uint32_t a_addr);

    uint32_t GenerateTextureSampler(uint32_t a_textureAddr, e_TextureMode a_textureMode, e_TextureFilter a_filterMode, e_TextureAddress a_addressMode, uint32_t a_slot = 0);
    void DestroyTextureSampler(uint32_t a_addr);
    TextureSamplerBuffer GetTextureSampler(uint32_t a_addr);

    Font* GetFont(uint32_t a_addr);
};
#endif