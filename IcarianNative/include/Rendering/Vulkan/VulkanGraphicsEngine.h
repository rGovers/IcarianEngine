#pragma once

#include <unordered_map>
#include <vector>

#include "Rendering/Vulkan/VulkanConstants.h"
#include "Rendering/Vulkan/VulkanDepthRenderTexture.h"

struct CanvasBuffer;

class Font;
class RuntimeFunction;
class RuntimeManager;
class VulkanGraphicsEngineBindings;
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

#include "DataTypes/TArray.h"
#include "DataTypes/TNCArray.h"
#include "DataTypes/TStatic.h"
#include "Flare/RenderProgram.h"
#include "Flare/TextureSampler.h"
#include "Rendering/CameraBuffer.h"
#include "Rendering/Light.h"
#include "Rendering/MaterialRenderStack.h"
#include "Rendering/MeshRenderBuffer.h"
#include "Rendering/UI/CanvasRendererBuffer.h"

class VulkanGraphicsEngine
{
private:
    friend class VulkanGraphicsEngineBindings;

    static constexpr uint32_t DrawingPassCount = 4;

    RuntimeManager*                               m_runtimeManager;
    VulkanGraphicsEngineBindings*                 m_runtimeBindings;
    VulkanSwapchain*                              m_swapchain;

    RuntimeFunction*                              m_shadowSetupFunc;
    RuntimeFunction*                              m_preShadowFunc;
    RuntimeFunction*                              m_postShadowFunc;
    RuntimeFunction*                              m_preRenderFunc;
    RuntimeFunction*                              m_postRenderFunc;
    RuntimeFunction*                              m_lightSetupFunc;
    RuntimeFunction*                              m_preLightFunc;
    RuntimeFunction*                              m_postLightFunc;
    RuntimeFunction*                              m_postProcessFunc;

    VulkanRenderEngineBackend*                    m_vulkanEngine;

    std::shared_mutex                             m_pipeLock;
    std::unordered_map<uint64_t, VulkanPipeline*> m_pipelines;

    TStatic<VulkanRenderCommand>                  m_renderCommands;
    TStatic<VulkanLightData>                      m_lightData;

    TArray<FlareBase::RenderProgram>              m_shaderPrograms;
     
    TArray<VulkanVertexShader*>                   m_vertexShaders;
    TArray<VulkanPixelShader*>                    m_pixelShaders;
     
    TArray<FlareBase::TextureSampler>             m_textureSampler;

    TNCArray<VulkanModel*>                        m_models;
    TArray<VulkanTexture*>                        m_textures;

    // This comment is to make language server stop showing an error for the next line
    // it compiles fine but the language server is fucking dumb
    TNCArray<VulkanRenderTexture*>                m_renderTextures;
    TNCArray<VulkanDepthRenderTexture*>           m_depthRenderTextures;

    TArray<MeshRenderBuffer>                      m_renderBuffers;
    TArray<MaterialRenderStack*>                  m_renderStacks;

    TNCArray<DirectionalLightBuffer>              m_directionalLights;
    TNCArray<PointLightBuffer>                    m_pointLights;
    TNCArray<SpotLightBuffer>                     m_spotLights;

    TArray<Font*>                                 m_fonts;

    std::vector<VulkanUniformBuffer*>             m_directionalLightUniforms;
    std::vector<VulkanUniformBuffer*>             m_pointLightUniforms;
    std::vector<VulkanUniformBuffer*>             m_spotLightUniforms;

    TArray<CameraBuffer>                          m_cameraBuffers;
    std::vector<VulkanUniformBuffer*>             m_cameraUniforms;

    std::vector<vk::CommandPool>                  m_commandPool[VulkanFlightPoolSize];
    std::vector<vk::CommandBuffer>                m_commandBuffers[VulkanFlightPoolSize];
    
    TArray<CanvasRendererBuffer>                  m_canvasRenderers;

    uint32_t                                      m_textUIPipelineAddr;
    uint32_t                                      m_imageUIPipelineAddr;

    vk::CommandBuffer StartCommandBuffer(uint32_t a_bufferIndex, uint32_t a_index) const;

    vk::CommandBuffer ShadowPass(uint32_t a_camIndex, uint32_t a_bufferIndex, uint32_t a_index);
    vk::CommandBuffer DrawPass(uint32_t a_camIndex, uint32_t a_bufferIndex, uint32_t a_index);
    vk::CommandBuffer LightPass(uint32_t a_camIndex, uint32_t a_bufferIndex, uint32_t a_index);
    vk::CommandBuffer PostPass(uint32_t a_camIndex, uint32_t a_bufferIndex, uint32_t a_index);

    void DrawUIElement(vk::CommandBuffer a_commandBuffer, uint32_t a_addr, const CanvasBuffer& a_canvas, const glm::vec2& a_screenSize, uint32_t a_index);
    
protected:

public:
    VulkanGraphicsEngine(RuntimeManager* a_runtime, VulkanRenderEngineBackend* a_vulkanEngine);
    ~VulkanGraphicsEngine();

    inline void SetSwapchain(VulkanSwapchain* a_swapchaing)
    {
        m_swapchain = a_swapchaing;
    }

    std::vector<vk::CommandBuffer> Update(uint32_t a_index);

    uint32_t GenerateGLSLVertexShader(const std::string_view& a_source);
    uint32_t GenerateFVertexShader(const std::string_view& a_source);
    void DestroyVertexShader(uint32_t a_addr);
    VulkanVertexShader* GetVertexShader(uint32_t a_addr);

    uint32_t GenerateGLSLPixelShader(const std::string_view& a_source);
    uint32_t GenerateFPixelShader(const std::string_view& a_source);
    void DestroyPixelShader(uint32_t a_addr);
    VulkanPixelShader* GetPixelShader(uint32_t a_addr);

    uint32_t GenerateRenderProgram(const FlareBase::RenderProgram& a_program);
    void DestroyRenderProgram(uint32_t a_addr);
    FlareBase::RenderProgram GetRenderProgram(uint32_t a_addr);
    VulkanPipeline* GetPipeline(uint32_t a_renderTexture, uint32_t a_pipeline);
    
    CameraBuffer GetCameraBuffer(uint32_t a_addr);
    inline VulkanUniformBuffer* GetCameraUniformBuffer(uint32_t a_addr) const
    {
        return m_cameraUniforms[a_addr];
    }

    VulkanModel* GetModel(uint32_t a_addr);

    uint32_t GenerateAlphaTexture(uint32_t a_width, uint32_t a_height, const void* a_data);
    void DestroyTexture(uint32_t a_addr);
    VulkanTexture* GetTexture(uint32_t a_addr);

    VulkanRenderTexture* GetRenderTexture(uint32_t a_addr);
    VulkanDepthRenderTexture* GetDepthRenderTexture(uint32_t a_addr);

    uint32_t GenerateTextureSampler(uint32_t a_textureAddr, FlareBase::e_TextureMode a_textureMode, FlareBase::e_TextureFilter a_filterMode, FlareBase::e_TextureAddress a_addressMode, uint32_t a_slot = 0);
    void DestroyTextureSampler(uint32_t a_addr);

    Font* GetFont(uint32_t a_addr);
};