#include "Rendering/Vulkan/VulkanTexture.h"

#include "Flare/IcarianAssert.h"
#include "Rendering/Vulkan/VulkanRenderEngineBackend.h"
#include "Trace.h"

VulkanTexture::VulkanTexture(VulkanRenderEngineBackend* a_engine, uint32_t a_width, uint32_t a_height, const void* a_data)
{
    TRACE("Creating Texture");
    m_engine = a_engine;

    m_width = a_width;
    m_height = a_height;

    const vk::DeviceSize imageSize = (vk::DeviceSize)m_width * m_height * 4;

    const vk::Device device = m_engine->GetLogicalDevice();
    const VmaAllocator allocator = m_engine->GetAllocator();

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

    if (a_data != nullptr)
    {
        memcpy(stagingAllocationInfo.pMappedData, a_data, (size_t)imageSize);
        ICARIAN_ASSERT_R(vmaFlushAllocation(allocator, stagingAllocation, 0, imageSize) == VK_SUCCESS);
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
    imageInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
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

    vk::CommandBuffer cmd = m_engine->BeginSingleCommand();

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

    m_engine->EndSingleCommand(cmd);

    vmaDestroyBuffer(allocator, stagingBuffer, stagingAllocation);

    const vk::ImageViewCreateInfo viewInfo = vk::ImageViewCreateInfo
    (
        { }, 
        m_image, 
        vk::ImageViewType::e2D, 
        vk::Format::eR8G8B8A8Srgb,
        { vk::ComponentSwizzle::eIdentity, vk::ComponentSwizzle::eIdentity, vk::ComponentSwizzle::eIdentity, vk::ComponentSwizzle::eIdentity },
        SubresourceRange
    );

    ICARIAN_ASSERT_MSG_R(device.createImageView(&viewInfo, nullptr, &m_view) == vk::Result::eSuccess, "Failed to create VulkanTexture View");
}
VulkanTexture::~VulkanTexture()
{
    const vk::Device device = m_engine->GetLogicalDevice();
    const VmaAllocator allocator = m_engine->GetAllocator();

    TRACE("Destroying Texture");

    device.destroyImageView(m_view);
    vmaDestroyImage(allocator, m_image, m_allocation);
}