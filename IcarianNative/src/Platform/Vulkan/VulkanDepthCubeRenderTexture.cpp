// Icarian Engine - C# Game Engine
// 
// License at end of file.

#ifdef ICARIANNATIVE_ENABLE_GRAPHICS_VULKAN

#include "Rendering/Vulkan/VulkanDepthCubeRenderTexture.h"

#include "Core/IcarianAssert.h"
#include "Core/IcarianDefer.h"
#include "Rendering/Vulkan/VulkanRenderEngineBackend.h"
#include "Trace.h"

class VulkanDepthCubeRenderTextureTextureDeletionObject : public VulkanDeletionObject
{
private:
    VulkanRenderEngineBackend* m_engine;

    vk::Image                  m_image;
    vk::ImageView              m_imageView;
    vk::ImageView              m_imageViewFrameBuffer[6];
    vk::Framebuffer            m_frameBuffer[6];
    VmaAllocation              m_allocation;

protected:

public:
    VulkanDepthCubeRenderTextureTextureDeletionObject(VulkanRenderEngineBackend* a_engine, vk::Image a_image, vk::ImageView a_imageView, vk::ImageView a_imageViewFrameBuffer[6], vk::Framebuffer a_frameBuffer[6], VmaAllocation a_allocation)
    {
        m_engine = a_engine;

        m_image = a_image;
        m_imageView = a_imageView;
        m_allocation = a_allocation;

        for (int i = 0; i < 6; ++i)
        {
            m_imageViewFrameBuffer[i] = a_imageViewFrameBuffer[i];
            m_frameBuffer[i] = a_frameBuffer[i];
        }
    }
    virtual ~VulkanDepthCubeRenderTextureTextureDeletionObject()
    {

    }

    virtual void Destroy()
    {
        TRACE("Destroying Depth Cube Texture Render Texture");
        const vk::Device device = m_engine->GetLogicalDevice();
        const VmaAllocator allocator = m_engine->GetAllocator();

        for (int i = 0; i < 6; ++i)
        {
            device.destroyFramebuffer(m_frameBuffer[i]);
            device.destroyImageView(m_imageViewFrameBuffer[i]);
        }
        device.destroyImageView(m_imageView);
        
        vmaDestroyImage(allocator, m_image, m_allocation);
    }
};

class VulkanDepthCubeRenderTextureDeletionObject : public VulkanDeletionObject
{
private:
    VulkanRenderEngineBackend* m_engine;

    vk::RenderPass             m_renderPass;
    vk::RenderPass             m_renderPassNoClear;

protected:

public:
    VulkanDepthCubeRenderTextureDeletionObject(VulkanRenderEngineBackend* a_engine, vk::RenderPass a_renderPass, vk::RenderPass a_renderPassNoClear)
    {
        m_engine = a_engine;

        m_renderPass = a_renderPass;
        m_renderPassNoClear = a_renderPassNoClear;
    }
    virtual ~VulkanDepthCubeRenderTextureDeletionObject()
    {

    }

