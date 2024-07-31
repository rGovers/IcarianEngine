#ifdef ICARIANNATIVE_ENABLE_GRAPHICS_VULKAN

#include "Rendering/Vulkan/VulkanModel.h"

#include "Core/IcarianDefer.h"
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

    TLockObj<vk::CommandBuffer, SpinLock>* buffer = m_engine->BeginSingleCommand();
    IDEFER(m_engine->EndSingleCommand(buffer));

    const vk::CommandBuffer cmdBuffer = buffer->Get();

    TRACE("Creating Vertex Buffer");
    const VkBufferCreateInfo vBCreateInfo = 
    {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = (VkDeviceSize)vbSize,
        .usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
    };

    const VmaAllocationCreateInfo vBAllocInfo = 
    { 
        .flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_HOST_ACCESS_ALLOW_TRANSFER_INSTEAD_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT,
        .usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE,
    };

    VkBuffer tVertexBuffer;
    VmaAllocationInfo vBInfo;
    VKRESERRMSG(vmaCreateBuffer(allocator, &vBCreateInfo, &vBAllocInfo, &tVertexBuffer, &m_vbAlloc, &vBInfo), "Failed to create vertex buffer");
    m_vertexBuffer = tVertexBuffer;
#ifdef DEBUG
    vmaSetAllocationName(allocator, m_vbAlloc, "Model Vertex Buffer");
#endif

    VkMemoryPropertyFlags flags;
    vmaGetAllocationMemoryProperties(allocator, m_vbAlloc, &flags);

    if (flags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
    {
        IDEFER(vmaFlushAllocation(allocator, m_vbAlloc, 0, (VkDeviceSize)vbSize));

        memcpy(vBInfo.pMappedData, a_vertices, vbSize);

        const vk::BufferMemoryBarrier barrier = vk::BufferMemoryBarrier
        (
            vk::AccessFlagBits::eHostWrite,
            vk::AccessFlagBits::eVertexAttributeRead,
            VK_QUEUE_FAMILY_IGNORED,
            VK_QUEUE_FAMILY_IGNORED,
            m_vertexBuffer,
            0,
            VK_WHOLE_SIZE
        );

        cmdBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eHost, vk::PipelineStageFlagBits::eVertexInput, { }, 0, nullptr, 1, &barrier, 0, nullptr);
    }
    else
    {
        const VkBufferCreateInfo vBSInfo = 
        { 
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .size = (VkDeviceSize)vbSize,
            .usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        };

        const VmaAllocationCreateInfo vBSAInfo = 
        { 
            .flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT,
            .usage = VMA_MEMORY_USAGE_AUTO,
        };

        VkBuffer stagingVBuffer;
        VmaAllocation stagingVBAlloc;
        VmaAllocationInfo stagingVBInfo;
        VKRESERRMSG(vmaCreateBuffer(allocator, &vBSInfo, &vBSAInfo, &stagingVBuffer, &stagingVBAlloc, &stagingVBInfo), "Failed to create vertex staging buffer");
        IDEFER(m_engine->PushDeletionObject(new VulkanModelBufferDeletionObject(m_engine, stagingVBuffer, stagingVBAlloc)));
        IDEFER(VKRESERR(vmaFlushAllocation(allocator, stagingVBAlloc, 0, (VkDeviceSize)vbSize)));

#ifdef DEBUG
        vmaSetAllocationName(allocator, stagingVBAlloc, "Staging Vertex Buffer");
#endif

        memcpy(stagingVBInfo.pMappedData, a_vertices, vbSize);

        const vk::BufferCopy vBCopy = vk::BufferCopy(0, 0, (vk::DeviceSize)vbSize);
        cmdBuffer.copyBuffer(stagingVBuffer, m_vertexBuffer, 1, &vBCopy);
    }

    TRACE("Creating Index Buffer");
    const VkBufferCreateInfo iBCreateInfo = 
    { 
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = (VkDeviceSize)ibSize,
        .usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
    };

    const VmaAllocationCreateInfo iBAllocInfo = 
    { 
        .flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_HOST_ACCESS_ALLOW_TRANSFER_INSTEAD_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT,
        .usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE,
    };

    VkBuffer tIndexBuffer;
    VmaAllocationInfo iBInfo;
    VKRESERRMSG(vmaCreateBuffer(allocator, &iBCreateInfo, &iBAllocInfo, &tIndexBuffer, &m_ibAlloc, &iBInfo), "Failed to create index buffer");
    m_indexBuffer = tIndexBuffer;
#ifdef DEBUG
    vmaSetAllocationName(allocator, m_ibAlloc, "Model Index Buffer");
#endif

    vmaGetAllocationMemoryProperties(allocator, m_ibAlloc, &flags);

    if (flags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
    {
        IDEFER(vmaFlushAllocation(allocator, m_ibAlloc, 0, (VkDeviceSize)ibSize));

        memcpy(iBInfo.pMappedData, a_indices, ibSize);

        const vk::BufferMemoryBarrier barrier = vk::BufferMemoryBarrier
        (
            vk::AccessFlagBits::eHostWrite,
            vk::AccessFlagBits::eIndexRead,
            VK_QUEUE_FAMILY_IGNORED,
            VK_QUEUE_FAMILY_IGNORED,
            m_indexBuffer,
            0,
            VK_WHOLE_SIZE
        );

        cmdBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eHost, vk::PipelineStageFlagBits::eVertexInput, { }, 0, nullptr, 1, &barrier, 0, nullptr);
    }
    else
    {
        const VkBufferCreateInfo iBSInfo = 
        { 
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .size = (VkDeviceSize)ibSize,
            .usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        };

        const VmaAllocationCreateInfo iBSAInfo = 
        { 
            .flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT,
            .usage = VMA_MEMORY_USAGE_AUTO,
        };

        VkBuffer stagingIBuffer;
        VmaAllocation stagingIBAlloc;
        VmaAllocationInfo stagingIBInfo;
        VKRESERRMSG(vmaCreateBuffer(allocator, &iBSInfo, &iBSAInfo, &stagingIBuffer, &stagingIBAlloc, &stagingIBInfo), "Failed to create index staging buffer");
        IDEFER(m_engine->PushDeletionObject(new VulkanModelBufferDeletionObject(m_engine, stagingIBuffer, stagingIBAlloc)));
        IDEFER(VKRESERR(vmaFlushAllocation(allocator, stagingIBAlloc, 0, (VkDeviceSize)ibSize)));
#ifdef DEBUG
        vmaSetAllocationName(allocator, stagingIBAlloc, "Staging Index Buffer");
#endif

        memcpy(stagingIBInfo.pMappedData, a_indices, ibSize);

        const vk::BufferCopy iBCopy = vk::BufferCopy(0, 0, (vk::DeviceSize)ibSize);
        cmdBuffer.copyBuffer(stagingIBuffer, m_indexBuffer, 1, &iBCopy);
    }
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
#endif