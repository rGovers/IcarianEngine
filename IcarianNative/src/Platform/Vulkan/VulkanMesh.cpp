// Icarian Engine - C# Game Engine
//
// License at end of file.

#ifdef ICARIANNATIVE_ENABLE_GRAPHICS_VULKAN

#include "Rendering/Vulkan/VulkanMesh.h"

#include "Rendering/Vulkan/VulkanRenderEngineBackend.h"
#include "Trace.h"

class VulkanMeshBufferDeletionObject : public VulkanDeletionObject
{
private:
    VulkanRenderEngineBackend* m_engine;

    vk::Buffer                 m_buffer;
    VmaAllocation              m_allocation;

protected:

public:
    VulkanMeshBufferDeletionObject(VulkanRenderEngineBackend* a_engine, vk::Buffer a_buffer, VmaAllocation a_allocation)
    {
        m_engine = a_engine;

        m_buffer = a_buffer;
        m_allocation = a_allocation;
    }
    virtual ~VulkanMeshBufferDeletionObject()
    {

    }

    virtual void Destroy()
    {
        TRACE("Destroying Mesh Buffer");
        const VmaAllocator allocator = m_engine->GetVMAAllocator();

        vmaDestroyBuffer(allocator, m_buffer, m_allocation);
    }
};

template<typename T>
constexpr static T Align(T a_offset)
{
    unsigned int alignOffset = a_offset % 16;
    if (alignOffset != 0)
    {
        alignOffset = 16 - alignOffset;
    }

    return a_offset + alignOffset;
}

