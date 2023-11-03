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

void VulkanTexture::Init(VulkanRenderEngineBackend* a_engine, uint32_t a_width, uint32_t a_height, const void* a_data, vk::Format a_format, uint32_t a_channels)
{
    m_engine = a_engine;

    m_width = a_width;
    m_height = a_height;

    m_format = a_format;

    const vk::DeviceSize imageSize = (vk::DeviceSize)m_width * m_height * a_channels;

    const vk::Device device = m_engine->GetLogicalDevice();
    const VmaAllocator allocator = m_engine->GetAllocator();

    VkBufferCreateInfo stagingBufferInfo = { };
    stagingBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    stagingBufferInfo.size = imageSize;
    stagingBufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

    VmaAllocationCreateInfo stagingBufferAllocInfo = { 0 };
    stagingBufferAllocInfo.usage = VMA_MEMORY_USAGE_AUTO;
    stagingBufferAllocInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;

    struct 
    {
        VkBuffer buffer;
        VmaAllocation allocation;
        VmaAllocator allocator;
    } stagingBufferInfoStruct;
    stagingBufferInfoStruct.allocator = allocator;

    VmaAllocationInfo stagingAllocationInfo;
    ICARIAN_ASSERT_MSG_R(vmaCreateBuffer(allocator, &stagingBufferInfo, &stagingBufferAllocInfo, &stagingBufferInfoStruct.buffer, &stagingBufferInfoStruct.allocation, &stagingAllocationInfo) == VK_SUCCESS, "Failed to create staging texture");

    IDEFER(m_engine->PushDeletionObject(new VulkanTextureBufferDeletionObject(m_engine, stagingBufferInfoStruct.buffer, stagingBufferInfoStruct.allocation)));

    if (a_data != nullptr)
    {
        memcpy(stagingAllocationInfo.pMappedData, a_data, (size_t)imageSize);
        ICARIAN_ASSERT_R(vmaFlushAllocation(allocator, stagingBufferInfoStruct.allocation, 0, imageSize) == VK_SUCCESS);
    }
    
    VkImageCreateInfo imageInfo = { };
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = m_width;
    imageInfo.extent.height = m_height;
    imageInfo.extent.depth = 1;
    // TODO: Add mip levels
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
    allocInfo.usage = VMA_MEMORY_USAGE_AUTO;
    allocInfo.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
    allocInfo.priority = 1.0f;

    VkImage image;
    ICARIAN_ASSERT_MSG_R(vmaCreateImage(allocator, &imageInfo, &allocInfo, &image, &m_allocation, nullptr) == VK_SUCCESS, "Failed to create VulkanTexture image");
    m_image = image;

    TLockObj<vk::CommandBuffer, std::mutex>* buffer = m_engine->BeginSingleCommand();
    const vk::CommandBuffer cmd = buffer->Get();

    constexpr vk::ImageSubresourceRange SubresourceRange = vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1);
    constexpr vk::ImageSubresourceLayers SubresourceLayers = vk::ImageSubresourceLayers(vk::ImageAspectFlagBits::eColor, 0, 0, 1);

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

    const vk::BufferImageCopy copyRegion = vk::BufferImageCopy(0, 0, 0, SubresourceLayers, { 0, 0, 0 }, { m_width, m_height, 1 });

    cmd.copyBufferToImage(stagingBufferInfoStruct.buffer, m_image, vk::ImageLayout::eTransferDstOptimal, 1, &copyRegion);

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

    m_engine->EndSingleCommand(buffer);
}
VulkanTexture::VulkanTexture()
{

}
VulkanTexture::~VulkanTexture()
{
    TRACE("Queueing Texture Deletion");
    m_engine->PushDeletionObject(new VulkanTextureDeletionObject(m_engine, m_image, m_allocation));
}

VulkanTexture* VulkanTexture::CreateRGBA(VulkanRenderEngineBackend* a_engine, uint32_t a_width, uint32_t a_height, const void* a_data)
{
    TRACE("Creating Texture");

    VulkanTexture* texture = new VulkanTexture();
    texture->Init(a_engine, a_width, a_height, a_data, vk::Format::eR8G8B8A8Srgb, 4);

    return texture;
}
VulkanTexture* VulkanTexture::CreateAlpha(VulkanRenderEngineBackend* a_engine, uint32_t a_width, uint32_t a_height, const void *a_data)
{
    TRACE("Creating Alpha Texture");

    VulkanTexture* texture = new VulkanTexture();
    texture->Init(a_engine, a_width, a_height, a_data, vk::Format::eR8Unorm, 1);

    return texture;
}
#endif