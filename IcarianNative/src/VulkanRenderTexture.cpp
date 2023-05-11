#include "Rendering/Vulkan/VulkanRenderTexture.h"

#include "Flare/IcarianAssert.h"
#include "Logger.h"
#include "Rendering/Vulkan/VulkanRenderEngineBackend.h"
#include "Trace.h"

const static std::vector<vk::Format> DepthFormats = 
{
    vk::Format::eD32Sfloat,
    vk::Format::eD32SfloatS8Uint,
    vk::Format::eD24UnormS8Uint
};

static constexpr vk::Format GetFormat(bool a_hdr)
{
    if (a_hdr)
    {
        return vk::Format::eR16G16B16A16Sfloat;
    }

    return vk::Format::eR8G8B8A8Unorm;
}

static vk::Format GetValidDepthFormat(vk::PhysicalDevice a_device)
{
    for (const vk::Format format : DepthFormats)
    {
        vk::FormatProperties properties;
        a_device.getFormatProperties(format, &properties);

        if (properties.optimalTilingFeatures & vk::FormatFeatureFlagBits::eDepthStencilAttachment)
        {
            return format;
        }
    }

    Logger::Error("IcarianEngine: No valid depth format");

    assert(0);

    return vk::Format::eD16Unorm;
}

static constexpr vk::ImageLayout GetDepthLayout(vk::Format a_format)
{
    // TODO: Solve why not matter what configuration why depth stencil is the only valid value.
    // Have tried samples and got the same validation error.
    // Not a major issue however. Probably just missed a flag somewhere in setup.
    // if (a_format >= vk::Format::eD16UnormS8Uint)
    {
        return vk::ImageLayout::eDepthStencilAttachmentOptimal;
    }

    return vk::ImageLayout::eDepthAttachmentOptimal;
}

