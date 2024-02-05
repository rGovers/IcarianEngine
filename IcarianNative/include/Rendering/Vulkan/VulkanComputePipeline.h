#pragma once

#ifdef ICARIANNATIVE_ENABLE_GRAPHICS_VULKAN
#include "Rendering/Vulkan/VulkanConstants.h"

class VulkanComputeEngine;
class VulkanComputeLayout;
class VulkanRenderEngineBackend;

class VulkanComputePipeline
{
private:
    VulkanComputeEngine*       m_engine;

    vk::Pipeline               m_pipeline;

    VulkanComputePipeline(VulkanComputeEngine* a_engine, vk::Pipeline a_pipeline);

protected:

public:
    ~VulkanComputePipeline();

    inline vk::Pipeline GetPipeline() const
    {
        return m_pipeline;
    }

    static VulkanComputePipeline* CreatePipeline(VulkanComputeEngine* a_cEngine, uint32_t a_layoutAddr, uint32_t a_shaderAddr);
};

#endif