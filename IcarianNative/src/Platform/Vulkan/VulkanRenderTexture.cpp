// Icarian Engine - C# Game Engine
// 
// License at end of file.

#ifdef ICARIANNATIVE_ENABLE_GRAPHICS_VULKAN

#include "Rendering/Vulkan/VulkanRenderTexture.h"

#include "Core/IcarianLambda.h"
#include "Rendering/Vulkan/VulkanDepthRenderTexture.h"
#include "Rendering/Vulkan/VulkanGraphicsEngine.h"
#include "Rendering/Vulkan/VulkanRenderEngineBackend.h"
#include "Trace.h"

class VulkanRenderTextureDeletionObject : public VulkanDeletionObject
{
private:
    VulkanRenderEngineBackend* m_engine;

    vk::Image*                 m_images;
    vk::ImageView*             m_views;
    VmaAllocation*             m_allocations;

    vk::Framebuffer            m_framebuffer;

    uint32_t                   m_textureCount;

protected:

public:
    VulkanRenderTextureDeletionObject(VulkanRenderEngineBackend* a_engine, uint32_t a_textureCount, const vk::Image* a_images, const vk::ImageView* a_views, const VmaAllocation* a_allocations, vk::Framebuffer a_framebuffer)
    {
        m_engine = a_engine;

        m_textureCount = a_textureCount;

        BlockAllocator* allocator = m_engine->GetDeletionAllocator();

        m_images = allocator->TAllocate<vk::Image>(m_textureCount);
        m_views = allocator->TAllocate<vk::ImageView>(m_textureCount);
        m_allocations = allocator->TAllocate<VmaAllocation>(m_textureCount);

        for (uint32_t i = 0; i < m_textureCount; ++i)
        {
            m_images[i] = a_images[i];
            m_views[i] = a_views[i];
            m_allocations[i] = a_allocations[i];
        }

        m_framebuffer = a_framebuffer;
    }
    virtual ~VulkanRenderTextureDeletionObject()
    {
        BlockAllocator* allocator = m_engine->GetDeletionAllocator();

        allocator->Free(m_images);
        allocator->Free(m_views);
        allocator->Free(m_allocations);
    }

    virtual void Destroy()
    {
        TRACE("Destroying Render Texture Textures");
        const vk::Device device = m_engine->GetLogicalDevice();
        const VmaAllocator allocator = m_engine->GetAllocator();

        for (uint32_t i = 0; i < m_textureCount; ++i)
        {
            vmaDestroyImage(allocator, m_images[i], m_allocations[i]);
            device.destroyImageView(m_views[i]);
        }

        device.destroyFramebuffer(m_framebuffer);
    }
};

class VulkanRenderTextureRenderPassDeletionObject : public VulkanDeletionObject
{
private:
    VulkanRenderEngineBackend* m_engine;

    vk::RenderPass             m_renderPass;
    vk::RenderPass             m_renderPassColorClear;
    vk::RenderPass             m_renderPassNoClear;

protected:

public:
    VulkanRenderTextureRenderPassDeletionObject(VulkanRenderEngineBackend* a_engine, vk::RenderPass a_renderPass, vk::RenderPass a_renderPassColorClear, vk::RenderPass a_renderPassNoClear)
    {
        m_engine = a_engine;

        m_renderPass = a_renderPass;
        m_renderPassColorClear = a_renderPassColorClear;
        m_renderPassNoClear = a_renderPassNoClear;
    }
    virtual ~VulkanRenderTextureRenderPassDeletionObject()
    {

    }

    virtual void Destroy()
    {
        TRACE("Destroying Render Texture Render Pass");
        const vk::Device device = m_engine->GetLogicalDevice();

        device.destroyRenderPass(m_renderPass);
        device.destroyRenderPass(m_renderPassColorClear);
        device.destroyRenderPass(m_renderPassNoClear);
    }
};

constexpr vk::Format DepthFormats[] = 
{
    vk::Format::eD32Sfloat,
    vk::Format::eD32SfloatS8Uint,
    vk::Format::eD24UnormS8Uint
};

