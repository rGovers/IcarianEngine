#ifdef ICARIANNATIVE_ENABLE_GRAPHICS_VULKAN

#include "Rendering/Vulkan/VulkanTexture.h"

#include "Flare/IcarianAssert.h"
#include "Flare/IcarianDefer.h"
#include "Rendering/Vulkan/VulkanRenderEngineBackend.h"
#include "Trace.h"

class VulkanTextureDeletionObject : public VulkanDeletionObject
{
private:
    VulkanRenderEngineBackend* m_engine;

    vk::Image                  m_image;
    VmaAllocation              m_allocation;

protected:

public:
    VulkanTextureDeletionObject(VulkanRenderEngineBackend* a_engine, vk::Image a_image, VmaAllocation a_allocation)
    {
        m_engine = a_engine;

        m_image = a_image;
        m_allocation = a_allocation;
    }
    virtual ~VulkanTextureDeletionObject()
    {
        
    }

    virtual void Destroy()
    {
        TRACE("Destroying Texture");
        const VmaAllocator allocator = m_engine->GetAllocator();

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
    
    VkImageCreateInfo imageInfo = { };
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = m_width;
    imageInfo.extent.height = m_height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = (VkFormat)m_format;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.flags = 0;

    VmaAllocationCreateInfo allocInfo = { 0 };
    allocInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
    allocInfo.priority = 1.0f;

    VkImage image;
    ICARIAN_ASSERT_MSG_R(vmaCreateImage(allocator, &imageInfo, &allocInfo, &image, &m_allocation, NULL) == VK_SUCCESS, "Failed to create VulkanTexture image");
    m_image = image;

    WriteData(a_data, true);
}
void VulkanTexture::InitMipMapped(uint32_t a_levels, const uint64_t* a_offsets, const void* a_data, vk::Format a_format, uint32_t a_channels, uint64_t a_dataSize)
{
    m_channels = a_channels;
    m_format = a_format;

    const vk::Device device = m_engine->GetLogicalDevice();
    const VmaAllocator allocator = m_engine->GetAllocator();

    VkImageCreateInfo imageInfo = { };
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = m_width;
    imageInfo.extent.height = m_height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = a_levels;
    imageInfo.arrayLayers = 1;
    imageInfo.format = (VkFormat)m_format;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.flags = 0;

    VmaAllocationCreateInfo allocInfo = { 0 };
    allocInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
    allocInfo.priority = 1.0f;

    VkImage image;
    ICARIAN_ASSERT_MSG_R(vmaCreateImage(allocator, &imageInfo, &allocInfo, &image, &m_allocation, NULL) == VK_SUCCESS, "Failed to create VulkanTexture image");
    m_image = image;

    VkBufferCreateInfo stagingBufferInfo = { };
    stagingBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    stagingBufferInfo.size = (VkDeviceSize)a_dataSize;
    stagingBufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

    VmaAllocationCreateInfo stagingBufferAllocInfo = { 0 };
    stagingBufferAllocInfo.usage = VMA_MEMORY_USAGE_AUTO;
    stagingBufferAllocInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;

    VkBuffer stagingBuffer;
    VmaAllocation stagingAllocation;

    VmaAllocationInfo stagingAllocationInfo;
    ICARIAN_ASSERT_MSG_R(vmaCreateBuffer(allocator, &stagingBufferInfo, &stagingBufferAllocInfo, &stagingBuffer, &stagingAllocation, &stagingAllocationInfo) == VK_SUCCESS, "Failed to create staging texture");
    IDEFER(m_engine->PushDeletionObject(new VulkanTextureBufferDeletionObject(m_engine, stagingBuffer, stagingAllocation)));

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

    TLockObj<vk::CommandBuffer, std::mutex>* buffer = m_engine->BeginSingleCommand();
    IDEFER(m_engine->EndSingleCommand(buffer));

    const vk::CommandBuffer cmd = buffer->Get();

    const vk::ImageSubresourceRange subresourceRange = vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, a_levels, 0, 1);

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
    m_engine->PushDeletionObject(new VulkanTextureDeletionObject(m_engine, m_image, m_allocation));
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

    VkBufferCreateInfo stagingBufferInfo = { };
    stagingBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    stagingBufferInfo.size = imageSize;
    stagingBufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

    VmaAllocationCreateInfo stagingBufferAllocInfo = { 0 };
    stagingBufferAllocInfo.usage = VMA_MEMORY_USAGE_AUTO;
    stagingBufferAllocInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;

    VkBuffer stagingBuffer;
    VmaAllocation stagingAllocation;

    VmaAllocationInfo stagingAllocationInfo;
    ICARIAN_ASSERT_MSG_R(vmaCreateBuffer(allocator, &stagingBufferInfo, &stagingBufferAllocInfo, &stagingBuffer, &stagingAllocation, &stagingAllocationInfo) == VK_SUCCESS, "Failed to create staging texture");
    IDEFER(m_engine->PushDeletionObject(new VulkanTextureBufferDeletionObject(m_engine, stagingBuffer, stagingAllocation)));

    if (a_data != nullptr)
    {
        IDEFER(ICARIAN_ASSERT_R(vmaFlushAllocation(allocator, stagingAllocation, 0, (VkDeviceSize)imageSize) == VK_SUCCESS));
        memcpy(stagingAllocationInfo.pMappedData, a_data, (size_t)imageSize);
    }

    TLockObj<vk::CommandBuffer, std::mutex>* buffer = m_engine->BeginSingleCommand();
    IDEFER(m_engine->EndSingleCommand(buffer));

    const vk::CommandBuffer cmd = buffer->Get();

    constexpr vk::ImageSubresourceRange SubresourceRange = vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1);
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