    virtual void Destroy()
    {
        TRACE("Destroying Depth Cube Texture Render Texture");
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

VulkanDepthCubeRenderTexture::VulkanDepthCubeRenderTexture(VulkanRenderEngineBackend* a_engine, uint32_t a_width, uint32_t a_height)
{
    TRACE("Creating VulkanDepthCubeRenderTexture");
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

    ICARIAN_ASSERT_MSG_R(device.createRenderPass(&renderPassInfo, nullptr, &m_renderPass) == vk::Result::eSuccess, "Failed to create render pass");
    ICARIAN_ASSERT_MSG_R(device.createRenderPass(&renderPassInfoNoClear, nullptr, &m_renderPassNoClear) == vk::Result::eSuccess, "Failed to create render pass");

    Init(m_width, m_height);
}
VulkanDepthCubeRenderTexture::~VulkanDepthCubeRenderTexture()
{
    TRACE("Queueing Depth Cube Render Texture Deletion");
    m_engine->PushDeletionObject(new VulkanDepthCubeRenderTextureTextureDeletionObject(m_engine, m_texture, m_textureView, m_textureViewFramebuffer, m_frameBuffer, m_textureAllocation));

    m_engine->PushDeletionObject(new VulkanDepthCubeRenderTextureDeletionObject(m_engine, m_renderPass, m_renderPassNoClear));
}

void VulkanDepthCubeRenderTexture::Init(uint32_t a_width, uint32_t a_height)
{
    const VmaAllocator allocator = m_engine->GetAllocator();
    const vk::Device device = m_engine->GetLogicalDevice();
    const vk::PhysicalDevice physicalDevice = m_engine->GetPhysicalDevice();

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
    imageInfo.arrayLayers = 6;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    // After driver update crash and validation error for this missing
    imageInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;

    VmaAllocationCreateInfo imageAllocInfo = { };
    imageAllocInfo.usage = VMA_MEMORY_USAGE_AUTO;
    imageAllocInfo.preferredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    imageAllocInfo.flags = 0;

    VkImage image;
    ICARIAN_ASSERT_MSG_R(vmaCreateImage(allocator, &imageInfo, &imageAllocInfo, &image, &m_textureAllocation, nullptr) == VK_SUCCESS, "Failed to create depth texture");
    m_texture = image;

    constexpr vk::ImageSubresourceRange DepthSubresourceRange = vk::ImageSubresourceRange
    (
        vk::ImageAspectFlagBits::eDepth,
        0,
        1,
        0,
        6
    );

    const vk::ImageViewCreateInfo imageViewInfo = vk::ImageViewCreateInfo
    (
        { },
        m_texture,
        vk::ImageViewType::eCube,
        depthFormat,
        { },
        DepthSubresourceRange
    );
    ICARIAN_ASSERT_MSG_R(device.createImageView(&imageViewInfo, nullptr, &m_textureView) == vk::Result::eSuccess, "Failed to create depth texture view");

    TRACE("Creating Frame Buffer");
    for (int i = 0; i < 6; ++i)
    {
        const vk::ImageSubresourceRange depthSubresourceRangeFramebuffer = vk::ImageSubresourceRange
        (
            vk::ImageAspectFlagBits::eDepth,
            0,
            1,
            i,
            1
        );

        const vk::ImageViewCreateInfo imageViewInfoFramebuffer = vk::ImageViewCreateInfo
        (
            { },
            m_texture,
            vk::ImageViewType::e2D,
            depthFormat,
            { },
            depthSubresourceRangeFramebuffer
        );

        ICARIAN_ASSERT_MSG_R(device.createImageView(&imageViewInfoFramebuffer, nullptr, &m_textureViewFramebuffer[i]) == vk::Result::eSuccess, "Failed to create depth texture view");

        const vk::FramebufferCreateInfo frameBufferInfo = vk::FramebufferCreateInfo
        (
            { },
            m_renderPass,
            1,
            &m_textureViewFramebuffer[i],
            m_width,
            m_height,
            1
        );

        ICARIAN_ASSERT_MSG_R(device.createFramebuffer(&frameBufferInfo, nullptr, &m_frameBuffer[i]) == vk::Result::eSuccess, "Failed to create frame buffer");
    }

    // Due to optimization this is needed as all faces may not be wrote to by point lights triggering the transition
    // Uninitialized memory can be a problem but not on screen for point light so does not matter
    // This causes assertion to trigger after driver update if missing
    // Was technically undefined behaviour so probably should not have ignored but was not an issue up until now
    // My bad
    // Should only be necessary for cube maps due to optimization normal render textures should be fine
    // And yes that optimization is needed as in some scenes the difference between 20 and 120 fps 
    TLockObj<vk::CommandBuffer, SpinLock>* l = m_engine->BeginSingleCommand();
    IDEFER(m_engine->EndSingleCommand(l));

    vk::CommandBuffer commandBuffer = l->Get();

    const vk::ImageMemoryBarrier memoryBarrier = vk::ImageMemoryBarrier
    (
        vk::AccessFlags(),
        vk::AccessFlagBits::eShaderRead,
        vk::ImageLayout::eUndefined,
        vk::ImageLayout::eDepthStencilReadOnlyOptimal,
        VK_QUEUE_FAMILY_IGNORED,
        VK_QUEUE_FAMILY_IGNORED,
        m_texture,
        DepthSubresourceRange
    );

    commandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eFragmentShader, vk::PipelineStageFlagBits::eFragmentShader, vk::DependencyFlags(), 0, nullptr, 0, nullptr, 1, &memoryBarrier);
}

void VulkanDepthCubeRenderTexture::Resize(uint32_t a_width, uint32_t a_height)
{
    m_engine->PushDeletionObject(new VulkanDepthCubeRenderTextureTextureDeletionObject(m_engine, m_texture, m_textureView, m_textureViewFramebuffer, m_frameBuffer, m_textureAllocation));

    Init(a_width, a_height);
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