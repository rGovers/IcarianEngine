#include "Rendering/Vulkan/VulkanShaderStorageObject.h"

#include "Flare/IcarianAssert.h"
#include "Flare/IcarianDefer.h"
#include "Rendering/Vulkan/VulkanRenderEngineBackend.h"
#include "Trace.h"

class VulkanSSBOBufferDeletionObject : public VulkanDeletionObject
{
private:
    VulkanRenderEngineBackend* m_engine;

    vk::Buffer                 m_buffer;
    VmaAllocation              m_allocation;

protected:

public:
    VulkanSSBOBufferDeletionObject(VulkanRenderEngineBackend* a_engine, vk::Buffer a_buffer, VmaAllocation a_allocation)
    {
        m_engine = a_engine;

        m_buffer = a_buffer;
        m_allocation = a_allocation;
    }
    virtual ~VulkanSSBOBufferDeletionObject()
    {

    }

    virtual void Destroy()
    {
        const VmaAllocator allocator = m_engine->GetAllocator();

        vmaDestroyBuffer(allocator, m_buffer, m_allocation);
    }
};

VulkanShaderStorageObject::VulkanShaderStorageObject(VulkanRenderEngineBackend* a_engine, uint32_t a_bufferSize, const void* a_data)
{
    m_engine = a_engine;

    m_bufferSize = a_bufferSize;

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
    vmaCreateBuffer(m_engine->GetAllocator(), (VkBufferCreateInfo*)&bufferInfo, &allocInfo, &tBuffer, &m_allocation, &vmaAllocInfo);
    m_buffer = tBuffer;

    memcpy(vmaAllocInfo.pMappedData, a_data, a_bufferSize);
}
VulkanShaderStorageObject::~VulkanShaderStorageObject()
{
    m_engine->PushDeletionObject(new VulkanSSBOBufferDeletionObject(m_engine, m_buffer, m_allocation));
}