VulkanRenderTexture::VulkanRenderTexture(VulkanRenderEngineBackend* a_engine, uint32_t a_textureCount, uint32_t a_width, uint32_t a_height, bool a_depthTexture, bool a_hdr)
{
    TRACE("Creating Render Texture");
    m_engine = a_engine;

    m_textureCount = a_textureCount;

    m_width = a_width;
    m_height = a_height;

    m_flags = 0;
    if (a_hdr)
    {
        m_flags |= 0b1 << HDRFlag;
    }
    if (a_depthTexture)
    {
        m_flags |= 0b1 << DepthTextureFlag;
    }

    const uint32_t totalTextureCount = GetTotalTextureCount();

    const VmaAllocator allocator = m_engine->GetAllocator();
    const vk::Device device = m_engine->GetLogicalDevice();
    const vk::PhysicalDevice physicalDevice = m_engine->GetPhysicalDevice();

    const vk::Format format = GetFormat(a_hdr);
    const vk::Format depthFormat = GetValidDepthFormat(physicalDevice);

    TRACE("Creating Attachments");
    std::vector<vk::AttachmentDescription> attachments = std::vector<vk::AttachmentDescription>(totalTextureCount);
    for (uint32_t i = 0; i < m_textureCount; ++i)
    {
        attachments[i].format = format;
        attachments[i].samples = vk::SampleCountFlagBits::e1;
        attachments[i].loadOp = vk::AttachmentLoadOp::eClear;
        attachments[i].storeOp = vk::AttachmentStoreOp::eStore;
        attachments[i].stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
        attachments[i].stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
        attachments[i].initialLayout = vk::ImageLayout::eUndefined;
        attachments[i].finalLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
    }
    if (a_depthTexture)
    {
        attachments[m_textureCount].format = depthFormat;
        attachments[m_textureCount].samples = vk::SampleCountFlagBits::e1;
        attachments[m_textureCount].loadOp = vk::AttachmentLoadOp::eClear;
        attachments[m_textureCount].storeOp = vk::AttachmentStoreOp::eStore;
        attachments[m_textureCount].stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
        attachments[m_textureCount].stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
        attachments[m_textureCount].initialLayout = vk::ImageLayout::eUndefined;
        attachments[m_textureCount].finalLayout = vk::ImageLayout::eDepthStencilReadOnlyOptimal;
    }

    std::vector<vk::AttachmentReference> colorAttachmentRef = std::vector<vk::AttachmentReference>(m_textureCount);
    for (uint32_t i = 0; i < m_textureCount; ++i)
    {
        colorAttachmentRef[i].attachment = i;
        colorAttachmentRef[i].layout = vk::ImageLayout::eColorAttachmentOptimal;
    }
    const vk::AttachmentReference depthAttachmentRef = vk::AttachmentReference
    (
        m_textureCount,
        GetDepthLayout(depthFormat)
    );

    vk::SubpassDescription subpass = vk::SubpassDescription
    (
        { },
        vk::PipelineBindPoint::eGraphics,
        0,
        nullptr,
        m_textureCount,
        colorAttachmentRef.data()
    );
    if (a_depthTexture)
    {
        subpass.pDepthStencilAttachment = &depthAttachmentRef;
    }

    vk::SubpassDependency dependencies[2];
    dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[0].dstSubpass = 0;
    dependencies[0].srcStageMask = vk::PipelineStageFlagBits::eFragmentShader;
    dependencies[0].dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
    dependencies[0].srcAccessMask = vk::AccessFlagBits::eShaderRead;
    dependencies[0].dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
    dependencies[0].dependencyFlags = vk::DependencyFlagBits::eByRegion;
    dependencies[1].srcSubpass = 0;
    dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[1].srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
    dependencies[1].dstStageMask = vk::PipelineStageFlagBits::eFragmentShader;
    dependencies[1].srcAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
    dependencies[1].dstAccessMask = vk::AccessFlagBits::eShaderRead;
    dependencies[1].dependencyFlags = vk::DependencyFlagBits::eByRegion;
    if (a_depthTexture)
    {
        dependencies[0].dstStageMask |= vk::PipelineStageFlagBits::eEarlyFragmentTests;
        dependencies[0].dstAccessMask |= vk::AccessFlagBits::eDepthStencilAttachmentRead | vk::AccessFlagBits::eDepthStencilAttachmentWrite;
        dependencies[1].srcStageMask |= vk::PipelineStageFlagBits::eLateFragmentTests;
        dependencies[1].srcAccessMask |= vk::AccessFlagBits::eDepthStencilAttachmentRead | vk::AccessFlagBits::eDepthStencilAttachmentWrite;
    }

    const vk::RenderPassCreateInfo renderPassInfo = vk::RenderPassCreateInfo
    (
        { },
        totalTextureCount,
        attachments.data(),
        1,
        &subpass,
        2,
        dependencies
    );

    if (device.createRenderPass(&renderPassInfo, nullptr, &m_renderPass) != vk::Result::eSuccess)
    {
        Logger::Error("Failed to create render texture render pass");

        assert(0);
    }    

    m_textures = new vk::Image[totalTextureCount];
    m_textureViews = new vk::ImageView[totalTextureCount];
    m_textureAllocations = new VmaAllocation[totalTextureCount];
    m_clearValues = new vk::ClearValue[totalTextureCount];
    for (uint32_t i = 0; i < m_textureCount; ++i)
    {
        m_clearValues[i] = vk::ClearValue({ 0.0f, 0.0f, 0.0f, 0.0f });
    }
    if (a_depthTexture)
    {
        m_clearValues[m_textureCount] = vk::ClearValue(vk::ClearDepthStencilValue(1.0f, 0));
    }

    Init(m_width, m_height);
}
VulkanRenderTexture::~VulkanRenderTexture()
{
    TRACE("Destroying Render Texture");
    
    const vk::Device device = m_engine->GetLogicalDevice();

    Destroy();

    device.destroyRenderPass(m_renderPass);

    delete[] m_textures;
    delete[] m_textureViews;
    delete[] m_textureAllocations;
    delete[] m_clearValues;
}

