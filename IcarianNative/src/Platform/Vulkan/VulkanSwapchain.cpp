// Icarian Engine - C# Game Engine
//
// License at end of file.

#ifdef ICARIANNATIVE_ENABLE_GRAPHICS_VULKAN

#include "Rendering/Vulkan/VulkanSwapchain.h"

#include "AppWindow/AppWindow.h"
#include "AppWindow/HeadlessAppWindow.h"
#include "Config.h"
#include "Core/DataTypes/Allocators/MallocAllocator.h"
#include "Core/IcarianDefer.h"
#include "Core/IcarianLambda.h"
#include "Rendering/Vulkan/VulkanRenderEngineBackend.h"
#include "Runtime/RuntimeFunction.h"
#include "Runtime/RuntimeManager.h"
#include "Trace.h"

#ifdef ICARIANNATIVE_ENABLE_DMA
#ifndef WIN32
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#endif
#endif

static vk::SurfaceFormatKHR GetSurfaceFormatFromFormats(const IcarianCore::Array<vk::SurfaceFormatKHR>& a_formats)
{
    for (const vk::SurfaceFormatKHR& format : a_formats)
    {
        // if (format.format == vk::Format::eB8G8R8A8Srgb && format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear)
        // To bring it in line with the editor
        // TODO: Make this configurable
        if (format.format == vk::Format::eB8G8R8A8Uint)
        {
            return format;
        }
    }

    return a_formats[0];
}

static constexpr vk::Extent2D GetSwapExtent(const vk::SurfaceCapabilitiesKHR& a_capabilities, uint32_t a_width, uint32_t a_height)
{
    const vk::Extent2D minExtent = a_capabilities.minImageExtent;
    const vk::Extent2D maxExtent = a_capabilities.maxImageExtent;

    const uint32_t xExtent = glm::clamp(a_width, minExtent.width, maxExtent.width);
    const uint32_t yExtent = glm::clamp(a_height, minExtent.height, maxExtent.height);

    return vk::Extent2D(xExtent, yExtent);
}

