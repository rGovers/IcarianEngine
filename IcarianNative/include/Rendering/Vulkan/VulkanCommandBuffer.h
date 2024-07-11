#pragma once

#ifdef ICARIANNATIVE_ENABLE_GRAPHICS_VULKAN

#include "Rendering/Vulkan/IcarianVulkanHeader.h"

enum e_VulkanCommandBufferType
{
    VulkanCommandBufferType_Compute,
    VulkanCommandBufferType_VideoDecode,
    VulkanCommandBufferType_Graphics,
};

// TODO: Switch to this down the line
class VulkanCommandBuffer
{
private:
    vk::CommandBuffer         m_commandBuffer;
    e_VulkanCommandBufferType m_type;

protected:

public:
    VulkanCommandBuffer(const vk::CommandBuffer& a_buffer, e_VulkanCommandBufferType a_type)
    {
        m_commandBuffer = a_buffer;
        m_type = a_type;
    }
    ~VulkanCommandBuffer() { }

    inline vk::CommandBuffer GetCommandBuffer() const
    {
        return m_commandBuffer;
    }
    inline void SetCommandBuffer(const vk::CommandBuffer& a_buffer)
    {
        m_commandBuffer = a_buffer;
    }

    inline e_VulkanCommandBufferType GetBufferType() const
    {
        return m_type;
    }
};

#endif