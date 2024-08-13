// Icarian Engine - C# Game Engine
// 
// License at end of file.

#ifdef ICARIANNATIVE_ENABLE_GRAPHICS_VULKAN

#include "Rendering/Vulkan/VulkanTexture.h"

#include "Core/IcarianAssert.h"
#include "Core/IcarianDefer.h"
#include "Rendering/Vulkan/VulkanRenderEngineBackend.h"
#include "Trace.h"

class VulkanTextureDeletionObject : public VulkanDeletionObject
{
private:
    VulkanRenderEngineBackend* m_engine;

    vk::Image                  m_image;
    vk::ImageView              m_view;
    VmaAllocation              m_allocation;

protected:

public:
    VulkanTextureDeletionObject(VulkanRenderEngineBackend* a_engine, vk::Image a_image, vk::ImageView a_view, VmaAllocation a_allocation)
    {
        m_engine = a_engine;

        m_image = a_image;
        m_view = a_view;
        m_allocation = a_allocation;
    }
    virtual ~VulkanTextureDeletionObject()
    {
        
    }

    virtual void Destroy()
    {
        TRACE("Destroying Texture");
        const vk::Device device = m_engine->GetLogicalDevice();
        const VmaAllocator allocator = m_engine->GetAllocator();

        // Even if it is not in use have to delete the view first cause Vulkan
        device.destroyImageView(m_view);
        vmaDestroyImage(allocator, m_image, m_allocation);
    }
};
class VulkanTextureBufferDeletionObject : public VulkanDeletionObject
{
private:
    VulkanRenderEngineBackend* m_engine;

    vk::Buffer                 m_buffer;
    VmaAllocation              m_allocation;

protected:

public:
    VulkanTextureBufferDeletionObject(VulkanRenderEngineBackend* a_engine, vk::Buffer a_buffer, VmaAllocation a_allocation)
    {
        m_engine = a_engine;

        m_buffer = a_buffer;
        m_allocation = a_allocation;
    }
    virtual ~VulkanTextureBufferDeletionObject()
    {
        
    }

    virtual void Destroy()
    {
        TRACE("Destroying Texture Buffer");
        const VmaAllocator allocator = m_engine->GetAllocator();

        vmaDestroyBuffer(allocator, m_buffer, m_allocation);
    }
};

constexpr static vk::Format ToVulkanFormat(e_TextureFormat a_format)
{
    switch (a_format)
    {
    case TextureFormat_Alpha:
    {
        return vk::Format::eR8Unorm;
    }
    case TextureFormat_RGBA:
    {
        return vk::Format::eR8G8B8A8Srgb;
    }
    case TextureFormat_BC3:
    {
        return vk::Format::eBc3UnormBlock;
    }
    case TextureFormat_BC7:
    {
        return vk::Format::eBc7UnormBlock;
    }
    }

    return vk::Format::eR8G8B8A8Srgb;
}
constexpr static uint32_t ToChannels(e_TextureFormat a_format)
{
    switch (a_format)
    {
    case TextureFormat_Alpha:
    {
        return 1;
    }
    case TextureFormat_RGBA:
    {
        return 4;
    }
    case TextureFormat_BC3:
    {
        return 4;
    }
    case TextureFormat_BC7:
    {
        return 4;
    }
    }

    return 4;
}