void VulkanSwapchain::Init(uint32_t a_width, uint32_t a_height, IcarianCore::Allocator* a_tempAllocator)
{
    m_mode = SwapchainMode_Application;

    const vk::Instance instance = m_engine->GetInstance();
    const vk::PhysicalDevice pDevice = m_engine->GetPhysicalDevice();
    const vk::SurfaceKHR surface = m_window->GetSurface(instance);
    const vk::Device device = m_engine->GetLogicalDevice();

    device.waitIdle();

    constexpr vk::SemaphoreCreateInfo SemaphoreInfo = vk::SemaphoreCreateInfo();
    for (uint32_t i = 0; i < VulkanMaxFlightFrames; ++i)
    {
        VKRESERRMSG(device.createSemaphore(&SemaphoreInfo, nullptr, &m_startSemaphores[i]), "Failed to create semaphore");
        VKRESERRMSG(device.createSemaphore(&SemaphoreInfo, nullptr, &m_endSemaphores[i]), "Failed to create semaphore");
    }

    const SwapChainSupportInfo info = QuerySwapChainSupport(pDevice, surface, a_tempAllocator, a_tempAllocator);
    const vk::Extent2D extents = GetSwapExtent(info.Capabilites, a_width, a_height);

    m_width = extents.width;
    m_height = extents.height;

    const vk::PresentModeKHR presentMode = ILAMBDA(
    {
        if (IISBITSET(m_flags, VSyncBit))
        {
            for (const vk::PresentModeKHR& p : info.PresentModes)
            {
                if (p == vk::PresentModeKHR::eMailbox)
                {
                    ILRETURN vk::PresentModeKHR::eMailbox;
                }
            }

            ILRETURN vk::PresentModeKHR::eFifo;
        }
        else
        {
            for (const vk::PresentModeKHR& p : info.PresentModes)
            {
                if (p == vk::PresentModeKHR::eImmediate)
                {
                    ILRETURN vk::PresentModeKHR::eImmediate;
                }
            }
        }

        Logger::Warning("Failed to find suitable present mode falling back to FIFO");

        ILRETURN vk::PresentModeKHR::eFifo;
    });

    const vk::SurfaceFormatKHR surfaceFormat = GetSurfaceFormat(a_tempAllocator);

    const vk::AttachmentDescription colorAttachment = vk::AttachmentDescription
    (
        { },
        surfaceFormat.format,
        vk::SampleCountFlagBits::e1,
        vk::AttachmentLoadOp::eClear,
        vk::AttachmentStoreOp::eStore,
        vk::AttachmentLoadOp::eDontCare,
        vk::AttachmentStoreOp::eDontCare,
        vk::ImageLayout::eUndefined,
        vk::ImageLayout::ePresentSrcKHR
    );

    const vk::AttachmentDescription colorNoClearAttachment = vk::AttachmentDescription
    (
        { },
        surfaceFormat.format,
        vk::SampleCountFlagBits::e1,
        vk::AttachmentLoadOp::eLoad,
        vk::AttachmentStoreOp::eStore,
        vk::AttachmentLoadOp::eDontCare,
        vk::AttachmentStoreOp::eDontCare,
        vk::ImageLayout::ePresentSrcKHR,
        vk::ImageLayout::ePresentSrcKHR
    );

    constexpr vk::AttachmentReference ColorAttachmentRef = vk::AttachmentReference
    (
        0,
        vk::ImageLayout::eColorAttachmentOptimal
    );
    const vk::SubpassDescription subpass = vk::SubpassDescription
    (
        vk::SubpassDescriptionFlags(),
        vk::PipelineBindPoint::eGraphics,
        0,
        nullptr,
        1,
        &ColorAttachmentRef
    );

    constexpr vk::SubpassDependency Dependency = vk::SubpassDependency
    (
        vk::SubpassExternal,
        0,
        vk::PipelineStageFlagBits::eColorAttachmentOutput,
        vk::PipelineStageFlagBits::eColorAttachmentOutput,
        vk::AccessFlags(),
        vk::AccessFlagBits::eColorAttachmentWrite
    );

    const vk::RenderPassCreateInfo renderPassInfo = vk::RenderPassCreateInfo
    (
        { },
        1,
        &colorAttachment,
        1,
        &subpass,
        1,
        &Dependency
    );
    const vk::RenderPassCreateInfo renderPassNoClearInfo = vk::RenderPassCreateInfo
    (
        { },
        1,
        &colorNoClearAttachment,
        1,
        &subpass,
        1,
        &Dependency
    );

    VKRESERRMSG(device.createRenderPass(&renderPassInfo, nullptr, &m_renderPass), "Failed to create swapchain renderpass");
    VKRESERRMSG(device.createRenderPass(&renderPassNoClearInfo, nullptr, &m_renderPassNoClear), "Failed to create swapchain NC renderpass");

    m_imageCount = ILAMBDA(
    {
        const uint32_t targetCount = info.Capabilites.minImageCount + 1;

        if (info.Capabilites.maxImageCount > 0)
        {
            if (targetCount > info.Capabilites.maxImageCount)
            {
                ILRETURN info.Capabilites.maxImageCount;
            }
        }

        ILRETURN targetCount;
    });

    const uint32_t graphicsQueueIndex = m_engine->GetGraphicsQueueIndex();
    const uint32_t presentQueueIndex = m_engine->GetPresentQueueIndex();

    const vk::SharingMode sharingMode = ILAMBDA(
    {
        if (graphicsQueueIndex != presentQueueIndex)
        {
            ILRETURN vk::SharingMode::eConcurrent;
        }

        ILRETURN vk::SharingMode::eExclusive;
    });

    const uint32_t queueFamilyCount = ILAMBDA(
    {
        if (graphicsQueueIndex != presentQueueIndex)
        {
            ILRETURN uint32_t(2);
        }

        ILRETURN uint32_t(1);
    });

    const uint32_t queueFamilyIndices[] = { graphicsQueueIndex, presentQueueIndex };

    TRACE("Creating Vulkan Swapchain");
    vk::SwapchainCreateInfoKHR createInfo = vk::SwapchainCreateInfoKHR
    (
        { },
        surface,
        m_imageCount,
        surfaceFormat.format,
        surfaceFormat.colorSpace,
        extents,
        1,
        vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferDst,
        sharingMode,
        queueFamilyCount,
        queueFamilyIndices,
        info.Capabilites.currentTransform,
        vk::CompositeAlphaFlagBitsKHR::eOpaque,
        presentMode,
        vk::True
    );

    VKRESERRMSG(device.createSwapchainKHR(&createInfo, nullptr, &m_swapchain), "Failed to create swapchain");

    TRACE("Getting Swapchain images");
    VKRESERR(device.getSwapchainImagesKHR(m_swapchain, &m_imageCount, nullptr));

    vk::Image* swapImages = a_tempAllocator->TAllocate<vk::Image>();
    VKRESERR(device.getSwapchainImagesKHR(m_swapchain, &m_imageCount, swapImages));

    m_images = m_allocator->TAllocate<VulkanSwapchainImage>(m_imageCount);

    TRACE("Creating Swapchain Framebuffers");
    for (uint32_t i = 0; i < m_imageCount; ++i)
    {
        const vk::ImageViewCreateInfo createInfo = vk::ImageViewCreateInfo
        (
            { },
            swapImages[i],
            vk::ImageViewType::e2D,
            surfaceFormat.format,
            { vk::ComponentSwizzle::eIdentity, vk::ComponentSwizzle::eIdentity, vk::ComponentSwizzle::eIdentity, vk::ComponentSwizzle::eIdentity },
            vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1)
        );

        vk::ImageView view;
        VKRESERRMSG(device.createImageView(&createInfo, nullptr, &view), "Failed to create swapchain ImageView");

        const vk::FramebufferCreateInfo framebufferInfo = vk::FramebufferCreateInfo
        (
            { },
            m_renderPass,
            1,
            &view,
            m_width,
            m_height,
            1
        );

        vk::Framebuffer buffer;
        VKRESERRMSG(device.createFramebuffer(&framebufferInfo, nullptr, &buffer), "Failed to create swapchain framebuffer");

        const VulkanSwapchainImage chainImage =
        {
            .Image = swapImages[i],
            .View = view,
            .Framebuffer = buffer,
        };

        m_images[i] = chainImage;
    }
}
void VulkanSwapchain::InitHeadless(uint32_t a_width, uint32_t a_height, IcarianCore::Allocator* a_tempAllocator)
{
    m_mode = SwapchainMode_Headless;

    m_init = 0;

    m_width = a_width;
    m_height = a_height;

    const VmaAllocator allocator = m_engine->GetVMAAllocator();
    const vk::Device device = m_engine->GetLogicalDevice();

    device.waitIdle();

    constexpr vk::SemaphoreCreateInfo SemaphoreInfo = vk::SemaphoreCreateInfo();
    for (uint32_t i = 0; i < VulkanMaxFlightFrames; ++i)
    {
        VKRESERRMSG(device.createSemaphore(&SemaphoreInfo, nullptr, &m_startSemaphores[i]), "Failed to create semaphore");
        VKRESERRMSG(device.createSemaphore(&SemaphoreInfo, nullptr, &m_endSemaphores[i]), "Failed to create semaphore");
    }

    const vk::SurfaceFormatKHR surfaceFormat = GetSurfaceFormat(a_tempAllocator);

    const vk::AttachmentDescription colorAttachment = vk::AttachmentDescription
    (
        { },
        surfaceFormat.format,
        vk::SampleCountFlagBits::e1,
        vk::AttachmentLoadOp::eClear,
        vk::AttachmentStoreOp::eStore,
        vk::AttachmentLoadOp::eDontCare,
        vk::AttachmentStoreOp::eDontCare,
        vk::ImageLayout::eUndefined,
        vk::ImageLayout::eTransferSrcOptimal
    );

    const vk::AttachmentDescription colorNoClearAttachment = vk::AttachmentDescription
    (
        { },
        surfaceFormat.format,
        vk::SampleCountFlagBits::e1,
        vk::AttachmentLoadOp::eLoad,
        vk::AttachmentStoreOp::eStore,
        vk::AttachmentLoadOp::eDontCare,
        vk::AttachmentStoreOp::eDontCare,
        vk::ImageLayout::eTransferSrcOptimal,
        vk::ImageLayout::eTransferSrcOptimal
    );

    constexpr vk::AttachmentReference ColorAttachmentRef = vk::AttachmentReference
    (
        0,
        vk::ImageLayout::eColorAttachmentOptimal
    );
    const vk::SubpassDescription subpass = vk::SubpassDescription
    (
        vk::SubpassDescriptionFlags(),
        vk::PipelineBindPoint::eGraphics,
        0,
        nullptr,
        1,
        &ColorAttachmentRef
    );

    constexpr vk::SubpassDependency Dependency = vk::SubpassDependency
    (
        vk::SubpassExternal,
        0,
        vk::PipelineStageFlagBits::eColorAttachmentOutput,
        vk::PipelineStageFlagBits::eColorAttachmentOutput,
        vk::AccessFlags(),
        vk::AccessFlagBits::eColorAttachmentWrite
    );

    const vk::RenderPassCreateInfo renderPassInfo = vk::RenderPassCreateInfo
    (
        { },
        1,
        &colorAttachment,
        1,
        &subpass,
        1,
        &Dependency
    );
    const vk::RenderPassCreateInfo renderPassNoClearInfo = vk::RenderPassCreateInfo
    (
        { },
        1,
        &colorNoClearAttachment,
        1,
        &subpass,
        1,
        &Dependency
    );

    VKRESERRMSG(device.createRenderPass(&renderPassInfo, nullptr, &m_renderPass), "Failed to create swapchain renderpass");
    VKRESERRMSG(device.createRenderPass(&renderPassNoClearInfo, nullptr, &m_renderPassNoClear), "Failed to create swapchain NC renderpass");

    const vk::Extent3D extents = vk::Extent3D(m_width, m_height, 1);

    const VkImageCreateInfo imageInfo =
    {
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .imageType = VK_IMAGE_TYPE_2D,
        .format = VK_FORMAT_R8G8B8A8_UNORM,
        .extent = extents,
        .mipLevels = 1,
        .arrayLayers = 1,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .tiling = VK_IMAGE_TILING_OPTIMAL,
        .usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
    };

    const VmaAllocationCreateInfo allocInfo =
    {
        .usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE,
    };

    m_imageCount = VulkanMaxFlightFrames;
    m_images = m_allocator->TAllocate<VulkanSwapchainImage>(m_imageCount);

    TRACE("Creating Swapchain Headless Images");
    for (uint32_t i = 0; i < m_imageCount; ++i)
    {
        VkImage image;
        VmaAllocation vAllocation;
        VKRESERRMSG(vmaCreateImage
        (
            allocator,
            &imageInfo,
            &allocInfo,
            &image,
            &vAllocation,
            NULL
        ), "Failed to create swapchain image");

        constexpr vk::ImageSubresourceRange SubresourceRange = vk::ImageSubresourceRange
        (
            vk::ImageAspectFlagBits::eColor,
            0,
            1,
            0,
            1
        );
        const vk::ImageViewCreateInfo colorImageView = vk::ImageViewCreateInfo
        (
            { },
            image,
            vk::ImageViewType::e2D,
            vk::Format::eR8G8B8A8Unorm,
            vk::ComponentMapping(),
            SubresourceRange
        );

        vk::ImageView view;
        VKRESERRMSG(device.createImageView(&colorImageView, nullptr, &view), "Failed to create swapchain ImageView");

        const vk::FramebufferCreateInfo framebufferInfo = vk::FramebufferCreateInfo
        (
            { },
            m_renderPass,
            1,
            &view,
            m_width,
            m_height,
            1
        );

        vk::Framebuffer buffer;
        VKRESERRMSG(device.createFramebuffer(&framebufferInfo, nullptr, &buffer), "Failed to create swapchain framebuffer");

        const VulkanSwapchainImage chainImage =
        {
            .Image = image,
            .Allocation = vAllocation,
            .View = view,
            .Framebuffer = buffer,
        };

        m_images[i] = chainImage;
    }

    TRACE("Creating Swapchain Buffer");
    const VkDeviceSize bufferSize = (VkDeviceSize)m_width * m_height * 4;
    const VkBufferCreateInfo buffCreateInfo =
    {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = bufferSize,
        .usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT,
    };

    const VmaAllocationCreateInfo allocCreateInfo =
    {
        .flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT,
        .usage = VMA_MEMORY_USAGE_AUTO,
    };

    VkBuffer buff;
    VKRESERR(vmaCreateBuffer(allocator, &buffCreateInfo, &allocCreateInfo, &buff, &m_allocBuffer, NULL));
    m_buffer = buff;
}
void VulkanSwapchain::InitHeadlessDMA(uint32_t a_width, uint32_t a_height, IcarianCore::Allocator* a_tempAllocator)
{
#ifdef ICARIANNATIVE_ENABLE_DMA
    m_mode = SwapchainMode_HeadlessDMA;

    m_init = 0;

    m_timelineVal = 0;

    m_width = a_width;
    m_height = a_height;

    const VmaAllocator allocator = m_engine->GetVMAAllocator();
    const vk::Device device = m_engine->GetLogicalDevice();

    device.waitIdle();

    // constexpr vk::SemaphoreTypeCreateInfo TimelineCreateInfo = vk::SemaphoreTypeCreateInfo
    // (
    //     vk::SemaphoreType::eTimeline,
    //     0
    // );
    // const vk::SemaphoreCreateInfo startSemaphoreInfo = vk::SemaphoreCreateInfo
    // (
    //     { },
    //     &TimelineCreateInfo
    // );

    // const vk::SemaphoreCreateInfo endSemaphoreInfo = vk::SemaphoreCreateInfo();

    constexpr vk::SemaphoreCreateInfo SemaphoreInfo = vk::SemaphoreCreateInfo();

    for (uint32_t i = 0; i < VulkanMaxFlightFrames; ++i)
    {
        VKRESERRMSG(device.createSemaphore(&SemaphoreInfo, nullptr, &m_startSemaphores[i]), "Failed to create semaphore");
        VKRESERRMSG(device.createSemaphore(&SemaphoreInfo, nullptr, &m_endSemaphores[i]), "Failed to create semaphore");
    }

    const vk::SurfaceFormatKHR surfaceFormat = GetSurfaceFormat(a_tempAllocator);

    const vk::AttachmentDescription colorAttachment = vk::AttachmentDescription
    (
        { },
        surfaceFormat.format,
        vk::SampleCountFlagBits::e1,
        vk::AttachmentLoadOp::eClear,
        vk::AttachmentStoreOp::eStore,
        vk::AttachmentLoadOp::eDontCare,
        vk::AttachmentStoreOp::eDontCare,
        vk::ImageLayout::eUndefined,
        vk::ImageLayout::eTransferSrcOptimal
    );

    const vk::AttachmentDescription colorNoClearAttachment = vk::AttachmentDescription
    (
        { },
        surfaceFormat.format,
        vk::SampleCountFlagBits::e1,
        vk::AttachmentLoadOp::eLoad,
        vk::AttachmentStoreOp::eStore,
        vk::AttachmentLoadOp::eDontCare,
        vk::AttachmentStoreOp::eDontCare,
        vk::ImageLayout::eTransferSrcOptimal,
        vk::ImageLayout::eTransferSrcOptimal
    );

    constexpr vk::AttachmentReference ColorAttachmentRef = vk::AttachmentReference
    (
        0,
        vk::ImageLayout::eColorAttachmentOptimal
    );
    const vk::SubpassDescription subpass = vk::SubpassDescription
    (
        vk::SubpassDescriptionFlags(),
        vk::PipelineBindPoint::eGraphics,
        0,
        nullptr,
        1,
        &ColorAttachmentRef
    );

    constexpr vk::SubpassDependency Dependency = vk::SubpassDependency
    (
        vk::SubpassExternal,
        0,
        vk::PipelineStageFlagBits::eColorAttachmentOutput,
        vk::PipelineStageFlagBits::eColorAttachmentOutput,
        vk::AccessFlags(),
        vk::AccessFlagBits::eColorAttachmentWrite
    );

    const vk::RenderPassCreateInfo renderPassInfo = vk::RenderPassCreateInfo
    (
        { },
        1,
        &colorAttachment,
        1,
        &subpass,
        1,
        &Dependency
    );
    const vk::RenderPassCreateInfo renderPassNoClearInfo = vk::RenderPassCreateInfo
    (
        { },
        1,
        &colorNoClearAttachment,
        1,
        &subpass,
        1,
        &Dependency
    );

    VKRESERRMSG(device.createRenderPass(&renderPassInfo, nullptr, &m_renderPass), "Failed to create swapchain renderpass");
    VKRESERRMSG(device.createRenderPass(&renderPassNoClearInfo, nullptr, &m_renderPassNoClear), "Failed to create swapchain NC renderpass");

    const vk::Extent3D extents = vk::Extent3D(m_width, m_height, 1);

    const VkExternalMemoryImageCreateInfo externalImageInfo =
    {
        .sType = VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_IMAGE_CREATE_INFO,
        .handleTypes = ILAMBDA(
        {
#ifdef WIN32
            ILRETURN VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_WIN32_BIT;
#else
            ILRETURN VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT;
#endif
        }),
    };

    const VkImageCreateInfo imageInfo =
    {
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .pNext = &externalImageInfo,
        .imageType = VK_IMAGE_TYPE_2D,
        .format = VK_FORMAT_R8G8B8A8_UNORM,
        .extent = extents,
        .mipLevels = 1,
        .arrayLayers = 1,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .tiling = VK_IMAGE_TILING_LINEAR,
        .usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
    };

    const VmaAllocationCreateInfo allocInfo =
    {
        .usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE,
        .pool = m_pool,
    };

    m_imageCount = VulkanMaxFlightFrames;
    m_images = m_allocator->TAllocate<VulkanSwapchainImage>(m_imageCount);

    TRACE("Creating Swapchain Headless Images");
    for (uint32_t i = 0; i < m_imageCount; ++i)
    {
        VkImage image;
        VmaAllocation vAllocation;
        VmaAllocationInfo info;
        VKRESERRMSG(vmaCreateImage
        (
            allocator,
            &imageInfo,
            &allocInfo,
            &image,
            &vAllocation,
            &info
        ), "Failed to create swapchain image");

        HeadlessAppWindow* window = (HeadlessAppWindow*)m_window;
#ifdef WIN32
        const vk::MemoryGetWin32HandleInfoKHR handleInfo = vk::MemoryGetWin32HandleInfoKHR
        (
            info.deviceMemory,
            vk::ExternalMemoryHandleTypeFlagBits::eOpaqueWin32
        );

        HANDLE handle = device.getMemoryWin32HandleKHR(handleInfo);
        IVERIFY(handle != INVALID_HANDLE_VALUE);

        const IcarianCore::DMASwapBufferHandle swapBuffer =
        {
            .Width = m_width,
            .Height = m_height,
            .Size = (uint64_t)info.size,
            .Offset = (uint64_t)info.offset,
            .ImageHandle = handle,
        };

        window->PushSwapBufferHandle(swapBuffer);
#else
        const vk::MemoryGetFdInfoKHR fdInfo = vk::MemoryGetFdInfoKHR
        (
            info.deviceMemory,
            vk::ExternalMemoryHandleTypeFlagBits::eOpaqueFd
        );

        int fd = device.getMemoryFdKHR(fdInfo);
        IVERIFY(fd >= 0);

        const IcarianCore::DMASwapBufferFD swapBuffer =
        {
            .Width = m_width,
            .Height = m_height,
            .Size = (uint64_t)info.size,
            .Offset = (uint64_t)info.offset,
            .ImageFD = fd,
        };

        window->PushSwapBufferFD(swapBuffer);
#endif

        constexpr vk::ImageSubresourceRange SubresourceRange = vk::ImageSubresourceRange
        (
            vk::ImageAspectFlagBits::eColor,
            0,
            1,
            0,
            1
        );
        const vk::ImageViewCreateInfo colorImageView = vk::ImageViewCreateInfo
        (
            { },
            image,
            vk::ImageViewType::e2D,
            vk::Format::eR8G8B8A8Unorm,
            vk::ComponentMapping(),
            SubresourceRange
        );

        vk::ImageView view;
        VKRESERRMSG(device.createImageView(&colorImageView, nullptr, &view), "Failed to create swapchain ImageView");

        const vk::FramebufferCreateInfo framebufferInfo = vk::FramebufferCreateInfo
        (
            { },
            m_renderPass,
            1,
            &view,
            m_width,
            m_height,
            1
        );

        vk::Framebuffer buffer;
        VKRESERRMSG(device.createFramebuffer(&framebufferInfo, nullptr, &buffer), "Failed to create swapchain framebuffer");

        const VulkanSwapchainImage chainImage =
        {
            .Image = image,
            .Allocation = vAllocation,
            .View = view,
            .Framebuffer = buffer,
#ifdef WIN32
            .Handle = handle,
#else
            .FD = fd,
#endif
        };

        m_images[i] = chainImage;
    }
#else
    IERROR("Init DMA swapchain with DMA disabled in build");
#endif
}
void VulkanSwapchain::Destroy()
{
    const VmaAllocator allocator = m_engine->GetVMAAllocator();
    const vk::Device device = m_engine->GetLogicalDevice();

    device.waitIdle();

    switch (m_mode)
    {
    case SwapchainMode_Application:
    {
        TRACE("Destroying Swapchain");
        device.destroySwapchainKHR(m_swapchain);

        break;
    }
    case SwapchainMode_Headless:
    {
        TRACE("Destroying Headless Images");
        if (m_images != nullptr)
        {
            for (uint32_t i = 0; i < m_imageCount; ++i)
            {
                const VulkanSwapchainImage& image = m_images[i];

                vmaDestroyImage(allocator, image.Image, image.Allocation);
            }
        }

        vmaDestroyBuffer(allocator, m_buffer, m_allocBuffer);

        break;
    }
    case SwapchainMode_HeadlessDMA:
    {
        TRACE("Destroying Headless Images");
        if (m_images != nullptr)
        {
            for (uint32_t i = 0; i < m_imageCount; ++i)
            {
                const VulkanSwapchainImage& image = m_images[i];

                vmaDestroyImage(allocator, image.Image, image.Allocation);
            }
        }

        HeadlessAppWindow* window = (HeadlessAppWindow*)m_window;
#ifdef WIN32
        window->FlushSwapBufferHandle();
#else
        window->FlushSwapBufferFD();
#endif

        break;
    }
    default:
    {
        IERROR("Invalid swapchain mode");

        break;
    }
    }

    if (m_images != nullptr)
    {
        TRACE("Destroying Swapchain Images");
        for (uint32_t i = 0; i < m_imageCount; ++i)
        {
            const VulkanSwapchainImage& image = m_images[i];

            device.destroyImageView(image.View);
            device.destroyFramebuffer(image.Framebuffer);
        }

        m_allocator->Free(m_images);
        m_imageCount = 0;
    }

    TRACE("Destroying RenderPass");
    if (m_renderPass != vk::RenderPass(nullptr))
    {
        device.destroyRenderPass(m_renderPass);
    }
    if (m_renderPassNoClear != vk::RenderPass(nullptr))
    {
        device.destroyRenderPass(m_renderPassNoClear);
    }

    for (uint32_t i = 0; i < VulkanMaxFlightFrames; ++i)
    {
        device.destroySemaphore(m_startSemaphores[i]);
        device.destroySemaphore(m_endSemaphores[i]);
    }
}

