#pragma once

#include "Rendering/Vulkan/VulkanConstants.h"

class VulkanRenderEngineBackend;

class VulkanShaderStorageObject
{
private:
    VulkanRenderEngineBackend* m_engine;

    uint32_t                   m_bufferSize;
    vk::Buffer                 m_buffers[VulkanFlightPoolSize];
    VmaAllocation              m_allocations[VulkanFlightPoolSize];

protected:

public:
    VulkanShaderStorageObject(VulkanRenderEngineBackend* a_engine, uint32_t a_bufferSize);
    ~VulkanShaderStorageObject();

    void SetData(uint32_t a_index, const void* a_data);

    inline vk::Buffer GetBuffer(uint32_t a_index) const
    {
        return m_buffers[a_index];
    }
    inline uint32_t GetSize() const
    {
        return m_bufferSize;
    }
};