void VulkanTexture::InitBase(const void* a_data, vk::Format a_format, uint32_t a_channels, uint64_t a_dataSize)
{
    m_channels = a_channels;
    m_format = a_format;

    vk::DeviceSize imageSize = (vk::DeviceSize)a_dataSize;
    if (imageSize == -1)
    {
        imageSize = (vk::DeviceSize)m_width * m_height * m_channels;
    }

    const vk::Device device = m_engine->GetLogicalDevice();
    const VmaAllocator allocator = m_engine->GetAllocator();
    
    const vk::Extent3D extent = vk::Extent3D(m_width, m_height, 1);

    const VkImageCreateInfo imageInfo = 
    { 
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .imageType = VK_IMAGE_TYPE_2D,
        .format = (VkFormat)m_format,
        .extent = extent,
        .mipLevels = 1,
        .arrayLayers = 1,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .tiling = VK_IMAGE_TILING_OPTIMAL,
        .usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
    };

    const VmaAllocationCreateInfo allocInfo = 
    { 
        .usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE
    };

    VkImage image;
    VKRESERRMSG(vmaCreateImage(allocator, &imageInfo, &allocInfo, &image, &m_allocation, NULL), "Failed to create VulkanTexture image");
    m_image = image;
#ifdef DEBUG
    vmaSetAllocationName(allocator, m_allocation, "Texture");
#endif

    constexpr vk::ImageSubresourceRange SubresourceRange = vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1);

    const vk::ImageViewCreateInfo viewInfo = vk::ImageViewCreateInfo
    (
        { }, 
        m_image, 
        vk::ImageViewType::e2D, 
        m_format,
        { vk::ComponentSwizzle::eIdentity, vk::ComponentSwizzle::eIdentity, vk::ComponentSwizzle::eIdentity, vk::ComponentSwizzle::eIdentity },
        SubresourceRange
    );

    VKRESERRMSG(device.createImageView(&viewInfo, nullptr, &m_imageView), "Failed to create VulkanTexture image view");

    WriteData(a_data, true);
}
void VulkanTexture::InitMipMapped(uint32_t a_levels, const uint64_t* a_offsets, const void* a_data, vk::Format a_format, uint32_t a_channels, uint64_t a_dataSize)
{
    m_channels = a_channels;
    m_format = a_format;

    const vk::Device device = m_engine->GetLogicalDevice();
    const VmaAllocator allocator = m_engine->GetAllocator();

    const vk::Extent3D extent = vk::Extent3D(m_width, m_height, 1);

    const VkImageCreateInfo imageInfo = 
    { 
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .imageType = VK_IMAGE_TYPE_2D,
        .format = (VkFormat)m_format,
        .extent = extent,
        .mipLevels = a_levels,
        .arrayLayers = 1,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .tiling = VK_IMAGE_TILING_OPTIMAL,
        .usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
    };
    
    const VmaAllocationCreateInfo allocInfo = 
    { 
        .usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE,
    };

    VkImage image;
    VKRESERRMSG(vmaCreateImage(allocator, &imageInfo, &allocInfo, &image, &m_allocation, NULL), "Failed to create VulkanTexture image");
    m_image = image;
#ifdef DEBUG
    vmaSetAllocationName(allocator, m_allocation, "MipTexture");
#endif

    const vk::ImageSubresourceRange subresourceRange = vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, a_levels, 0, 1);

    const vk::ImageViewCreateInfo viewInfo = vk::ImageViewCreateInfo
    (
        { }, 
        m_image, 
        vk::ImageViewType::e2D, 
        m_format,
        { vk::ComponentSwizzle::eIdentity, vk::ComponentSwizzle::eIdentity, vk::ComponentSwizzle::eIdentity, vk::ComponentSwizzle::eIdentity },
        subresourceRange
    );

    VKRESERRMSG(device.createImageView(&viewInfo, nullptr, &m_imageView), "Failed to create VulkanTexture image view");

    const VkBufferCreateInfo stagingBufferInfo = 
    { 
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = (VkDeviceSize)a_dataSize,
        .usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
    };

    const VmaAllocationCreateInfo stagingBufferAllocInfo = 
    { 
        .flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT,
        .usage = VMA_MEMORY_USAGE_AUTO,
    };
    
    VkBuffer stagingBuffer;
    VmaAllocation stagingAllocation;

    VmaAllocationInfo stagingAllocationInfo;
    VKRESERRMSG(vmaCreateBuffer(allocator, &stagingBufferInfo, &stagingBufferAllocInfo, &stagingBuffer, &stagingAllocation, &stagingAllocationInfo), "Failed to create staging texture");
    IDEFER(m_engine->PushDeletionObject(new VulkanTextureBufferDeletionObject(m_engine, stagingBuffer, stagingAllocation)));
#ifdef DEBUG
    vmaSetAllocationName(allocator, stagingAllocation, "StagingMipTexture");
