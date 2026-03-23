// Icarian Engine - C# Game Engine
// 
// License at end of file.

#ifdef ICARIANNATIVE_ENABLE_GRAPHICS_VULKAN

#include "Rendering/Vulkan/VulkanDepthRenderTexture.h"

#include "Core/IcarianAssert.h"
#include "Rendering/Vulkan/VulkanRenderEngineBackend.h"
#include "Trace.h"

class VulkanDepthTextureTextureDeletionObject : public VulkanDeletionObject
{
private:
    VulkanRenderEngineBackend* m_engine;

    vk::Image                  m_image;
    vk::ImageView              m_imageView;
    vk::Framebuffer            m_frameBuffer;
    VmaAllocation              m_allocation;

protected:

public:
    VulkanDepthTextureTextureDeletionObject(VulkanRenderEngineBackend* a_engine, vk::Image a_image, vk::ImageView a_imageView, VmaAllocation a_allocation, vk::Framebuffer a_frameBuffer)
    {
        m_engine = a_engine;

        m_image = a_image;
        m_imageView = a_imageView;
        m_frameBuffer = a_frameBuffer;
        m_allocation = a_allocation;
    }
    virtual ~VulkanDepthTextureTextureDeletionObject()
    {

    }

    virtual void Destroy()
    {
        TRACE("Destroying Depth Render Texture");
        const vk::Device device = m_engine->GetLogicalDevice();
        const VmaAllocator allocator = m_engine->GetVMAAllocator();

        device.destroyFramebuffer(m_frameBuffer);
        device.destroyImageView(m_imageView);

        vmaDestroyImage(allocator, m_image, m_allocation);
    }
};

class VulkanDepthTextureDeletionObject : public VulkanDeletionObject
{
private:
    VulkanRenderEngineBackend* m_engine;

    vk::RenderPass             m_renderPass;
    vk::RenderPass             m_renderPassNoClear;

protected:

public:
    VulkanDepthTextureDeletionObject(VulkanRenderEngineBackend* a_engine, vk::RenderPass a_renderPass, vk::RenderPass a_renderPassNoClear)
    {
        m_engine = a_engine;

        m_renderPass = a_renderPass;
        m_renderPassNoClear = a_renderPassNoClear;
    }
    virtual ~VulkanDepthTextureDeletionObject()
    {

    }

    virtual void Destroy()
    {
        TRACE("Destroying Depth Render Texture Render Passes");
        const vk::Device device = m_engine->GetLogicalDevice();

        device.destroyRenderPass(m_renderPass);
        device.destroyRenderPass(m_renderPassNoClear);
    }
};

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

    constexpr vk::SubpassDependency Dependencies[] =
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
    constexpr uint32_t DependencyCount = sizeof(Dependencies) / sizeof(*Dependencies);

    const vk::RenderPassCreateInfo renderPassInfo = vk::RenderPassCreateInfo
    (
        { },
        1,
        &depthAttachment,
        1,
        &subpass,
        DependencyCount,
        Dependencies
    );
    const vk::RenderPassCreateInfo renderPassInfoNoClear = vk::RenderPassCreateInfo
    (
        { },
        1,
        &depthAttachmentNoClear,
        1,
        &subpass,
        DependencyCount,
        Dependencies
    );

    VKRESERRMSG(device.createRenderPass(&renderPassInfo, nullptr, &m_renderPass), "Failed to create depth render texture render pass");
    VKRESERRMSG(device.createRenderPass(&renderPassInfoNoClear, nullptr, &m_renderPassNoClear), "Failed to create depth render texture render pass");

    Init(m_width, m_height);
}
VulkanDepthRenderTexture::~VulkanDepthRenderTexture()
{
    TRACE("Queueing Depth Render Texture for deletion");
    m_engine->PushDeletionObject<VulkanDepthTextureTextureDeletionObject>(m_engine, m_texture, m_textureView, m_textureAllocation, m_frameBuffer);
    m_engine->PushDeletionObject<VulkanDepthTextureDeletionObject>(m_engine, m_renderPass, m_renderPassNoClear);
}

void VulkanDepthRenderTexture::Init(uint32_t a_width, uint32_t a_height)
{
    TRACE("Creating Depth Render Texture");

    const vk::Device device = m_engine->GetLogicalDevice();
    const vk::PhysicalDevice physicalDevice = m_engine->GetPhysicalDevice();
    const VmaAllocator allocator = m_engine->GetVMAAllocator();

    m_width = a_width;
    m_height = a_height;

    const vk::Format depthFormat = GetValidDepthFormat(physicalDevice);

    const vk::Extent3D extents = vk::Extent3D(m_width, m_height, 1);

    const VkImageCreateInfo imageInfo = 
    { 
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .imageType = VK_IMAGE_TYPE_2D,
        .format = (VkFormat)depthFormat,
        .extent = extents,
        .mipLevels = 1,
        .arrayLayers = 1,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .tiling = VK_IMAGE_TILING_OPTIMAL,
        .usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
    };

    const VmaAllocationCreateInfo allocInfo = 
    {
        .usage = VMA_MEMORY_USAGE_AUTO,
        .preferredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
    };

    VkImage image;
    VKRESERRMSG(vmaCreateImage(allocator, &imageInfo, &allocInfo, &image, &m_textureAllocation, nullptr), "Failed to create depth texture");
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
    VKRESERRMSG(device.createImageView(&viewInfo, nullptr, &m_textureView), "Failed to create depth texture view");

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
    VKRESERRMSG(device.createFramebuffer(&framebufferInfo, nullptr, &m_frameBuffer), "Failed to create depth texture framebuffer");

    TLockObj<vk::CommandBuffer, SpinLock>* l = m_engine->BeginSingleCommand();
    IDEFER(m_engine->EndSingleCommand(l));

    const vk::CommandBuffer commandBuffer = l->Get();

    const vk::ImageMemoryBarrier memoryBarrier = vk::ImageMemoryBarrier
    (
        vk::AccessFlags(),
        vk::AccessFlagBits::eShaderRead,
        vk::ImageLayout::eUndefined,
        vk::ImageLayout::eDepthStencilReadOnlyOptimal,
        vk::QueueFamilyIgnored,
        vk::QueueFamilyIgnored,
        m_texture,
        DepthSubresourceRange
    );

    commandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eFragmentShader, vk::PipelineStageFlagBits::eFragmentShader, vk::DependencyFlags(), 0, nullptr, 0, nullptr, 1, &memoryBarrier);
}

void VulkanDepthRenderTexture::Resize(uint32_t a_width, uint32_t a_height)
{
    m_engine->PushDeletionObject<VulkanDepthTextureTextureDeletionObject>(m_engine, m_texture, m_textureView, m_textureAllocation, m_frameBuffer);

    Init(a_width, a_height);
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