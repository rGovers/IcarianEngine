#include "Rendering/Vulkan/VulkanShaderStorageObject.h"

#include "Flare/IcarianDefer.h"
#include "Rendering/Vulkan/VulkanRenderEngineBackend.h"
#include "Trace.h"

VulkanShaderStorageObject::VulkanShaderStorageObject(VulkanRenderEngineBackend* a_engine, uint32_t a_bufferSize)
{
    TRACE("Creating Vulkan SSBO");
    m_engine = a_engine;

    m_bufferSize = a_bufferSize;

    for (uint32_t i = 0; i < VulkanFlightPoolSize; i++)
    {
        VkBufferCreateInfo bufferInfo = { };
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = (VkDeviceSize)a_bufferSize;
        bufferInfo.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        VmaAllocationCreateInfo allocInfo = { 0 };
        allocInfo.usage = VMA_MEMORY_USAGE_AUTO;
        allocInfo.requiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
        allocInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;

        VmaAllocationInfo vmaAllocInfo = {};
        
        VkBuffer tBuffer;
        vmaCreateBuffer(m_engine->GetAllocator(), (VkBufferCreateInfo*)&bufferInfo, &allocInfo, &tBuffer, &m_allocations[i], &vmaAllocInfo);
        m_buffers[i] = tBuffer;
    }
}
VulkanShaderStorageObject::~VulkanShaderStorageObject()
{
    TRACE("Destroying Vulkan SSBO");
    const VmaAllocator allocator = m_engine->GetAllocator();

    for (uint32_t i = 0; i < VulkanFlightPoolSize; i++)
    {
        vmaDestroyBuffer(allocator, (VkBuffer)m_buffers[i], m_allocations[i]);
    }
}

void VulkanShaderStorageObject::SetData(uint32_t a_index, const void* a_data)
{
    const VmaAllocator allocator = m_engine->GetAllocator();

    void* dat;
    vmaMapMemory(allocator, m_allocations[a_index], &dat);
    IDEFER(vmaUnmapMemory(allocator, m_allocations[a_index]));

    memcpy(dat, a_data, m_bufferSize);
}