#include "Rendering/Vulkan/VulkanDepthRenderTexture.h"

#include "Flare/IcarianAssert.h"
#include "Rendering/Vulkan/VulkanRenderEngineBackend.h"
#include "Trace.h"

constexpr vk::Format DepthFormats[] =
{
    vk::Format::eD32Sfloat,
    vk::Format::eD32SfloatS8Uint,
    vk::Format::eD24UnormS8Uint,
    vk::Format::eD16Unorm,
    vk::Format::eD16UnormS8Uint
};

static vk::Format GetValidDepthFormat(vk::PhysicalDevice a_device)
{
    for (const vk::Format format : DepthFormats)
    {
        vk::FormatProperties formatProps = a_device.getFormatProperties(format);

        if (formatProps.optimalTilingFeatures & vk::FormatFeatureFlagBits::eDepthStencilAttachment)
        {
            return format;
        }
    }

    ICARIAN_ASSERT_MSG(0, "No valid depth format");

    return vk::Format::eUndefined;
}

static constexpr vk::ImageLayout GetDepthLayout(vk::Format a_format)
{
    // Refer to VulkanRenderTexture::GetDepthLayout
    // if (a_format >= vk::Format::eD16UnormS8Uint)
    {
        return vk::ImageLayout::eDepthStencilAttachmentOptimal;
    }

    return vk::ImageLayout::eDepthAttachmentOptimal;
}

VulkanDepthRenderTexture::VulkanDepthRenderTexture(VulkanRenderEngineBackend* a_engine, uint32_t a_width, uint32_t a_height)
{
    TRACE("Creating VulkanDepthRenderTexture");
    m_engine = a_engine;

    m_width = a_width;
    m_height = a_height;

    const VmaAllocator allocator = m_engine->GetAllocator();
    const vk::Device device = m_engine->GetLogicalDevice();
    const vk::PhysicalDevice physicalDevice = m_engine->GetPhysicalDevice();

    const vk::Format depthFormat = GetValidDepthFormat(physicalDevice);

    TRACE("Creating Attachments");
    const vk::AttachmentDescription depthAttachment = vk::AttachmentDescription
    (
        { },
        depthFormat,
        vk::SampleCountFlagBits::e1,
        vk::AttachmentLoadOp::eClear,
        vk::AttachmentStoreOp::eStore,
        vk::AttachmentLoadOp::eDontCare,
        vk::AttachmentStoreOp::eDontCare,
        vk::ImageLayout::eUndefined,
        vk::ImageLayout::eDepthStencilReadOnlyOptimal
    );
    const vk::AttachmentDescription depthAttachmentNoClear = vk::AttachmentDescription
    (
        { },
        depthFormat,
        vk::SampleCountFlagBits::e1,
        vk::AttachmentLoadOp::eLoad,
        vk::AttachmentStoreOp::eStore,
        vk::AttachmentLoadOp::eDontCare,
        vk::AttachmentStoreOp::eDontCare,
        vk::ImageLayout::eDepthStencilReadOnlyOptimal,
        vk::ImageLayout::eDepthStencilReadOnlyOptimal
    );

    const vk::AttachmentReference depthAttachmentRef = vk::AttachmentReference
    (
        0,
        GetDepthLayout(depthFormat)
    );

    const vk::SubpassDescription subpass = vk::SubpassDescription
    (
        { },
        vk::PipelineBindPoint::eGraphics,
        0,
        nullptr,
        0,
        nullptr,
        nullptr,
        &depthAttachmentRef,
        0,
        nullptr
    );

    constexpr vk::SubpassDependency Dependencies[2] =
    {
        vk::SubpassDependency
        (
            VK_SUBPASS_EXTERNAL,
            0,
            vk::PipelineStageFlagBits::eFragmentShader,
            vk::PipelineStageFlagBits::eEarlyFragmentTests,
            vk::AccessFlagBits::eShaderRead,
            vk::AccessFlagBits::eDepthStencilAttachmentRead | vk::AccessFlagBits::eDepthStencilAttachmentWrite,
            vk::DependencyFlagBits::eByRegion
        ),
        vk::SubpassDependency
        (
            0,
            VK_SUBPASS_EXTERNAL,
            vk::PipelineStageFlagBits::eLateFragmentTests,
            vk::PipelineStageFlagBits::eFragmentShader,
            vk::AccessFlagBits::eDepthStencilAttachmentRead | vk::AccessFlagBits::eDepthStencilAttachmentWrite,
            vk::AccessFlagBits::eShaderRead,
            vk::DependencyFlagBits::eByRegion
        )
    };

    const vk::RenderPassCreateInfo renderPassInfo = vk::RenderPassCreateInfo
    (
        { },
        1,
        &depthAttachment,
        1,
        &subpass,
        2,
        Dependencies
    );
    const vk::RenderPassCreateInfo renderPassInfoNoClear = vk::RenderPassCreateInfo
    (
        { },
        1,
        &depthAttachmentNoClear,
        1,
        &subpass,
        2,
        Dependencies
    );

    ICARIAN_ASSERT_MSG_R(device.createRenderPass(&renderPassInfo, nullptr, &m_renderPass) == vk::Result::eSuccess, "Failed to create depth render texture render pass");
    ICARIAN_ASSERT_MSG_R(device.createRenderPass(&renderPassInfoNoClear, nullptr, &m_renderPassNoClear) == vk::Result::eSuccess, "Failed to create depth render texture render pass");

    Init(m_width, m_height);
}
VulkanDepthRenderTexture::~VulkanDepthRenderTexture()
{
    const vk::Device device = m_engine->GetLogicalDevice();

    Destroy();

    device.destroyRenderPass(m_renderPass);
    device.destroyRenderPass(m_renderPassNoClear);
}

