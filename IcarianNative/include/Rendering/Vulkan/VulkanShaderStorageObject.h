#pragma once

#include "Rendering/Vulkan/VulkanConstants.h"

class VulkanRenderEngineBackend;

class VulkanShaderStorageObject
{
private:
    VulkanRenderEngineBackend* m_engine;

    uint32_t                   m_bufferSize;
    vk::Buffer                 m_buffer;
    VmaAllocation              m_allocation;

protected:

public:
    VulkanShaderStorageObject(VulkanRenderEngineBackend* a_engine, uint32_t a_bufferSize, const void* a_data);
    ~VulkanShaderStorageObject();

    inline vk::Buffer GetBuffer() const
    {
        return m_buffer;
    }
    inline uint32_t GetSize() const
    {
        return m_bufferSize;
    }
};