void VulkanRenderTexture::Init(uint32_t a_width, uint32_t a_height)
{
    const vk::Device device = m_engine->GetLogicalDevice();
    const vk::PhysicalDevice physicalDevice = m_engine->GetPhysicalDevice();
    const VmaAllocator allocator = m_engine->GetAllocator();

    const bool hasDepth = HasDepthTexture();
    const bool isHDR = IsHDR();

    const vk::Format format = GetFormat(isHDR);
    const vk::Format depthFormat = GetValidDepthFormat(physicalDevice);

    const uint32_t totalTextureCount = GetTotalTextureCount();

    m_width = a_width;
    m_height = a_height;

    TRACE("Creating Textures");
    VkImageCreateInfo textureCreateInfo = { };
    textureCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    textureCreateInfo.imageType = VK_IMAGE_TYPE_2D;
    textureCreateInfo.format = (VkFormat)format;
    textureCreateInfo.extent.width = m_width;
    textureCreateInfo.extent.height = m_height;
    textureCreateInfo.extent.depth = 1;
    textureCreateInfo.mipLevels = 1;
    textureCreateInfo.arrayLayers = 1;
    textureCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    textureCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    textureCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    textureCreateInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;

    VmaAllocationCreateInfo allocInfo = { 0 };
    allocInfo.usage = VMA_MEMORY_USAGE_AUTO;
    allocInfo.preferredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    allocInfo.flags = 0;

    constexpr vk::ImageSubresourceRange SubresourceRange = vk::ImageSubresourceRange
    (
        vk::ImageAspectFlagBits::eColor,
        0,
        1,
        0,
        1
    );

    for (uint32_t i = 0; i < m_textureCount; ++i)
    {
        VkImage image;
        if (vmaCreateImage(allocator, &textureCreateInfo, &allocInfo, &image, &m_textureAllocations[i], nullptr) != VK_SUCCESS)
        {
            Logger::Error("Failed to create RenderTexture Image");

            assert(0);
        }
        m_textures[i] = image;

        const vk::ImageViewCreateInfo textureImageView = vk::ImageViewCreateInfo
        (
            { },
            m_textures[i],
            vk::ImageViewType::e2D,
            format,
            vk::ComponentMapping(),
            SubresourceRange
        );

        if (device.createImageView(&textureImageView, nullptr, &m_textureViews[i]) != vk::Result::eSuccess)
        {
            Logger::Error("Failed to create RenderTexture Image View");

            assert(0);
        }
    }
    if (hasDepth)
    {
        constexpr vk::ImageSubresourceRange DepthSubresouceRange = vk::ImageSubresourceRange
        (
            vk::ImageAspectFlagBits::eDepth,
            0,
            1,
            0,
            1
        );

        VkImageCreateInfo depthCreateInfo = { };
        depthCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        depthCreateInfo.imageType = VK_IMAGE_TYPE_2D;
        depthCreateInfo.format = (VkFormat)depthFormat;
        depthCreateInfo.extent.width = m_width;
        depthCreateInfo.extent.height = m_height;
        depthCreateInfo.extent.depth = 1;
        depthCreateInfo.mipLevels = 1;
        depthCreateInfo.arrayLayers = 1;
        depthCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        depthCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        depthCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        depthCreateInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;

        VkImage image;
        if (vmaCreateImage(allocator, &depthCreateInfo, &allocInfo, &image, &m_textureAllocations[m_textureCount], nullptr) != VK_SUCCESS)
        {
            Logger::Error("Failed to create RenderTexture Depth Image");

            assert(0);
        }
        m_textures[m_textureCount] = image;

        const vk::ImageViewCreateInfo depthImageView = vk::ImageViewCreateInfo
        (
            { },
            m_textures[m_textureCount],
            vk::ImageViewType::e2D,
            depthFormat,
            vk::ComponentMapping(),
            DepthSubresouceRange
        );

        if (device.createImageView(&depthImageView, nullptr, &m_textureViews[m_textureCount]) != vk::Result::eSuccess)
        {
            Logger::Error("Failed to create RenderTexture depth Image View");

            assert(0);
        }
    }

    TRACE("Creating Frame Buffer");
    const vk::FramebufferCreateInfo fbCreateInfo = vk::FramebufferCreateInfo
    (
        { },
        m_renderPass,
        totalTextureCount,
        m_textureViews,
        m_width, 
        m_height,
        1
    );

    ICARIAN_ASSERT_R(device.createFramebuffer(&fbCreateInfo, nullptr, &m_frameBuffer) == vk::Result::eSuccess);
}
void VulkanRenderTexture::Destroy()
{
    const vk::Device device = m_engine->GetLogicalDevice();
    const VmaAllocator allocator = m_engine->GetAllocator();

    device.waitIdle();

    const uint32_t totalTextureCount = GetTotalTextureCount();

    TRACE("Destroying Render Texture Textures");
    for (uint32_t i = 0; i < totalTextureCount; ++i)
    {
        vmaDestroyImage(allocator, m_textures[i], m_textureAllocations[i]);
        device.destroyImageView(m_textureViews[i]);
    }

    TRACE("Destroying Framebuffer");
    device.destroyFramebuffer(m_frameBuffer);
}

void VulkanRenderTexture::Resize(uint32_t a_width, uint32_t a_height)
{
    TRACE("Resizing Render Texture");
    Destroy();

    Init(a_width, a_height);
}