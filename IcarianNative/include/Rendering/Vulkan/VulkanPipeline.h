#pragma once

#ifdef ICARIANNATIVE_ENABLE_GRAPHICS_VULKAN
#include "Rendering/Vulkan/VulkanConstants.h"

#include "Flare/RenderProgram.h"

class VulkanGraphicsEngine;
class VulkanRenderEngineBackend;
class VulkanRenderPass;
class VulkanShaderData;

class VulkanPipeline
{
private:
    VulkanRenderEngineBackend* m_engine;
    VulkanGraphicsEngine*      m_gEngine;

    uint32_t                   m_programAddr;
    
    vk::Pipeline               m_pipeline;

protected:

public:
    VulkanPipeline(VulkanRenderEngineBackend* a_engine, VulkanGraphicsEngine* a_gEngine, const vk::RenderPass& a_renderPass, bool a_depth, uint32_t a_textureCount, uint32_t a_programAddr);
    ~VulkanPipeline();

    inline vk::Pipeline GetPipeline() const
    {
        return m_pipeline;
    }

    VulkanShaderData* GetShaderData() const;

    void Bind(uint32_t a_index, vk::CommandBuffer a_commandBuffer) const;
};
#endif