void VulkanDepthRenderTexture::Init(uint32_t a_width, uint32_t a_height)
{
    const vk::Device device = m_engine->GetLogicalDevice();
    const vk::PhysicalDevice physicalDevice = m_engine->GetPhysicalDevice();
    const VmaAllocator allocator = m_engine->GetAllocator();

    m_width = a_width;
    m_height = a_height;

    const vk::Format depthFormat = GetValidDepthFormat(physicalDevice);

    TRACE("Creating Depth Texture");
    VkImageCreateInfo imageInfo = { };
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.format = (VkFormat)depthFormat;
    imageInfo.extent.width = m_width;
    imageInfo.extent.height = m_height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;

    VmaAllocationCreateInfo allocInfo = { 0 };
    allocInfo.usage = VMA_MEMORY_USAGE_AUTO;
    allocInfo.preferredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    allocInfo.flags = 0;

    VkImage image;
    ICARIAN_ASSERT_MSG_R(vmaCreateImage(allocator, &imageInfo, &allocInfo, &image, &m_textureAllocation, nullptr) == VK_SUCCESS, "Failed to create depth texture");
    m_texture = image;

    constexpr vk::ImageSubresourceRange DepthSubresourceRange = vk::ImageSubresourceRange
    (
        vk::ImageAspectFlagBits::eDepth,
        0,
        1,
        0,
        1
    );

    const vk::ImageViewCreateInfo viewInfo = vk::ImageViewCreateInfo
    (
        { },
        m_texture,
        vk::ImageViewType::e2D,
        depthFormat,
        { },
        DepthSubresourceRange
    );
    ICARIAN_ASSERT_MSG_R(device.createImageView(&viewInfo, nullptr, &m_textureView) == vk::Result::eSuccess, "Failed to create depth texture view");

    TRACE("Creating Framebuffer");
    const vk::FramebufferCreateInfo framebufferInfo = vk::FramebufferCreateInfo
    (
        { },
        m_renderPass,
        1,
        &m_textureView,
        m_width,
        m_height,
        1
    );
    ICARIAN_ASSERT_MSG_R(device.createFramebuffer(&framebufferInfo, nullptr, &m_frameBuffer) == vk::Result::eSuccess, "Failed to create depth texture framebuffer");
}
void VulkanDepthRenderTexture::Destroy()
{
    const vk::Device device = m_engine->GetLogicalDevice();
    const VmaAllocator allocator = m_engine->GetAllocator();

    device.waitIdle();

    device.destroyFramebuffer(m_frameBuffer);
    device.destroyImageView(m_textureView);

    vmaDestroyImage(allocator, m_texture, m_textureAllocation);
}