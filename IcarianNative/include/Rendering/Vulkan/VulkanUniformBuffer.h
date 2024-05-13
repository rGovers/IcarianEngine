#pragma once

#ifdef ICARIANNATIVE_ENABLE_GRAPHICS_VULKAN
#include "Rendering/Vulkan/IcarianVulkanHeader.h"

class VulkanRenderEngineBackend;

class VulkanUniformBuffer
{
private:
    VulkanRenderEngineBackend* m_engine;

    uint32_t                   m_uniformSize;
    vk::Buffer                 m_buffers[VulkanFlightPoolSize];
    VmaAllocation              m_allocations[VulkanFlightPoolSize];
protected:

public:
    VulkanUniformBuffer(VulkanRenderEngineBackend* a_engine, uint32_t a_uniformSize);
    ~VulkanUniformBuffer();

    void SetData(uint32_t a_index, const void* a_data);

    inline vk::Buffer GetBuffer(uint32_t a_index) const
    {
        return m_buffers[a_index];
    }
    inline uint32_t GetSize() const
    {
        return m_uniformSize;
    }
};
#endif