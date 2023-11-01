#ifdef ICARIANNATIVE_ENABLE_GRAPHICS_VULKAN

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

VulkanShaderStorageObject::VulkanShaderStorageObject(VulkanRenderEngineBackend* a_engine, uint32_t a_bufferSize, uint32_t a_count, const void* a_data)
{
    constexpr uint32_t CountSize = sizeof(int32_t);
    // Uniform memory must be aligned to 16 bytes
    constexpr uint32_t Offset = 16;

    const int32_t cVal = (int32_t)a_count;

    const VmaAllocator allocator = a_engine->GetAllocator();

    m_engine = a_engine;

    m_bufferSize = a_bufferSize;

    VkBufferCreateInfo bufferInfo = { };
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = (VkDeviceSize)a_bufferSize + Offset;
    bufferInfo.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VmaAllocationCreateInfo allocInfo = { 0 };
    allocInfo.usage = VMA_MEMORY_USAGE_AUTO;
    allocInfo.requiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
    allocInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;

    VmaAllocationInfo vmaAllocInfo = {};
        
    VkBuffer tBuffer;
    vmaCreateBuffer(allocator, (VkBufferCreateInfo*)&bufferInfo, &allocInfo, &tBuffer, &m_allocation, &vmaAllocInfo);
    m_buffer = tBuffer;

    constexpr uint32_t Align = Offset - CountSize;

    // Have to honour the sequential write flag
    // Funky stuff happens if you don't therefore memset is required despite the fact it is unused memory
    // GPUs are weird
    memcpy(vmaAllocInfo.pMappedData, &cVal, CountSize);
    memset((char*)vmaAllocInfo.pMappedData + CountSize, 0, Align);
    memcpy((char*)vmaAllocInfo.pMappedData + Offset, a_data, a_bufferSize);
}
VulkanShaderStorageObject::~VulkanShaderStorageObject()
{
    m_engine->PushDeletionObject(new VulkanSSBOBufferDeletionObject(m_engine, m_buffer, m_allocation));
}
#endif