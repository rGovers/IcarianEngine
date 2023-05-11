#pragma once

#include "Rendering/Vulkan/VulkanConstants.h"

#include <mutex>

class VulkanRenderEngineBackend;

class VulkanModel
{
private:
    VulkanRenderEngineBackend* m_engine;

    std::mutex                 m_lock;

    VmaAllocation              m_vbAlloc;
    VmaAllocation              m_ibAlloc;

    vk::Buffer                 m_vertexBuffer;
    vk::Buffer                 m_indexBuffer;

    uint32_t                   m_indexCount;

protected:

public:
    VulkanModel(VulkanRenderEngineBackend* a_engine, uint32_t a_vertexCount, const char* a_vertices, uint16_t a_vertexSize, uint32_t a_indexCount, const uint32_t* a_indices);
    ~VulkanModel();

    inline std::mutex& GetLock()
    {
        return m_lock;
    }

    inline uint32_t GetIndexCount() const
    {
        return m_indexCount;
    }

    void Bind(const vk::CommandBuffer& a_cmdBuffer) const;
};