#endif

    if (a_data != nullptr)
    {
        IDEFER(ICARIAN_ASSERT_R(vmaFlushAllocation(allocator, stagingAllocation, 0, (VkDeviceSize)a_dataSize) == VK_SUCCESS));
        memcpy(stagingAllocationInfo.pMappedData, a_data, (size_t)a_dataSize);
    }

    vk::BufferImageCopy* copyBuffers = new vk::BufferImageCopy[a_levels];
    IDEFER(delete[] copyBuffers);
    for (uint32_t i = 0; i < a_levels; ++i)
    {
        const vk::ImageSubresourceLayers layer = vk::ImageSubresourceLayers(vk::ImageAspectFlagBits::eColor, i, 0, 1);
        const vk::Extent3D extents = vk::Extent3D(m_width >> i, m_height >> i, 1);

        copyBuffers[i] = vk::BufferImageCopy
        (
            (vk::DeviceSize)a_offsets[i], 
            0, 
            0, 
            layer, 
            0,
            extents
        );
    }

    TLockObj<vk::CommandBuffer, SpinLock>* buffer = m_engine->BeginSingleCommand();
    IDEFER(m_engine->EndSingleCommand(buffer));

    const vk::CommandBuffer cmd = buffer->Get();

    const vk::ImageMemoryBarrier startImageBarrier = vk::ImageMemoryBarrier
    (
        { },
        vk::AccessFlagBits::eTransferWrite, 
        vk::ImageLayout::eUndefined, 
        vk::ImageLayout::eTransferDstOptimal, 
        VK_QUEUE_FAMILY_IGNORED, 
        VK_QUEUE_FAMILY_IGNORED, 
        m_image, 
        subresourceRange
    );

    cmd.pipelineBarrier(vk::PipelineStageFlagBits::eFragmentShader, vk::PipelineStageFlagBits::eTransfer, { }, 0, nullptr, 0, nullptr, 1, &startImageBarrier);

    cmd.copyBufferToImage(stagingBuffer, m_image, vk::ImageLayout::eTransferDstOptimal, a_levels, copyBuffers);

    const vk::ImageMemoryBarrier endImageBarrier = vk::ImageMemoryBarrier
    (
        vk::AccessFlagBits::eTransferWrite,
        vk::AccessFlagBits::eShaderRead, 
        vk::ImageLayout::eTransferDstOptimal, 
        vk::ImageLayout::eShaderReadOnlyOptimal, 
        VK_QUEUE_FAMILY_IGNORED, 
        VK_QUEUE_FAMILY_IGNORED, 
        m_image, 
        subresourceRange
    );

    cmd.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eFragmentShader, { }, 0, nullptr, 0, nullptr, 1, &endImageBarrier);
}

VulkanTexture::VulkanTexture()
{

}
VulkanTexture::~VulkanTexture()
{
    TRACE("Queueing Texture Deletion");
    m_engine->PushDeletionObject(new VulkanTextureDeletionObject(m_engine, m_image, m_imageView, m_allocation));
}

VulkanTexture* VulkanTexture::CreateTexture(VulkanRenderEngineBackend* a_engine, uint32_t a_width, uint32_t a_height, e_TextureFormat a_format, const void* a_data, uint64_t a_dataSize)
{
    TRACE("Creating Texture");

    VulkanTexture* texture = new VulkanTexture();
    texture->m_engine = a_engine;
    texture->m_width = a_width;
    texture->m_height = a_height;
    texture->InitBase(a_data, ToVulkanFormat(a_format), ToChannels(a_format), a_dataSize);

    return texture;
}
VulkanTexture* VulkanTexture::CreateTextureMipMapped(VulkanRenderEngineBackend* a_engine, uint32_t a_width, uint32_t a_height, uint32_t a_levels, const uint64_t* a_offsets, e_TextureFormat a_format, const void* a_data, uint64_t a_dataSize)
{
    VulkanTexture* texture = new VulkanTexture();
    texture->m_engine = a_engine;
    texture->m_width = a_width;
    texture->m_height = a_height;
    texture->InitMipMapped(a_levels, a_offsets, a_data, ToVulkanFormat(a_format), ToChannels(a_format), a_dataSize);

    return texture;
}

