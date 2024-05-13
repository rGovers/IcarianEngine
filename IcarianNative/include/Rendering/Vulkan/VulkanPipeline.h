#pragma once

#ifdef ICARIANNATIVE_ENABLE_GRAPHICS_VULKAN
#include "Rendering/Vulkan/IcarianVulkanHeader.h"

class VulkanGraphicsEngine;
class VulkanRenderEngineBackend;
class VulkanRenderPass;
class VulkanShaderData;

enum e_VulkanPipelineType
{
    VulkanPipelineType_Graphics,
    VulkanPipelineType_Shadow
};

class VulkanPipeline
{
private:
    VulkanRenderEngineBackend* m_engine;
    VulkanGraphicsEngine*      m_gEngine;

    uint32_t                   m_programAddr;
    
    vk::Pipeline               m_pipeline;

    e_VulkanPipelineType       m_type;

    VulkanPipeline(VulkanRenderEngineBackend* a_engine, VulkanGraphicsEngine* a_gEngine, uint32_t a_programAddr, e_VulkanPipelineType a_type);
    
protected:

public:
    ~VulkanPipeline();

    inline e_VulkanPipelineType GetType() const
    {
        return m_type;
    }

    inline vk::Pipeline GetPipeline() const
    {
        return m_pipeline;
    }

    VulkanShaderData* GetShaderData() const;

    void Bind(uint32_t a_index, vk::CommandBuffer a_commandBuffer) const;

    static VulkanPipeline* CreatePipeline(VulkanRenderEngineBackend* a_engine, VulkanGraphicsEngine* a_gEngine, const vk::RenderPass& a_renderPass, bool a_depth, uint32_t a_textureCount, uint32_t a_programAddr);
    static VulkanPipeline* CreateShadowPipeline(VulkanRenderEngineBackend* a_engine, VulkanGraphicsEngine* a_gEngine, const vk::RenderPass& a_renderPass, uint32_t a_programAddr);
};
#endif