static constexpr vk::Format GetFormat(bool a_hdr, uint32_t a_channelCount)
{
    switch (a_channelCount) 
    {
    case 1:
    {
        if (a_hdr)
        {
            return vk::Format::eR16Sfloat;
        }

        return vk::Format::eR8Unorm;
    }
    case 2:
    {
        if (a_hdr)
        {
            return vk::Format::eR16G16Sfloat;
        }

        return vk::Format::eR8G8Unorm;
    }
    case 3:
    {
        if (a_hdr)
        {
            return vk::Format::eR16G16B16Sfloat;
        }

        return vk::Format::eR8G8B8Unorm;
    }
    case 4:
    {
        if (a_hdr)
        {
            return vk::Format::eR16G16B16A16Sfloat;
        }

        return vk::Format::eR8G8B8A8Unorm;
    }
    }

    IERROR("No valid texture format");

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

    IERROR("No valid depth format");

    return vk::Format::eUndefined;
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

void VulkanRenderTexture::Setup()
{
    RENDERSCRATCHFRAME;

    TRACE("Creating Render Texture");
    const vk::Device device = m_engine->GetLogicalDevice();
    const vk::PhysicalDevice physicalDevice = m_engine->GetPhysicalDevice();

    BlockAllocator* blockAllocator = m_engine->GetBlockAllocator();

    const bool hdr = IsHDR();
    const bool hasDepth = HasDepthTexture();

    const uint32_t totalTextureCount = GetTotalTextureCount();

    const vk::Format format = GetFormat(hdr, m_channelCount);
    const vk::Format depthFormat = GetValidDepthFormat(physicalDevice);

    TRACE("Creating Attachments");
    const vk::AttachmentDescription* attachments = ILAMBDA(
    {
        vk::AttachmentDescription* vals = RenderScratchAlloc::TAllocate<vk::AttachmentDescription>(totalTextureCount);

        const vk::AttachmentDescription desc = vk::AttachmentDescription
        (
            { },
            format,
            vk::SampleCountFlagBits::e1,
            vk::AttachmentLoadOp::eClear,
            vk::AttachmentStoreOp::eStore,
            vk::AttachmentLoadOp::eDontCare,
            vk::AttachmentStoreOp::eDontCare,
            vk::ImageLayout::eUndefined,
            vk::ImageLayout::eShaderReadOnlyOptimal
        );

        for (uint32_t i = 0; i < m_textureCount; ++i)
        {
            vals[i] = desc;
        }

        if (hasDepth)
        {
            vals[m_textureCount] = vk::AttachmentDescription
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
        }

        ILRETURN vals;
    });

    const vk::AttachmentDescription* attachmentsNoClear = ILAMBDA(
    {
        vk::AttachmentDescription* vals = RenderScratchAlloc::TAllocate<vk::AttachmentDescription>(totalTextureCount);

        const vk::AttachmentDescription desc = vk::AttachmentDescription
        (
            { },
            format,
            vk::SampleCountFlagBits::e1,
            vk::AttachmentLoadOp::eLoad,
            vk::AttachmentStoreOp::eStore,
            vk::AttachmentLoadOp::eDontCare,
            vk::AttachmentStoreOp::eDontCare,
            vk::ImageLayout::eShaderReadOnlyOptimal,
            vk::ImageLayout::eShaderReadOnlyOptimal
        );

        for (uint32_t i = 0; i < m_textureCount; ++i)
        {
            vals[i] = desc;
        }

        if (hasDepth)
        {
            vals[m_textureCount] = vk::AttachmentDescription
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
        }

        ILRETURN vals;
    });

    const vk::AttachmentDescription* attachmentsColorClear = ILAMBDA(
    {
        vk::AttachmentDescription* vals = RenderScratchAlloc::TAllocate<vk::AttachmentDescription>(totalTextureCount);

        const vk::AttachmentDescription desc = vk::AttachmentDescription
        (
            { },
            format,
            vk::SampleCountFlagBits::e1,
            vk::AttachmentLoadOp::eClear,
            vk::AttachmentStoreOp::eStore,
            vk::AttachmentLoadOp::eDontCare,
            vk::AttachmentStoreOp::eDontCare,
            vk::ImageLayout::eUndefined,
            vk::ImageLayout::eShaderReadOnlyOptimal
        );

        for (uint32_t i = 0; i < m_textureCount; ++i)
        {
            vals[i] = desc;
        }

        if (hasDepth)
        {
            vals[m_textureCount] = vk::AttachmentDescription
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
        }

        ILRETURN vals;
    });

    const vk::AttachmentReference* colorAttachmentRef = ILAMBDA(
    {
        vk::AttachmentReference* vals = RenderScratchAlloc::TAllocate<vk::AttachmentReference>(m_textureCount);

        for (uint32_t i = 0; i < m_textureCount; ++i)
        {
            vals[i] = vk::AttachmentReference
            (
                i,
                vk::ImageLayout::eColorAttachmentOptimal
            );
        }

        ILRETURN vals;
    });

    const vk::ImageLayout depthLayout = GetDepthLayout(depthFormat);

    const vk::AttachmentReference depthAttachmentRef = vk::AttachmentReference
    (
        m_textureCount,
        depthLayout
    );

    const vk::SubpassDescription subDesc = ILAMBDA(
    {
        if (hasDepth)
        {
            ILRETURN vk::SubpassDescription
            (
                { },
                vk::PipelineBindPoint::eGraphics,
                0,
                nullptr,
                m_textureCount,
                colorAttachmentRef,
                nullptr,
                &depthAttachmentRef
            );
        }

        ILRETURN vk::SubpassDescription
        (
            { },
            vk::PipelineBindPoint::eGraphics,
            0,
            nullptr,
            m_textureCount,
            colorAttachmentRef
        );
    });

    const vk::SubpassDescription subpasses[] = 
    {
        subDesc
    };
    constexpr uint32_t SubpassCount = sizeof(subpasses) / sizeof(*subpasses);

    const vk::SubpassDependency dependencies[] = 
    {
        ILAMBDA(
        {
            if (hasDepth)
            {
                ILRETURN vk::SubpassDependency
                (
                    vk::SubpassExternal,
                    0,
                    vk::PipelineStageFlagBits::eEarlyFragmentTests | vk::PipelineStageFlagBits::eLateFragmentTests,
                    vk::PipelineStageFlagBits::eEarlyFragmentTests | vk::PipelineStageFlagBits::eLateFragmentTests,
                    vk::AccessFlagBits::eDepthStencilAttachmentRead | vk::AccessFlagBits::eDepthStencilAttachmentWrite,
                    vk::AccessFlagBits::eDepthStencilAttachmentRead | vk::AccessFlagBits::eDepthStencilAttachmentWrite,
                    vk::DependencyFlagBits::eByRegion
                );
            }

            ILRETURN vk::SubpassDependency
            (
                vk::SubpassExternal,
                0,
                vk::PipelineStageFlagBits::eEarlyFragmentTests | vk::PipelineStageFlagBits::eLateFragmentTests,
                vk::PipelineStageFlagBits::eEarlyFragmentTests | vk::PipelineStageFlagBits::eLateFragmentTests,
                { },
                { },
                vk::DependencyFlagBits::eByRegion
            );
        }),
        vk::SubpassDependency
        (
            vk::SubpassExternal,
            0,
            vk::PipelineStageFlagBits::eColorAttachmentOutput,
            vk::PipelineStageFlagBits::eFragmentShader,
            vk::AccessFlagBits::eColorAttachmentWrite,
            vk::AccessFlagBits::eShaderRead,
            vk::DependencyFlagBits::eByRegion
        ),
        vk::SubpassDependency
        (
            0,
            0,
            vk::PipelineStageFlagBits::eColorAttachmentOutput,
            vk::PipelineStageFlagBits::eColorAttachmentOutput,
            vk::AccessFlagBits::eColorAttachmentWrite,
            vk::AccessFlagBits::eColorAttachmentWrite,
            vk::DependencyFlagBits::eByRegion
        ),
        vk::SubpassDependency
        (
            0,
            vk::SubpassExternal,
            vk::PipelineStageFlagBits::eColorAttachmentOutput,
            vk::PipelineStageFlagBits::eBottomOfPipe,
            vk::AccessFlagBits::eColorAttachmentWrite,
            vk::AccessFlagBits::eMemoryRead,
            vk::DependencyFlagBits::eByRegion
        ),
    };
    constexpr uint32_t DependencyCount = sizeof(dependencies) / sizeof(*dependencies);

    const vk::RenderPassCreateInfo renderPassInfo = vk::RenderPassCreateInfo
    (
        { },
        totalTextureCount,
        attachments,
        SubpassCount,
        subpasses,
        DependencyCount,
        dependencies
    );
    VKRESERRMSG(device.createRenderPass(&renderPassInfo, nullptr, &m_renderPass), "Failed to create RenderTexture RenderPass");

    const vk::RenderPassCreateInfo renderPassNoClearInfo = vk::RenderPassCreateInfo
    (
        { },
        totalTextureCount,
        attachmentsNoClear,
        SubpassCount,
        subpasses,
        DependencyCount,
        dependencies
    );
    VKRESERRMSG(device.createRenderPass(&renderPassNoClearInfo, nullptr, &m_renderPassNoClear), "Failed to create RenderTexture RenderPass");
    
    const vk::RenderPassCreateInfo renderPassColorClearInfo = vk::RenderPassCreateInfo
    (
        { },
        totalTextureCount,
        attachmentsColorClear,
        SubpassCount,
        subpasses,
        DependencyCount,
        dependencies
    );
    VKRESERRMSG(device.createRenderPass(&renderPassColorClearInfo, nullptr, &m_renderPassColorClear), "Failed to create RenderTexture RenderPass");

    m_textures = blockAllocator->TAllocate<vk::Image>(m_textureCount);
    m_textureAllocations = blockAllocator->TAllocate<VmaAllocation>(m_textureCount);
    m_textureViews = blockAllocator->TAllocate<vk::ImageView>(totalTextureCount);
    m_clearValues = blockAllocator->TAllocate<vk::ClearValue>(totalTextureCount);
    for (uint32_t i = 0; i < m_textureCount; ++i)
    {
        m_clearValues[i] = vk::ClearValue({ 0.0f, 0.0f, 0.0f, 0.0f });
    }
    if (hasDepth)
    {
        m_clearValues[m_textureCount] = vk::ClearValue(vk::ClearDepthStencilValue(1.0f, 0));
    }

    Init(m_width, m_height);
}

VulkanRenderTexture::VulkanRenderTexture(VulkanRenderEngineBackend* a_engine, VulkanGraphicsEngine* a_gEngine, uint32_t a_textureCount, uint32_t a_width, uint32_t a_height, bool a_depthTexture, bool a_hdr, uint32_t a_channelCount)
{
    m_engine = a_engine;
    m_gEngine = a_gEngine;

    m_textureCount = a_textureCount;
    m_channelCount = a_channelCount;

    m_width = a_width;
    m_height = a_height;

    m_depthHandle = -1;

    m_flags = 0;
    if (a_hdr)
    {
        ISETBIT(m_flags, HDRFlag);
    }
    if (a_depthTexture)
    {
        ISETBIT(m_flags, OwnsDepthTextureFlag);

        m_depthHandle = m_gEngine->GenerateDepthRenderTexture(m_width, m_height);
    }

    Setup();
}
VulkanRenderTexture::VulkanRenderTexture(VulkanRenderEngineBackend* a_engine, VulkanGraphicsEngine* a_gEngine, uint32_t a_textureCount, uint32_t a_width, uint32_t a_height, uint32_t a_depthHandle, bool a_hdr, uint32_t a_channelCount)
{
    m_engine = a_engine;
    m_gEngine = a_gEngine;

    m_textureCount = a_textureCount;
    m_channelCount = a_channelCount;

    m_width = a_width;
    m_height = a_height;

    m_depthHandle = a_depthHandle;

    m_flags = 0;
    if (a_hdr)
    {
        ISETBIT(m_flags, HDRFlag);
    }

    Setup();
}
VulkanRenderTexture::~VulkanRenderTexture()
{
    BlockAllocator* blockAllocator = m_engine->GetBlockAllocator();

    TRACE("Queueing Render Texture for Deletion");
    m_engine->PushDeletionObject<VulkanRenderTextureDeletionObject>(m_engine, m_textureCount, m_textures, m_textureViews, m_textureAllocations, m_frameBuffer);
    m_engine->PushDeletionObject<VulkanRenderTextureRenderPassDeletionObject>(m_engine, m_renderPass, m_renderPassColorClear, m_renderPassNoClear);

    if (IISBITSET(m_flags, OwnsDepthTextureFlag))
    {
        m_gEngine->DestroyDepthRenderTexture(m_depthHandle);
    }

    blockAllocator->Free(m_textures);
    blockAllocator->Free(m_textureViews);
    blockAllocator->Free(m_textureAllocations);
    blockAllocator->Free(m_clearValues);
}

vk::Image VulkanRenderTexture::GetDepthTexture() const
{
    const VulkanDepthRenderTexture* texture = m_gEngine->GetDepthRenderTexture(m_depthHandle);
    if (texture == nullptr)
    {
        return nullptr;
    }

    return texture->GetTexture();
}

void VulkanRenderTexture::Init(uint32_t a_width, uint32_t a_height)
{
    const vk::Device device = m_engine->GetLogicalDevice();
    const VmaAllocator allocator = m_engine->GetAllocator();

    const bool isHDR = IsHDR();

    const vk::Format format = GetFormat(isHDR, m_channelCount);

    const uint32_t totalTextureCount = GetTotalTextureCount();

    m_width = a_width;
    m_height = a_height;

    const vk::Extent3D extent = vk::Extent3D(m_width, m_height, 1);

    TRACE("Creating Textures");
    const VkImageCreateInfo textureCreateInfo = VkImageCreateInfo
    {
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .imageType = VK_IMAGE_TYPE_2D,
        .format = (VkFormat)format,
        .extent = extent,
        .mipLevels = 1,
        .arrayLayers = 1,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .tiling = VK_IMAGE_TILING_OPTIMAL,
        .usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
    };

    const VmaAllocationCreateInfo allocInfo = 
    { 
        .usage = VMA_MEMORY_USAGE_AUTO,
        .preferredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
    };

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
        VKRESERRMSG(vmaCreateImage(allocator, &textureCreateInfo, &allocInfo, &image, &m_textureAllocations[i], nullptr), "Failed to create RenderTexture image");
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

        VKRESERRMSG(device.createImageView(&textureImageView, nullptr, &m_textureViews[i]), "Failed to create RenderTexture ImageView");
    }

    const bool hasDepth = HasDepthTexture();
    if (hasDepth)
    {
        const VulkanDepthRenderTexture* depth = m_gEngine->GetDepthRenderTexture(m_depthHandle);

        m_textureViews[m_textureCount] = depth->GetImageView();
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

    VKRESERR(device.createFramebuffer(&fbCreateInfo, nullptr, &m_frameBuffer));

    TLockObj<vk::CommandBuffer, SpinLock>* l = m_engine->BeginSingleCommand();
    IDEFER(m_engine->EndSingleCommand(l));

    vk::CommandBuffer commandBuffer = l->Get();

    for (uint32_t i = 0; i < m_textureCount; ++i)
    {
        const vk::ImageMemoryBarrier memoryBarrier = vk::ImageMemoryBarrier
        (
            vk::AccessFlags(),
            vk::AccessFlagBits::eShaderRead,
            vk::ImageLayout::eUndefined,
            vk::ImageLayout::eShaderReadOnlyOptimal,
            vk::QueueFamilyIgnored,
            vk::QueueFamilyIgnored,
            m_textures[i],
            SubresourceRange
        );

        commandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eFragmentShader, vk::PipelineStageFlagBits::eFragmentShader, vk::DependencyFlags(), 0, nullptr, 0, nullptr, 1, &memoryBarrier);
    }
}

void VulkanRenderTexture::Resize(uint32_t a_width, uint32_t a_height)
{
    TRACE("Resizing Render Texture");
    m_engine->PushDeletionObject<VulkanRenderTextureDeletionObject>(m_engine, m_textureCount, m_textures, m_textureViews, m_textureAllocations, m_frameBuffer);

    if (IISBITSET(m_flags, OwnsDepthTextureFlag))
    {
        IVERIFY(HasDepthTexture());

        VulkanDepthRenderTexture* depth = m_gEngine->GetDepthRenderTexture(m_depthHandle);

        depth->Resize(a_width, a_height);
    }

    Init(a_width, a_height);
}

#endif

// MIT License
// 
// Copyright (c) 2025 River Govers
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