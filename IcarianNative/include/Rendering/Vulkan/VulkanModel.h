#pragma once

#ifdef ICARIANNATIVE_ENABLE_GRAPHICS_VULKAN
#include "Rendering/Vulkan/IcarianVulkanHeader.h"

class VulkanRenderEngineBackend;

class VulkanModel
{
private:
    VulkanRenderEngineBackend* m_engine;

    float                      m_radius;

    VmaAllocation              m_vbAlloc;
    VmaAllocation              m_ibAlloc;

    vk::Buffer                 m_vertexBuffer;
    vk::Buffer                 m_indexBuffer;

    uint32_t                   m_indexCount;

protected:

public:
    VulkanModel(VulkanRenderEngineBackend* a_engine, uint32_t a_vertexCount, const void* a_vertices, uint16_t a_vertexSize, uint32_t a_indexCount, const uint32_t* a_indices, float a_radius);
    ~VulkanModel();

    inline uint32_t GetIndexCount() const
    {
        return m_indexCount;
    }
    inline float GetRadius() const
    {
        return m_radius;
    }

    void Bind(const vk::CommandBuffer& a_cmdBuffer) const;
};

#endif