#pragma once

#include <unordered_map>
#include <vector>

#include "Rendering/Vulkan/VulkanConstants.h"

class RuntimeFunction;
class RuntimeManager;
class VulkanGraphicsEngineBindings;
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
#include "DataTypes/TStatic.h"
#include "Flare/RenderProgram.h"
#include "Flare/TextureSampler.h"
#include "Rendering/CameraBuffer.h"
#include "Rendering/Light.h"
#include "Rendering/MaterialRenderStack.h"
#include "Rendering/MeshRenderBuffer.h"

class VulkanGraphicsEngine
{
private:
    friend class VulkanGraphicsEngineBindings;

    static constexpr uint32_t DrawingPassCount = 3;

    RuntimeManager*                               m_runtimeManager;
    VulkanGraphicsEngineBindings*                 m_runtimeBindings;
    VulkanSwapchain*                              m_swapchain;

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

    TArray<FlareBase::RenderProgram>              m_shaderPrograms;
     
    TArray<VulkanVertexShader*>                   m_vertexShaders;
    TArray<VulkanPixelShader*>                    m_pixelShaders;
     
    TArray<FlareBase::TextureSampler>             m_textureSampler;

    TArray<VulkanModel*>                          m_models;
    TArray<VulkanTexture*>                        m_textures;
    TArray<VulkanRenderTexture*>                  m_renderTextures;

    TArray<MeshRenderBuffer>                      m_renderBuffers;
    TArray<MaterialRenderStack>                   m_renderStacks;

    TArray<DirectionalLightBuffer>                m_directionalLights;
    TArray<PointLightBuffer>                      m_pointLights;
    TArray<SpotLightBuffer>                       m_spotLights;

    std::vector<VulkanUniformBuffer*>             m_directionalLightUniforms;
    std::vector<VulkanUniformBuffer*>             m_pointLightUniforms;
    std::vector<VulkanUniformBuffer*>             m_spotLightUniforms;

    TArray<CameraBuffer>                          m_cameraBuffers;
    std::vector<VulkanUniformBuffer*>             m_cameraUniforms;

    std::vector<vk::CommandPool>                  m_commandPool[VulkanFlightPoolSize];
    std::vector<vk::CommandBuffer>                m_commandBuffers[VulkanFlightPoolSize];
    
    vk::CommandBuffer StartCommandBuffer(uint32_t a_bufferIndex, uint32_t a_index) const;

    vk::CommandBuffer DrawPass(uint32_t a_camIndex, uint32_t a_bufferIndex, uint32_t a_index);
    vk::CommandBuffer LightPass(uint32_t a_camIndex, uint32_t a_bufferIndex, uint32_t a_index);
    vk::CommandBuffer PostPass(uint32_t a_camIndex, uint32_t a_bufferIndex, uint32_t a_index);

protected:

public:
    VulkanGraphicsEngine(RuntimeManager* a_runtime, VulkanRenderEngineBackend* a_vulkanEngine);
    ~VulkanGraphicsEngine();

    inline void SetSwapchain(VulkanSwapchain* a_swapchaing)
    {
        m_swapchain = a_swapchaing;
    }

    std::vector<vk::CommandBuffer> Update(uint32_t a_index);

    VulkanVertexShader* GetVertexShader(uint32_t a_addr);
    VulkanPixelShader* GetPixelShader(uint32_t a_addr);

    FlareBase::RenderProgram GetRenderProgram(uint32_t a_addr);
    VulkanPipeline* GetPipeline(uint32_t a_renderTexture, uint32_t a_pipeline);
    
    CameraBuffer GetCameraBuffer(uint32_t a_addr);
    inline VulkanUniformBuffer* GetCameraUniformBuffer(uint32_t a_addr) const
    {
        return m_cameraUniforms[a_addr];
    }

    VulkanModel* GetModel(uint32_t a_addr);

    VulkanTexture* GetTexture(uint32_t a_addr);
    VulkanRenderTexture* GetRenderTexture(uint32_t a_addr);
};