void VulkanTexture::WriteData(const void* a_data, bool a_init)
{
    const VmaAllocator allocator = m_engine->GetAllocator();

    const vk::DeviceSize imageSize = (vk::DeviceSize)m_width * m_height * m_channels;

    TLockObj<vk::CommandBuffer, SpinLock>* buffer = m_engine->BeginSingleCommand();
    IDEFER(m_engine->EndSingleCommand(buffer));

    const vk::CommandBuffer cmd = buffer->Get();

    constexpr vk::ImageSubresourceRange SubresourceRange = vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1);

    const VkBufferCreateInfo stagingBufferInfo = 
    {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = imageSize,
        .usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT
    };

    const VmaAllocationCreateInfo stagingBufferAllocInfo = 
    { 
        .flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT,
        .usage = VMA_MEMORY_USAGE_AUTO,
    };
    
    VkBuffer stagingBuffer;
    VmaAllocation stagingAllocation;
    VmaAllocationInfo stagingAllocationInfo;
    VKRESERRMSG(vmaCreateBuffer(allocator, &stagingBufferInfo, &stagingBufferAllocInfo, &stagingBuffer, &stagingAllocation, &stagingAllocationInfo), "Failed to create staging texture");
    IDEFER(m_engine->PushDeletionObject(new VulkanTextureBufferDeletionObject(m_engine, stagingBuffer, stagingAllocation)));
#ifdef DEBUG
    vmaSetAllocationName(allocator, stagingAllocation, "StagingTexture");
#endif

    if (a_data != nullptr)
    {
        IDEFER(VKRESERR(vmaFlushAllocation(allocator, stagingAllocation, 0, (VkDeviceSize)imageSize)));
        memcpy(stagingAllocationInfo.pMappedData, a_data, (size_t)imageSize);
    }

    constexpr vk::ImageSubresourceLayers SubresourceLayers = vk::ImageSubresourceLayers(vk::ImageAspectFlagBits::eColor, 0, 0, 1);

    if (a_init)
    {
        const vk::ImageMemoryBarrier startImageBarrier = vk::ImageMemoryBarrier
        (
            { },
            vk::AccessFlagBits::eTransferWrite, 
            vk::ImageLayout::eUndefined, 
            vk::ImageLayout::eTransferDstOptimal, 
            VK_QUEUE_FAMILY_IGNORED, 
            VK_QUEUE_FAMILY_IGNORED, 
            m_image, 
            SubresourceRange
        );

        cmd.pipelineBarrier(vk::PipelineStageFlagBits::eTopOfPipe, vk::PipelineStageFlagBits::eTransfer, { }, 0, nullptr, 0, nullptr, 1, &startImageBarrier);
    }
    else 
    {
        const vk::ImageMemoryBarrier startImageBarrier = vk::ImageMemoryBarrier
        (
            vk::AccessFlagBits::eShaderRead,
            vk::AccessFlagBits::eTransferWrite, 
            vk::ImageLayout::eShaderReadOnlyOptimal, 
            vk::ImageLayout::eTransferDstOptimal, 
            VK_QUEUE_FAMILY_IGNORED, 
            VK_QUEUE_FAMILY_IGNORED, 
            m_image, 
            SubresourceRange
        );

        cmd.pipelineBarrier(vk::PipelineStageFlagBits::eFragmentShader, vk::PipelineStageFlagBits::eTransfer, { }, 0, nullptr, 0, nullptr, 1, &startImageBarrier);
    }

    const vk::BufferImageCopy copyRegion = vk::BufferImageCopy(0, 0, 0, SubresourceLayers, { 0, 0, 0 }, { m_width, m_height, 1 });

    // Forgot about tiling and is driver specific so had to revert to using staging buffers only
    cmd.copyBufferToImage(stagingBuffer, m_image, vk::ImageLayout::eTransferDstOptimal, 1, &copyRegion);

    const vk::ImageMemoryBarrier endImageBarrier = vk::ImageMemoryBarrier
    (
        vk::AccessFlagBits::eTransferWrite,
        vk::AccessFlagBits::eShaderRead, 
        vk::ImageLayout::eTransferDstOptimal, 
        vk::ImageLayout::eShaderReadOnlyOptimal, 
        VK_QUEUE_FAMILY_IGNORED, 
        VK_QUEUE_FAMILY_IGNORED, 
        m_image, 
        SubresourceRange
    );

    cmd.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eFragmentShader, { }, 0, nullptr, 0, nullptr, 1, &endImageBarrier);
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