VulkanSwapchain::VulkanSwapchain
(
    VulkanRenderEngineBackend* a_engine,
    AppWindow* a_window,
    const Config* a_config,
    IcarianCore::Allocator* a_allocator,
    IcarianCore::Allocator* a_tempAllocator
)
{
    m_allocator = a_allocator;

    m_window = a_window;
    m_engine = a_engine;

    m_images = nullptr;
    m_swapchain = nullptr;
    m_renderPass = nullptr;
    m_renderPassNoClear = nullptr;

    m_init = 0;
    m_flags = 0;

    m_imageCount = 0;
    m_mode = SwapchainMode_Null;

    const bool allowDMA = a_config->AllowDMA();
    if (allowDMA)
    {
        ISETBIT(m_flags, DMABit);
    }

    m_resizeFunc = RuntimeManager::GetFunction("IcarianEngine.Rendering", "RenderPipeline", ":ResizeS(uint,uint)");

    const vk::Device device = m_engine->GetLogicalDevice();

#ifdef ICARIANNATIVE_ENABLE_DMA
    m_dmaBuffer = nullptr;

    m_ipcID = a_config->GetIPCID();

    if (allowDMA)
    {
#ifndef WIN32
        const IcarianCore::COWU8String dmaAddr = DMAName + IcarianCore::COWU8String::FromValue(m_ipcID, 10, a_tempAllocator);
        const int dmaFd = shm_open(dmaAddr.CStr(), O_RDWR, 0);
        if (dmaFd >= 0)
        {
            IDEFER(close(dmaFd));

            m_dmaBuffer = (IcarianCore::DMAMemoryBuffer*)mmap(NULL, sizeof(IcarianCore::DMAMemoryBuffer), PROT_READ | PROT_WRITE, MAP_SHARED, dmaFd, 0);
            if (m_dmaBuffer == MAP_FAILED || m_dmaBuffer == NULL)
            {
                shm_unlink(dmaAddr.CStr());
                m_dmaBuffer = nullptr;
            }
        }
#endif
    }

    const VmaAllocator allocator = m_engine->GetVMAAllocator();

    // Want to make sure it hits one of the bigger pools?
    constexpr uint32_t ExtentSize = 1 << 13;
    const vk::Extent3D extents = vk::Extent3D(ExtentSize, ExtentSize, 1);

    const VkExternalMemoryImageCreateInfo externalImageInfo =
    {
        .sType = VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_IMAGE_CREATE_INFO,
        .handleTypes = ILAMBDA(
        {
#ifdef WIN32
            ILRETURN VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_WIN32_BIT;
#else
            ILRETURN VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT;
#endif
        }),
    };

    const VkImageCreateInfo poolImageInfo =
    {
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .pNext = &externalImageInfo,
        .imageType = VK_IMAGE_TYPE_2D,
        .format = VK_FORMAT_R8G8B8A8_UNORM,
        .extent = extents,
        .mipLevels = 1,
        .arrayLayers = 1,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .tiling = VK_IMAGE_TILING_OPTIMAL,
        .usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
    };

    const VmaAllocationCreateInfo allocInfo =
    {
        .usage = VMA_MEMORY_USAGE_AUTO,
        .preferredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
    };

    uint32_t memIndex;
    VKRESERR(vmaFindMemoryTypeIndexForImageInfo(allocator, &poolImageInfo, &allocInfo, &memIndex));

    // Cannot be fucked and this needs to remain valid
    m_exportInfo =
    {
        .sType = VK_STRUCTURE_TYPE_EXPORT_MEMORY_ALLOCATE_INFO,
        .handleTypes = ILAMBDA(
        {
#ifdef WIN32
            ILRETURN VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_WIN32_BIT;
#else
            ILRETURN VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT;
#endif
        }),
    };

    const VmaPoolCreateInfo poolCreateInfo =
    {
        .memoryTypeIndex = memIndex,
        .pMemoryAllocateNext = &m_exportInfo,
    };
    VKRESERRMSG(vmaCreatePool(allocator, &poolCreateInfo, &m_pool), "Failed to create Swapchain DMA Pool");
#endif

    constexpr vk::FenceCreateInfo FenceInfo = vk::FenceCreateInfo
    (
        vk::FenceCreateFlagBits::eSignaled
    );

    for (uint32_t i = 0; i < VulkanMaxFlightFrames; ++i)
    {
        VKRESERRMSG(device.createFence(&FenceInfo, nullptr, &m_fences[i]), "Failed to create fence");
    }
}
VulkanSwapchain::~VulkanSwapchain()
{
    const vk::Device device = m_engine->GetLogicalDevice();

    IcarianCore::MallocAllocator::Instance->Destroy(m_resizeFunc);

    Destroy();

    for (uint32_t i = 0; i < VulkanMaxFlightFrames; ++i)
    {
        device.destroyFence(m_fences[i]);
    }

#ifdef ICARIANNATIVE_ENABLE_DMA
    const VmaAllocator allocator = m_engine->GetVMAAllocator();

    if (m_dmaBuffer != nullptr)
    {
        const IcarianCore::COWU8String addrStr = DMAName + IcarianCore::COWU8String::FromValue(m_ipcID, 10, m_allocator);
        shm_unlink(addrStr.CStr());
    }

    vmaDestroyPool(allocator, m_pool);
#endif
}