VulkanMesh::VulkanMesh
(
    VulkanRenderEngineBackend* a_engine,
    const void* a_vertices,
    uint32_t a_vertexCount,
    uint16_t a_vertexSize,
    const uint32_t* a_meshVertices,
    uint32_t a_meshVertexCount,
    const uint8_t* a_meshTriangles,
    uint32_t a_meshTriangleCount,
    const IcarianCore::ShaderMeshletBuffer* a_meshlets,
    uint32_t a_meshletCount,
    float a_radius
)
{
    TRACE("Creating Vulkan Mesh");
    m_engine = a_engine;

    m_meshletCount = a_meshletCount;
    m_radius = a_radius;

    const VmaAllocator allocator = m_engine->GetVMAAllocator();

    const uint32_t vbSize = a_vertexCount * a_vertexSize;
    const uint32_t mvSize = a_meshVertexCount * sizeof(uint32_t);
    const uint32_t mtSize = a_meshTriangleCount * sizeof(uint8_t) * 3;
    const uint32_t mbSize = a_meshletCount * sizeof(IcarianCore::ShaderMeshletBuffer);

    m_meshletVertexOffset = (vk::DeviceSize)Align(vbSize);
    const vk::DeviceSize mvEnd = m_meshletVertexOffset + mvSize;
    m_meshletTriangleOffset = Align(mvEnd);
    const vk::DeviceSize mtEnd = m_meshletTriangleOffset + mtSize;
    m_meshletOffset = Align(mtEnd);
    const vk::DeviceSize end = m_meshletOffset + mbSize;

    const vk::DeviceSize bufferSize = Align(end);

    const unsigned int vDiff = (unsigned int)(m_meshletVertexOffset - vbSize);
    const unsigned int mvDiff = (unsigned int)(m_meshletTriangleOffset - mvEnd);
    const unsigned int mtDiff = (unsigned int)(m_meshletOffset - mtEnd);
    const unsigned int mDiff = (unsigned int)(bufferSize - end);

    TLockObj<vk::CommandBuffer, SpinLock>* cmdBuffer = m_engine->BeginSingleCommand();
    IDEFER(m_engine->EndSingleCommand(cmdBuffer));

    const vk::CommandBuffer cmd = cmdBuffer->Get();

    TRACE("Creating Mesh Buffer");
    const VkBufferCreateInfo createInfo =
    {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = bufferSize,
        .usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE
    };

    const VmaAllocationCreateInfo allocInfo =
    {
        .flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_HOST_ACCESS_ALLOW_TRANSFER_INSTEAD_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT,
        .usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE
    };

    VkBuffer buffer;
    VmaAllocationInfo bufferInfo;
    VKRESERRMSG(vmaCreateBuffer(allocator, &createInfo, &allocInfo, &buffer, &m_allocation, &bufferInfo), "Failed to create Mesh Vertex buffer");
    m_buffer = buffer;
#ifdef  DEBUG
    vmaSetAllocationName(allocator, m_allocation, "Mesh Buffer");
#endif

    VkMemoryPropertyFlags flags;
    vmaGetAllocationMemoryProperties(allocator, m_allocation, &flags);

    const bool cpuCanWrite = (flags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) != 0;
    if (cpuCanWrite)
    {
        IDEFER(vmaFlushAllocation(allocator, m_allocation, 0, VK_WHOLE_SIZE));

        // Copy all the meshlet data + vertex data into a buffer
        // Need alignment + honour the sequential write so memset for padding
        memcpy(bufferInfo.pMappedData, a_vertices, vbSize);
        memset((uint8_t*)bufferInfo.pMappedData + vbSize, 0, vDiff);
        memcpy((uint8_t*)bufferInfo.pMappedData + m_meshletVertexOffset, a_meshVertices, mvSize);
        memset((uint8_t*)bufferInfo.pMappedData + mvEnd, 0, mvDiff);
        memcpy((uint8_t*)bufferInfo.pMappedData + m_meshletTriangleOffset, a_meshTriangles, mtSize);
        memset((uint8_t*)bufferInfo.pMappedData + mtEnd, 0, mtDiff);
        memcpy((uint8_t*)bufferInfo.pMappedData + m_meshletOffset, a_meshlets, mbSize);
        memset((uint8_t*)bufferInfo.pMappedData + end, 0, mDiff);

        const vk::BufferMemoryBarrier barrier = vk::BufferMemoryBarrier
        (
            vk::AccessFlagBits::eHostWrite,
            vk::AccessFlagBits::eShaderRead,
            vk::QueueFamilyIgnored,
            vk::QueueFamilyIgnored,
            m_buffer,
            0,
            vk::WholeSize
        );

        cmd.pipelineBarrier(vk::PipelineStageFlagBits::eHost, vk::PipelineStageFlagBits::eTaskShaderEXT, { }, 0, nullptr, 1, &barrier, 0, nullptr);

        return;
    }

    const VkBufferCreateInfo sCreateInfo =
    {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = bufferSize,
        .usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE
    };

    const VmaAllocationCreateInfo sAllocInfo =
    {
        .flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT,
        .usage = VMA_MEMORY_USAGE_AUTO
    };

    VkBuffer stagingBuffer;
    VmaAllocation stagingAlloc;
    VmaAllocationInfo stagingInfo;
    VKRESERRMSG(vmaCreateBuffer(allocator, &sCreateInfo, &sAllocInfo, &stagingBuffer, &stagingAlloc, &stagingInfo), "Failed to create mesh staging buffer");
    IDEFER(m_engine->PushDeletionObject<VulkanMeshBufferDeletionObject>(m_engine, stagingBuffer, stagingAlloc));
    IDEFER(vmaFlushAllocation(allocator, stagingAlloc, 0, VK_WHOLE_SIZE));

#ifdef DEBUG
    vmaSetAllocationName(allocator, stagingAlloc, "Staging Mesh Buffer");
#endif

    memcpy(stagingInfo.pMappedData, a_vertices, vbSize);
    memset((uint8_t*)stagingInfo.pMappedData + vbSize, 0, vDiff);
    memcpy((uint8_t*)stagingInfo.pMappedData + m_meshletVertexOffset, a_meshVertices, mvSize);
    memset((uint8_t*)stagingInfo.pMappedData + mvEnd, 0, mvDiff);
    memcpy((uint8_t*)stagingInfo.pMappedData + m_meshletTriangleOffset, a_meshTriangles, mtSize);
    memset((uint8_t*)stagingInfo.pMappedData + mtEnd, 0, mtDiff);
    memcpy((uint8_t*)stagingInfo.pMappedData + m_meshletOffset, a_meshlets, mbSize);
    memset((uint8_t*)stagingInfo.pMappedData + end, 0, mDiff);

    const vk::BufferCopy copy = vk::BufferCopy(0, 0, bufferSize);
    cmd.copyBuffer(stagingBuffer, m_buffer, 1, &copy);
}
VulkanMesh::~VulkanMesh()
{
    TRACE("Queuing Mesh Deletion");
    m_engine->PushDeletionObject<VulkanMeshBufferDeletionObject>(m_engine, m_buffer, m_allocation);
}

#endif

// MIT License
//
// Copyright (c) 2026 River Govers
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
