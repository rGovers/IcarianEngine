#include "Rendering/Vulkan/VulkanModel.h"

#include "Flare/IcarianAssert.h"
#include "Flare/IcarianDefer.h"
#include "Logger.h"
#include "Rendering/Vulkan/VulkanRenderEngineBackend.h"
#include "Trace.h"

class VulkanModelDeletionObject : public VulkanDeletionObject
{
private:
    VulkanRenderEngineBackend* m_engine;

    VmaAllocation              m_vbAlloc;
    VmaAllocation              m_ibAlloc;
         
    vk::Buffer                 m_vertexBuffer;
    vk::Buffer                 m_indexBuffer;

protected:

public:
    VulkanModelDeletionObject(VulkanRenderEngineBackend* a_engine, VmaAllocation a_vbAlloc, VmaAllocation a_ibAlloc, vk::Buffer a_vertexBuffer, vk::Buffer a_indexBuffer)
    {
        m_engine = a_engine;

        m_vbAlloc = a_vbAlloc;
        m_ibAlloc = a_ibAlloc;

        m_vertexBuffer = a_vertexBuffer;
        m_indexBuffer = a_indexBuffer;
    }
    virtual ~VulkanModelDeletionObject()
    {

    }

    virtual void Destroy() override
    {
        TRACE("Destroying Model");
        const VmaAllocator allocator = m_engine->GetAllocator();

        vmaDestroyBuffer(allocator, m_vertexBuffer, m_vbAlloc);
        vmaDestroyBuffer(allocator, m_indexBuffer, m_ibAlloc);
    }
};

class VulkanModelBufferDeletionObject : public VulkanDeletionObject
{
private:
    VulkanRenderEngineBackend* m_engine;

    vk::Buffer                 m_buffer;
    VmaAllocation              m_allocation;    

protected:

public:
    VulkanModelBufferDeletionObject(VulkanRenderEngineBackend* a_engine, vk::Buffer a_buffer, VmaAllocation a_allocation)
    {
        m_engine = a_engine;

        m_buffer = a_buffer;
        m_allocation = a_allocation;
    }
    virtual ~VulkanModelBufferDeletionObject()
    {

    }

    virtual void Destroy() 
    {
        TRACE("Destroying Model Buffer");
        const VmaAllocator allocator = m_engine->GetAllocator();

        vmaDestroyBuffer(allocator, m_buffer, m_allocation);
    }
};

VulkanModel::VulkanModel(VulkanRenderEngineBackend* a_engine, uint32_t a_vertexCount, const void* a_vertices, uint16_t a_vertexSize, uint32_t a_indexCount, const uint32_t* a_indices, float a_radius)
{
    TRACE("Creating Vulkan Model");
    m_engine = a_engine;

    m_indexCount = a_indexCount;
    m_radius = a_radius;

    const vk::Device device = m_engine->GetLogicalDevice();
    const VmaAllocator allocator = m_engine->GetAllocator();

    const uint32_t vbSize = a_vertexCount * a_vertexSize;
    const uint32_t ibSize = a_indexCount * sizeof(uint32_t);

    TRACE("Creating Staging Vertex Buffer");
    VkBufferCreateInfo vBInfo = { };
    vBInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    vBInfo.size = (VkDeviceSize)vbSize;
    vBInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    vBInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VmaAllocationCreateInfo vBAInfo = { 0 };
    vBAInfo.usage = VMA_MEMORY_USAGE_AUTO;
    vBAInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;

    VkBuffer stagingVBuffer = VK_NULL_HANDLE;
    VmaAllocation stagingVBAlloc = VK_NULL_HANDLE;
    VmaAllocationInfo stagingVBInfo = { 0 };
    ICARIAN_ASSERT_MSG_R(vmaCreateBuffer(allocator, &vBInfo, &vBAInfo, &stagingVBuffer, &stagingVBAlloc, &stagingVBInfo) == VK_SUCCESS, "Failed to create vertex staging buffer");
    
    IDEFER(m_engine->PushDeletionObject(new VulkanModelBufferDeletionObject(m_engine, stagingVBuffer, stagingVBAlloc)));

    memcpy(stagingVBInfo.pMappedData, a_vertices, vbSize);

    TRACE("Creating Vertex Buffer");
    vBInfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    vBAInfo.flags = 0;

    VkBuffer tVertexBuffer;
    ICARIAN_ASSERT_MSG_R(vmaCreateBuffer(allocator, &vBInfo, &vBAInfo, &tVertexBuffer, &m_vbAlloc, nullptr) == VK_SUCCESS, "Failed to create vertex buffer");
    m_vertexBuffer = tVertexBuffer;

    TRACE("Creating Staging Index Buffers");
    VkBufferCreateInfo iBInfo = { };
    iBInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    iBInfo.size = (VkDeviceSize)ibSize;
    iBInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    iBInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VmaAllocationCreateInfo iBAInfo = { 0 };
    iBAInfo.usage = VMA_MEMORY_USAGE_AUTO;
    iBAInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;

    VkBuffer stagingIBuffer = VK_NULL_HANDLE;
    VmaAllocation stagingIBAlloc = VK_NULL_HANDLE;
    VmaAllocationInfo stagingIBInfo = { 0 };
    ICARIAN_ASSERT_MSG_R(vmaCreateBuffer(allocator, &iBInfo, &iBAInfo, &stagingIBuffer, &stagingIBAlloc, &stagingIBInfo) == VK_SUCCESS, "Failed to create index staging buffer");

    IDEFER(m_engine->PushDeletionObject(new VulkanModelBufferDeletionObject(m_engine, stagingIBuffer, stagingIBAlloc)));

    memcpy(stagingIBInfo.pMappedData, a_indices, ibSize);

    TRACE("Creating Index Buffer");
    iBInfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    iBAInfo.flags = 0;

    VkBuffer tIndexBuffer;
    ICARIAN_ASSERT_MSG_R(vmaCreateBuffer(allocator, &iBInfo, &iBAInfo, &tIndexBuffer, &m_ibAlloc, nullptr) == VK_SUCCESS, "Failed to create index buffer");
    m_indexBuffer = tIndexBuffer;

    TRACE("Copying buffers");
    TLockObj<vk::CommandBuffer, std::mutex>* buffer = m_engine->BeginSingleCommand();
    const vk::CommandBuffer cmdBuffer = buffer->Get();

    const vk::BufferCopy vBCopy = vk::BufferCopy(0, 0, (vk::DeviceSize)vbSize);
    cmdBuffer.copyBuffer(stagingVBuffer, m_vertexBuffer, 1, &vBCopy);

    const vk::BufferCopy iBCopy = vk::BufferCopy(0, 0, (vk::DeviceSize)ibSize);
    cmdBuffer.copyBuffer(stagingIBuffer, m_indexBuffer, 1, &iBCopy);

    m_engine->EndSingleCommand(buffer);
}   
VulkanModel::~VulkanModel()
{
    TRACE("Queuing Model Deletion");
    m_engine->PushDeletionObject(new VulkanModelDeletionObject(m_engine, m_vbAlloc, m_ibAlloc, m_vertexBuffer, m_indexBuffer));
}

void VulkanModel::Bind(const vk::CommandBuffer& a_cmdBuffer) const
{
    constexpr vk::DeviceSize Offsets[] = { 0 };

    a_cmdBuffer.bindVertexBuffers(0, 1, &m_vertexBuffer, Offsets);
    a_cmdBuffer.bindIndexBuffer(m_indexBuffer, 0, vk::IndexType::eUint32);
}