SwapChainSupportInfo VulkanSwapchain::QuerySwapChainSupport
(
    const vk::PhysicalDevice& a_device,
    const vk::SurfaceKHR& a_surface,
    IcarianCore::Allocator* a_allocator,
    IcarianCore::Allocator* a_tempAllocator
)
{
    vk::SurfaceCapabilitiesKHR capabilites;
    VKRESERR(a_device.getSurfaceCapabilitiesKHR(a_surface, &capabilites));

    const SwapChainSupportInfo info =
    {
        .Capabilites = capabilites,
        .Formats = ILAMBDA(
        {
            uint32_t formatCount;
            VKRESERR(a_device.getSurfaceFormatsKHR(a_surface, &formatCount, nullptr));
            if (formatCount > 0)
            {
                vk::SurfaceFormatKHR* formats = a_tempAllocator->TAllocate<vk::SurfaceFormatKHR>(formatCount);
                IDEFER(a_tempAllocator->Free(formats));

                VKRESERR(a_device.getSurfaceFormatsKHR(a_surface, &formatCount, formats));

                ILRETURN IcarianCore::Array<vk::SurfaceFormatKHR>(formats, formatCount, a_allocator);
            }

            ILRETURN IcarianCore::Array<vk::SurfaceFormatKHR>(a_allocator);
        }),
        .PresentModes = ILAMBDA(
        {
            uint32_t presentModeCount;
            VKRESERR(a_device.getSurfacePresentModesKHR(a_surface, &presentModeCount, nullptr));
            if (presentModeCount > 0)
            {
                vk::PresentModeKHR* modes = a_tempAllocator->TAllocate<vk::PresentModeKHR>();
                IDEFER(a_tempAllocator->Free(modes));

                VKRESERR(a_device.getSurfacePresentModesKHR(a_surface, &presentModeCount, modes));

                ILRETURN IcarianCore::Array<vk::PresentModeKHR>(modes, presentModeCount, a_allocator);
            }

            ILRETURN IcarianCore::Array<vk::PresentModeKHR>(a_allocator);
        })
    };

    return info;
}

