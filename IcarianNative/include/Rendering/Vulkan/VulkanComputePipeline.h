#pragma once

#ifdef ICARIANNATIVE_ENABLE_GRAPHICS_VULKAN
#include "Rendering/Vulkan/VulkanConstants.h"

class VulkanComputeEngine;
class VulkanComputeLayout;
class VulkanRenderEngineBackend;

class VulkanComputePipeline
{
private:
    VulkanRenderEngineBackend* m_engine;
    VulkanComputeEngine*       m_cEngine;

    uint32_t                   m_shaderAddr;

    vk::Pipeline               m_pipeline;

    VulkanComputePipeline(VulkanRenderEngineBackend* a_engine, VulkanComputeEngine* a_cEngine, uint32_t a_shaderAddr);

protected:

public:
    ~VulkanComputePipeline();

    inline vk::Pipeline GetPipeline() const
    {
        return m_pipeline;
    }

    static VulkanComputePipeline* CreatePipeline(VulkanRenderEngineBackend* a_engine, VulkanComputeEngine* a_cEngine, VulkanComputeLayout* a_layout, uint32_t a_shaderAddr);
};

#endif