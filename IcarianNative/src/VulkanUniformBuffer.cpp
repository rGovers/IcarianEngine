#ifdef ICARIANNATIVE_ENABLE_GRAPHICS_VULKAN

#include "Rendering/Vulkan/VulkanUniformBuffer.h"

#include "Core/IcarianAssert.h"
#include "Rendering/Vulkan/VulkanRenderEngineBackend.h"
#include "Trace.h"

class VulkanUBOBufferDeletionObject : public VulkanDeletionObject
{
private:
    VulkanRenderEngineBackend* m_engine;

    vk::Buffer                 m_buffer[VulkanFlightPoolSize];
    VmaAllocation              m_allocation[VulkanFlightPoolSize];

protected:

public:
    VulkanUBOBufferDeletionObject(VulkanRenderEngineBackend* a_engine, const vk::Buffer* a_buffer, const VmaAllocation* a_allocation)
    {
        m_engine = a_engine;

        for (uint32_t i = 0; i < VulkanFlightPoolSize; ++i)
        {
            m_buffer[i] = a_buffer[i];
            m_allocation[i] = a_allocation[i];
        }
    }
    virtual ~VulkanUBOBufferDeletionObject()
    {

    }

    virtual void Destroy()
    {
        TRACE("Destroying UBO");
        const VmaAllocator allocator = m_engine->GetAllocator();

        for (uint32_t i = 0; i < VulkanFlightPoolSize; ++i)
        {
            vmaDestroyBuffer(allocator, m_buffer[i], m_allocation[i]);
        }
    }
};

VulkanUniformBuffer::VulkanUniformBuffer(VulkanRenderEngineBackend* a_engine, uint32_t a_uniformSize)
{
    TRACE("Creating Vulkan UBO");
    m_engine = a_engine;

    m_uniformSize = a_uniformSize;

    const VmaAllocator allocator = m_engine->GetAllocator();

    VkBufferCreateInfo bufferInfo = { };
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = (VkDeviceSize)a_uniformSize;
    bufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VmaAllocationCreateInfo bufferAllocInfo = { 0 };
    bufferAllocInfo.usage = VMA_MEMORY_USAGE_AUTO;
    bufferAllocInfo.requiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
    bufferAllocInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;
    
    for (uint32_t i = 0; i < VulkanFlightPoolSize; ++i)
    {
        VkBuffer tBuffer;
        VKRESERRMSG(vmaCreateBuffer(allocator, &bufferInfo, &bufferAllocInfo, &tBuffer, &m_allocations[i], nullptr), "Failed to create Uniform Buffer");
#ifdef DEBUG
        vmaSetAllocationName(allocator, m_allocations[i], "Uniform Buffer Object");
#endif
        m_buffers[i] = tBuffer;
    }
}
VulkanUniformBuffer::~VulkanUniformBuffer()
{
    TRACE("Queueing UBO for deletion");
    m_engine->PushDeletionObject(new VulkanUBOBufferDeletionObject(m_engine, m_buffers, m_allocations));
}

void VulkanUniformBuffer::SetData(uint32_t a_index, const void* a_data)
{
    const VmaAllocator allocator = m_engine->GetAllocator();

    void* dat;
    vmaMapMemory(allocator, m_allocations[a_index], &dat);

    memcpy(dat, a_data, m_uniformSize);

    vmaUnmapMemory(allocator, m_allocations[a_index]);
}
#endif