vk::SurfaceFormatKHR VulkanSwapchain::GetSurfaceFormat(IcarianCore::Allocator* a_tempAllocator) const
{
    switch (m_mode)
    {
    case SwapchainMode_Application:
    {
        const vk::Instance instance = m_engine->GetInstance();
        const vk::PhysicalDevice pDevice = m_engine->GetPhysicalDevice();
        const vk::SurfaceKHR surface = m_window->GetSurface(instance);

        const SwapChainSupportInfo info = QuerySwapChainSupport
        (
            pDevice,
            surface,
            a_tempAllocator,
            a_tempAllocator
        );

        return GetSurfaceFormatFromFormats(info.Formats);
    }
    case SwapchainMode_Headless:
    case SwapchainMode_HeadlessDMA:
    {
        return vk::SurfaceFormatKHR
        (
            vk::Format::eR8G8B8A8Unorm,
            vk::ColorSpaceKHR::eSrgbNonlinear
        );
    }
    default:
    {
        break;
    }
    }

    IERROR("Cannot get surface format");

    return vk::SurfaceFormatKHR();
}

vk::Framebuffer VulkanSwapchain::GetFramebuffer(uint32_t a_index) const
{
    const VulkanSwapchainImage& image = m_images[a_index];

    return image.Framebuffer;
}
vk::Image VulkanSwapchain::GetTexture() const
{
    const uint32_t imageIndex = m_engine->GetImageIndex();

    return m_images[imageIndex].Image;
}
vk::ImageLayout VulkanSwapchain::GetImageLayout() const
{
    switch (m_mode)
    {
    case SwapchainMode_Application:
    {
        return vk::ImageLayout::ePresentSrcKHR;
    }
    case SwapchainMode_Headless:
    case SwapchainMode_HeadlessDMA:
    {
        return vk::ImageLayout::eTransferSrcOptimal;
    }
    default:
    {
        break;
    }
    }

    IERROR("Cannot get image layout");

    return vk::ImageLayout();
}

