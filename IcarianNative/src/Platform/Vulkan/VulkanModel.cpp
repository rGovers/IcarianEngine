// Icarian Engine - C# Game Engine
// 
// License at end of file.

#ifdef ICARIANNATIVE_ENABLE_GRAPHICS_VULKAN

#include "Rendering/Vulkan/VulkanModel.h"

#include "Core/IcarianDefer.h"
#include "Rendering/Vulkan/VulkanRenderEngineBackend.h"
#include "Trace.h"

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

    m_offset = vbSize;

    TLockObj<vk::CommandBuffer, SpinLock>* cmdBuffer = m_engine->BeginSingleCommand();
    IDEFER(m_engine->EndSingleCommand(cmdBuffer));

    const vk::CommandBuffer cmd = cmdBuffer->Get();

    TRACE("Creating Model Buffer");
    const VkBufferCreateInfo createInfo = 
    {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = (VkDeviceSize)vbSize + ibSize,
        .usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
    };

    const VmaAllocationCreateInfo allocInfo = 
    { 
        .flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_HOST_ACCESS_ALLOW_TRANSFER_INSTEAD_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT,
        .usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE,
    };

    VkBuffer buffer;
    VmaAllocationInfo bufferInfo;
    VKRESERRMSG(vmaCreateBuffer(allocator, &createInfo, &allocInfo, &buffer, &m_allocation, &bufferInfo), "Failed to create model buffer");
    m_buffer = buffer;
#ifdef DEBUG
    vmaSetAllocationName(allocator, m_allocation, "Model Buffer");
#endif

    VkMemoryPropertyFlags flags;
    vmaGetAllocationMemoryProperties(allocator, m_allocation, &flags);

    const bool cpuCanWrite = (flags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) != 0;
    if (cpuCanWrite)
    {
        IDEFER(vmaFlushAllocation(allocator, m_allocation, 0, (VkDeviceSize)vbSize));

        memcpy(bufferInfo.pMappedData, a_vertices, (size_t)vbSize);
        memcpy((uint8_t*)bufferInfo.pMappedData + m_offset, a_indices, (size_t)ibSize);

        const vk::BufferMemoryBarrier barrier = vk::BufferMemoryBarrier
        (
            vk::AccessFlagBits::eHostWrite,
            vk::AccessFlagBits::eVertexAttributeRead | vk::AccessFlagBits::eIndexRead,
            VK_QUEUE_FAMILY_IGNORED,
            VK_QUEUE_FAMILY_IGNORED,
            m_buffer,
            0,
            VK_WHOLE_SIZE
        );

        cmd.pipelineBarrier(vk::PipelineStageFlagBits::eHost, vk::PipelineStageFlagBits::eVertexInput, { }, 0, nullptr, 1, &barrier, 0, nullptr);

        return;
    }

    const VkBufferCreateInfo sCreateInfo = 
    { 
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = (VkDeviceSize)vbSize,
        .usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
    };

    const VmaAllocationCreateInfo sAllocInfo = 
    { 
        .flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT,
        .usage = VMA_MEMORY_USAGE_AUTO,
    };

    VkBuffer stagingBuffer;
    VmaAllocation stagingAlloc;
    VmaAllocationInfo stagingInfo;
    VKRESERRMSG(vmaCreateBuffer(allocator, &sCreateInfo, &sAllocInfo, &stagingBuffer, &stagingAlloc, &stagingInfo), "Failed to create model staging buffer");
    IDEFER(m_engine->PushDeletionObject(new VulkanModelBufferDeletionObject(m_engine, stagingBuffer, stagingAlloc)));
    IDEFER(VKRESERR(vmaFlushAllocation(allocator, stagingAlloc, 0, (VkDeviceSize)vbSize)));

#ifdef DEBUG
    vmaSetAllocationName(allocator, stagingAlloc, "Staging Model Buffer");
#endif

    memcpy(stagingInfo.pMappedData, a_vertices, vbSize);
    memcpy((uint8_t*)stagingInfo.pMappedData + m_offset, a_indices, ibSize);

    const vk::BufferCopy copy = vk::BufferCopy(0, 0, (vk::DeviceSize)vbSize + ibSize);
    cmd.copyBuffer(stagingBuffer, m_buffer, 1, &copy);
}   
VulkanModel::~VulkanModel()
{
    TRACE("Queuing Model Deletion");
    m_engine->PushDeletionObject(new VulkanModelBufferDeletionObject(m_engine, m_buffer, m_allocation));
}

void VulkanModel::Bind(const vk::CommandBuffer& a_cmdBuffer) const
{
    constexpr vk::DeviceSize Offsets[] = { 0 };

    a_cmdBuffer.bindVertexBuffers(0, 1, &m_buffer, Offsets);
    a_cmdBuffer.bindIndexBuffer(m_buffer, m_offset, vk::IndexType::eUint32);
}

#endif

// MIT License
// 
// Copyright (c) 2024 River Govers
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.