bool VulkanSwapchain::StartFrame(uint32_t* a_imageIndex, vk::Semaphore* a_semaphore, double a_delta, double a_time, IcarianCore::Allocator* a_tempAllocator)
{
    *a_semaphore = nullptr;

    const uint32_t winWidth = ILAMBDA(
    {
        const uint32_t val = m_window->GetWidth();
        if (val > 2)
        {
            ILRETURN val;
        }

        ILRETURN (uint32_t)2;
    });
    const uint32_t winHeight = ILAMBDA(
    {
        const uint32_t val = m_window->GetHeight();
        if (val > 2)
        {
            ILRETURN val;
        }

        ILRETURN (uint32_t)2;
    });

    const bool setup = m_mode == SwapchainMode_Null;
    if (setup)
    {
        const bool headless = m_window->IsHeadless() || ForceHeadless;
        if (headless)
        {
            if (IsDMAEnabled())
            {
                InitHeadlessDMA(winWidth, winHeight, a_tempAllocator);
            }
            else
            {
                InitHeadless(winWidth, winHeight, a_tempAllocator);
            }
        }
        else
        {
            Init(winWidth, winHeight, a_tempAllocator);
        }
    }

    const vk::Device device = m_engine->GetLogicalDevice();
    const uint32_t flightFrame = m_engine->GetCurrentFlightFrame();

    const vk::Fence fence = m_fences[flightFrame];

    {
        PROFILESTACK("Fence");

        const vk::Result result = device.waitForFences(1, &fence, vk::True, 1000);
        if (result != vk::Result::eSuccess)
        {
            VKRESWARNMSG(result, "Could not wait for fence");

            return false;
        }
    }

    const bool sizeEqual = m_width == winWidth && m_height == winHeight;
    switch (m_mode)
    {
    case SwapchainMode_Application:
    {
        *a_semaphore = m_startSemaphores[flightFrame];
        const vk::Result res = device.acquireNextImageKHR(m_swapchain, std::numeric_limits<uint64_t>::max(), *a_semaphore, nullptr, a_imageIndex);

        switch (res)
        {
        case vk::Result::eErrorOutOfDateKHR:
        {
            Destroy();
            Init(winWidth, winHeight, a_tempAllocator);

            if (!sizeEqual)
            {
                void* args[] =
                {
                    &m_width,
                    &m_height
                };

                m_resizeFunc->Exec(args);
            }

            return false;
        }
        case vk::Result::eSuccess:
        case vk::Result::eSuboptimalKHR:
        {
            if (!sizeEqual)
            {
                Destroy();
                Init(winWidth, winHeight, a_tempAllocator);

                void* args[] =
                {
                    &m_width,
                    &m_height
                };

                m_resizeFunc->Exec(args);
            }

            break;
        }
        default:
        {
            VKRESERRMSG(res, "Failed to aquire swapchain image");

            break;
        }
        }

        break;
    }
    case SwapchainMode_Headless:
    {
        if (!sizeEqual)
        {
            Destroy();
            InitHeadless(winWidth, winHeight, a_tempAllocator);

            void* args[] =
            {
                &m_width,
                &m_height
            };

            m_resizeFunc->Exec(args);
        }

        HeadlessAppWindow* window = (HeadlessAppWindow*)m_window;
        window->PushFrameInfo(a_delta, a_time);

        if (IISBITSET(m_init, *a_imageIndex))
        {
            const VmaAllocator allocator = m_engine->GetVMAAllocator();

            uint8_t* dat;
            VKRESERR((vk::Result)vmaMapMemory(allocator, m_allocBuffer, (void**)&dat));
            IDEFER(vmaUnmapMemory(allocator, m_allocBuffer));

            window->PushFrameData(m_width, m_height, dat);

            *a_semaphore = m_startSemaphores[flightFrame];
        }

        *a_imageIndex = (*a_imageIndex + 1) % VulkanMaxFlightFrames;

        break;
    }
    case SwapchainMode_HeadlessDMA:
    {
#ifdef ICARIANNATIVE_ENABLE_DMA
        if (!sizeEqual)
        {
            Destroy();
            InitHeadlessDMA(winWidth, winHeight, a_tempAllocator);

            void* args[] =
            {
                &m_width,
                &m_height
            };

            m_resizeFunc->Exec(args);
        }

        // Yes this is bad practice but the Vulkan spec does not get explict enough with semaphores and the driver and DRM freak the fuck out
        // The workaround just dont sync and handle syncing yourself
        // So yes the null semaphore is intentional and it gives me the shivers
        // *a_semaphore = m_startSemaphores[flightFrame];
        ++m_dmaBuffer->RenderOutput;

        const uint64_t timelineVal = ILAMBDA(
        {
            PROFILESTACK("Timeline Wait");

            while (true)
            {
                const uint64_t val = m_dmaBuffer->Timeline;
                if (val > m_timelineVal)
                {
                    ILRETURN val;
                }

                std::this_thread::yield();
            }

            ILRETURN uint64_t(-1);
        });
        IDEFER(m_timelineVal = timelineVal);

        HeadlessAppWindow* window = (HeadlessAppWindow*)m_window;
        window->PushFrameInfo(a_delta, a_time);

        *a_imageIndex = timelineVal % VulkanMaxFlightFrames;
#else
        IERROR("Swapchain in DMA mode with DMA disabled in build settings");
#endif

        break;
    }
    default:
    {
        IERROR("Invalid swapchain mode");

        break;
    }
    }

    {
        PROFILESTACK("Reset Fence");

        VKRESERR(device.resetFences(1, &fence));
    }

    return true;
}
void VulkanSwapchain::EndFrame(uint32_t a_imageIndex)
{
    const uint32_t flightFrame = m_engine->GetCurrentFlightFrame();

    switch (m_mode)
    {
    case SwapchainMode_Application:
    {
        const vk::Queue presentQueue = m_engine->GetPresentQueue();

        const vk::SwapchainKHR swapChains[] = { m_swapchain };

        const vk::PresentInfoKHR presentInfo = vk::PresentInfoKHR
        (
            1,
            &m_endSemaphores[flightFrame],
            1,
            swapChains,
            &a_imageIndex
        );

        VKRESWARNMSG(presentQueue.presentKHR(&presentInfo), "Failed to present swapchain");

        break;
    }
    case SwapchainMode_Headless:
    {
        ISETBIT(m_init, a_imageIndex);

        const vk::Queue graphicsQueue = m_engine->GetGraphicsQueue();

        TLockObj<vk::CommandBuffer, IcarianCore::SpinLock>* buffer = m_engine->CreateCommandBuffer(vk::CommandBufferLevel::ePrimary);
        IDEFER(m_engine->DestroyCommandBuffer(buffer));

        const vk::CommandBuffer cmdBuffer = buffer->Get();

        constexpr vk::CommandBufferBeginInfo BufferBeginInfo = vk::CommandBufferBeginInfo
        (
            vk::CommandBufferUsageFlagBits::eOneTimeSubmit
        );
        VKRESERR(cmdBuffer.begin(&BufferBeginInfo));
        IDEFER(cmdBuffer.end());

        constexpr vk::ImageSubresourceLayers SubResource = vk::ImageSubresourceLayers
        (
            vk::ImageAspectFlagBits::eColor,
            0,
            0,
            1
        );

        const vk::BufferImageCopy imageCopy = vk::BufferImageCopy
        (
            0,
            0,
            0,
            SubResource,
            { 0, 0, 0 },
            { m_width, m_height, 1 }
        );

        cmdBuffer.copyImageToBuffer(m_images[a_imageIndex].Image, vk::ImageLayout::eTransferSrcOptimal, m_buffer, 1, &imageCopy);

        constexpr vk::PipelineStageFlags WaitStages[] = { vk::PipelineStageFlagBits::eColorAttachmentOutput };

        const vk::SubmitInfo submitInfo = vk::SubmitInfo
        (
            1,
            &m_endSemaphores[flightFrame],
            WaitStages,
            1,
            &cmdBuffer,
            1,
            &m_startSemaphores[(flightFrame + 1) % VulkanMaxFlightFrames]
        );

        VKRESWARNMSG(graphicsQueue.submit(1, &submitInfo, m_fences[flightFrame]), "Failed to submit swap copy");

        break;
    }
    case SwapchainMode_HeadlessDMA:
    {
#ifndef ICARIANNATIVE_ENABLE_DMA
        IERROR("Swapchain in DMA mode with DMA disabled in build settings");
#endif

        break;
    }
    default:
    {
        IERROR("Invalid Swapchain mode");

        break;
    }
    }
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
