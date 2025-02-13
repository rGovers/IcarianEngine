// Icarian Engine - C# Game Engine
// 
// License at end of file.

#ifdef ICARIANNATIVE_ENABLE_GRAPHICS_VULKAN

#include "Rendering/Vulkan/VulkanSwapchain.h"

#include "AppWindow/AppWindow.h"
#include "AppWindow/HeadlessAppWindow.h"
#include "Core/IcarianDefer.h"
#include "Rendering/Vulkan/VulkanRenderEngineBackend.h"
#include "Runtime/RuntimeFunction.h"
#include "Runtime/RuntimeManager.h"
#include "Trace.h"

static vk::SurfaceFormatKHR GetSurfaceFormatFromFormats(const Array<vk::SurfaceFormatKHR>& a_formats)
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

    return vk::Extent2D(glm::clamp(a_width, minExtent.width, maxExtent.width), glm::clamp(a_height, minExtent.height, maxExtent.height));
}

void VulkanSwapchain::Init(uint32_t a_width, uint32_t a_height)
{
    const vk::Instance instance = m_engine->GetInstance();
    const vk::PhysicalDevice pDevice = m_engine->GetPhysicalDevice();
    const vk::SurfaceKHR surface = m_window->GetSurface(instance);
    const vk::Device device = m_engine->GetLogicalDevice();

    device.waitIdle();

    const SwapChainSupportInfo info = QuerySwapChainSupport(pDevice, surface);

    constexpr vk::PresentModeKHR PresentMode = vk::PresentModeKHR::eFifo;
    const vk::Extent2D extents = GetSwapExtent(info.Capabilites, a_width, a_height);

    m_width = extents.width;
    m_height = extents.height;

    uint32_t imageCount = info.Capabilites.minImageCount + 1;
    if (info.Capabilites.maxImageCount > 0)
    {
        imageCount = glm::min(imageCount, info.Capabilites.maxImageCount);
    }

    TRACE("Creating Vulkan Swapchain");
    vk::SwapchainCreateInfoKHR createInfo = vk::SwapchainCreateInfoKHR
    (
        { }, 
        surface, 
        imageCount, 
        m_surfaceFormat.format, 
        m_surfaceFormat.colorSpace, 
        extents, 
        1, 
        vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferDst, 
        vk::SharingMode::eExclusive, 
        nullptr,
        info.Capabilites.currentTransform,
        vk::CompositeAlphaFlagBitsKHR::eOpaque,
        PresentMode,
        VK_TRUE
    );

    const uint32_t queueFamilyIndices[] = { m_engine->GetGraphicsQueueIndex(), m_engine->GetPresentQueueIndex() };

    if (m_engine->GetGraphicsQueue() != m_engine->GetPresentQueue())
    {
        createInfo.imageSharingMode = vk::SharingMode::eConcurrent;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    }

    VKRESERRMSG(device.createSwapchainKHR(&createInfo, nullptr, &m_swapchain), "Failed to create swapchain");

    TRACE("Getting Swapchain images");
    VKRESERR(device.getSwapchainImagesKHR(m_swapchain, &imageCount, nullptr));

    m_images.Reserve(imageCount);
    vk::Image* images = new vk::Image[imageCount];
    IDEFER(delete[] images);

    VKRESERR(device.getSwapchainImagesKHR(m_swapchain, &imageCount, images));

    TRACE("Creating swapchain framebuffers");
    for (uint32_t i = 0; i < imageCount; ++i)
    {
        VulkanSwapchainImage swapImage = 
        { 
            .Image = images[i]
        };

        const vk::ImageViewCreateInfo createInfo = vk::ImageViewCreateInfo
        (
            { }, 
            swapImage.Image, 
            vk::ImageViewType::e2D, 
            m_surfaceFormat.format, 
            { vk::ComponentSwizzle::eIdentity, vk::ComponentSwizzle::eIdentity, vk::ComponentSwizzle::eIdentity, vk::ComponentSwizzle::eIdentity },
            vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1)
        );

        VKRESERRMSG(device.createImageView(&createInfo, nullptr, &swapImage.View), "Failed to create swapchain ImageView");

        const vk::FramebufferCreateInfo framebufferInfo = vk::FramebufferCreateInfo
        (
            { },
            m_renderPass,
            1,
            &swapImage.View,
            m_width,
            m_height,
            1
        );

        VKRESERRMSG(device.createFramebuffer(&framebufferInfo, nullptr, &swapImage.Framebuffer), "Failed to create swapchain framebuffer");

        m_images.Push(swapImage);
    }
}
void VulkanSwapchain::InitHeadless(uint32_t a_width, uint32_t a_height)
{
#ifndef ICARIANNATIVE_ENABLE_DMA
    m_init = 0;
#endif

    m_width = glm::max(2U, a_width);
    m_height = glm::max(2U, a_height);

    const VmaAllocator allocator = m_engine->GetAllocator();
    const vk::Device device = m_engine->GetLogicalDevice();

    device.waitIdle();

    const vk::Extent3D extents = vk::Extent3D(m_width, m_height, 1);

#ifdef ICARIANNATIVE_ENABLE_DMA
    const VkExternalMemoryImageCreateInfo externalImageInfo =
    {
        .sType = VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_IMAGE_CREATE_INFO,
#ifdef WIN32
        .handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_WIN32_BIT,
#else
        .handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT,
#endif
    };
#endif

    const VkImageCreateInfo imageInfo = 
    { 
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
#ifdef ICARIANNATIVE_ENABLE_DMA
        .pNext = &externalImageInfo,
#endif
        .imageType = VK_IMAGE_TYPE_2D,
        .format = VK_FORMAT_R8G8B8A8_UNORM,
        .extent = extents,
        .mipLevels = 1,
        .arrayLayers = 1,
        .samples = VK_SAMPLE_COUNT_1_BIT,
#ifdef ICARIANNATIVE_ENABLE_DMA
        .tiling = VK_IMAGE_TILING_LINEAR,
#else
        .tiling = VK_IMAGE_TILING_OPTIMAL,
#endif
        .usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
    };

    const VmaAllocationCreateInfo allocInfo = 
    { 
        .usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE,
#ifdef ICARIANNATIVE_ENABLE_DMA
        .pool = m_pool,
#endif
    };

    m_images.Reserve(VulkanMaxFlightFrames);

    TRACE("Creating Swapchain Headless Images");
    for (uint32_t i = 0; i < VulkanMaxFlightFrames; ++i)
    {
        VulkanSwapchainImage swapImage = { };

        VkImage image;
        VmaAllocationInfo info;
        VKRESERRMSG(vmaCreateImage
        (
            allocator, 
            &imageInfo,
            &allocInfo, 
            &image, 
            &swapImage.Allocation, 
#ifdef ICARIANNATIVE_ENABLE_DMA
            &info
#else
            NULL
#endif  
        ), "Failed to create swapchain image");
        swapImage.Image = image;

#ifdef ICARIANNATIVE_ENABLE_DMA
        HeadlessAppWindow* window = (HeadlessAppWindow*)m_window;
#ifdef WIN32
        const vk::MemoryGetWin32HandleInfoKHR handleInfo = vk::MemoryGetWin32HandleInfoKHR
        (
            info.deviceMemory,
            vk::ExternalMemoryHandleTypeFlagBits::eOpaqueWin32
        );

        swapImage.Handle = device.getMemoryWin32HandleKHR(handleInfo);
        IVERIFY(swapImage.Handle != INVALID_HANDLE_VALUE);

        const DMASwapBufferHandle swapBuffer =
        {
            .Width = m_width,
            .Height = m_height,
            .Size = (uint64_t)info.size,
            .Offset = (uint64_t)info.offset,
            .ImageHandle = swapImage.Handle,
            .StartSemaphore = m_startSemaphoreHandle[i],
            .EndSemaphore = m_endSemaphoreHandle[i],
        };

        window->PushSwapBufferHandle(swapBuffer);
#else
        const vk::MemoryGetFdInfoKHR fdInfo = vk::MemoryGetFdInfoKHR
        (
            info.deviceMemory,
            vk::ExternalMemoryHandleTypeFlagBits::eOpaqueFd
        );

        swapImage.FD = device.getMemoryFdKHR(fdInfo);
        IVERIFY(swapImage.FD >= 0);

        const DMASwapBufferFD swapBuffer = 
        {
            .Width = m_width,
            .Height = m_height,
            .Size = (uint64_t)info.size,
            .Offset = (uint64_t)info.offset,
            .ImageFD = swapImage.FD,
            .StartSemaphore = m_startSemaphoreFD[i],
            .EndSemaphore = m_endSemaphoreFD[i],
        };
    
        window->PushSwapBufferFD(swapBuffer);
#endif
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
            swapImage.Image,
            vk::ImageViewType::e2D,
            vk::Format::eR8G8B8A8Unorm,
            vk::ComponentMapping(),
            SubresourceRange
        );
        VKRESERRMSG(device.createImageView(&colorImageView, nullptr, &swapImage.View), "Failed to create swapchain ImageView");

        const vk::FramebufferCreateInfo framebufferInfo = vk::FramebufferCreateInfo
        (
            { },
            m_renderPass,
            1,
            &swapImage.View,
            m_width,
            m_height,
            1
        );

        VKRESERRMSG(device.createFramebuffer(&framebufferInfo, nullptr, &swapImage.Framebuffer), "Failed to create swapchain framebuffer");

        m_images.Push(swapImage);
    }

#ifndef ICARIANNATIVE_ENABLE_DMA
    TRACE("Creating Swapchain Buffer");

    const VkBufferCreateInfo buffCreateInfo = 
    { 
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = (VkDeviceSize)m_width * m_height * 4,
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
#endif
}
void VulkanSwapchain::Destroy()
{
    const VmaAllocator allocator = m_engine->GetAllocator();
    const vk::Device device = m_engine->GetLogicalDevice();

    device.waitIdle();

    const bool headless = m_window->IsHeadless() || ForceHeadless;
    if (headless)
    {
        TRACE("Destroying Headless Images");
        for (const VulkanSwapchainImage& image : m_images)
        {
            vmaDestroyImage(allocator, image.Image, image.Allocation);
        }
        
#ifdef ICARIANNATIVE_ENABLE_DMA
        HeadlessAppWindow* window = (HeadlessAppWindow*)m_window;

#ifdef WIN32
        window->FlushSwapBufferHandle();
#else
        window->FlushSwapBufferFD();
#endif
#else
        vmaDestroyBuffer(allocator, m_buffer, m_allocBuffer);
#endif
    }
    else
    {
        TRACE("Destroying Swapchain");
        device.destroySwapchainKHR(m_swapchain);
    }

        TRACE("Destroying Swapchain Images");
    for (const VulkanSwapchainImage& image : m_images)
    {
        device.destroyImageView(image.View);
        device.destroyFramebuffer(image.Framebuffer);
    }

    m_images.Clear();
}

VulkanSwapchain::VulkanSwapchain(VulkanRenderEngineBackend* a_engine, AppWindow* a_window)
{
    m_window = a_window;
    m_engine = a_engine;

    m_swapchain = nullptr;
    m_renderPass = nullptr;
    m_renderPassNoClear = nullptr;
    
    m_resizeFunc = RuntimeManager::GetFunction("IcarianEngine.Rendering", "RenderPipeline", ":ResizeS(uint,uint)");

    const vk::Instance instance = m_engine->GetInstance();
    const vk::Device device = m_engine->GetLogicalDevice();
    const vk::PhysicalDevice pDevice = m_engine->GetPhysicalDevice();
    const vk::SurfaceKHR surface = m_window->GetSurface(instance);

    const uint32_t winWidth = glm::max(2U, m_window->GetWidth());
    const uint32_t winHeight = glm::max(2U, m_window->GetHeight());

    const bool headless = a_window->IsHeadless() || ForceHeadless;

#ifdef WIN32
    constexpr vk::ExportSemaphoreCreateInfo SemaphoreExportInfo = vk::ExportSemaphoreCreateInfo
    (
        vk::ExternalSemaphoreHandleTypeFlagBits::eOpaqueWin32
    );
#else
    constexpr vk::ExportSemaphoreCreateInfo SemaphoreExportInfo = vk::ExportSemaphoreCreateInfo
    (
        vk::ExternalSemaphoreHandleTypeFlagBits::eOpaqueFd
    );
#endif

    vk::SemaphoreCreateInfo semaphoreInfo;

    if (headless)
    {
#ifdef ICARIANNATIVE_ENABLE_DMA
        const VmaAllocator allocator = m_engine->GetAllocator();

        // Want to make sure it hits one of the bigger pools?
        constexpr uint32_t ExtentSize = 1 << 13;
        const vk::Extent3D extents = vk::Extent3D(ExtentSize, ExtentSize, 1);

        const VkExternalMemoryImageCreateInfo externalImageInfo =
        {
            .sType = VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_IMAGE_CREATE_INFO,
#ifdef WIN32
            .handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_WIN32_BIT,
#else
            .handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT,
#endif
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
#ifdef WIN32
            .handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_WIN32_BIT,
#else
            .handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT,
#endif
        };

        const VmaPoolCreateInfo poolCreateInfo = 
        {
            .memoryTypeIndex = memIndex,
            .pMemoryAllocateNext = &m_exportInfo,
        };

        VKRESERRMSG(vmaCreatePool(allocator, &poolCreateInfo, &m_pool), "Failed to create Swapchain DMA Pool");

        semaphoreInfo.pNext = &SemaphoreExportInfo;
#endif

        m_surfaceFormat = vk::SurfaceFormatKHR
        (
            vk::Format::eR8G8B8A8Unorm,
            vk::ColorSpaceKHR::eSrgbNonlinear
        );
    }
    else
    {
        const SwapChainSupportInfo info = QuerySwapChainSupport(pDevice, surface);
        
        m_surfaceFormat = GetSurfaceFormatFromFormats(info.Formats);
    }

    constexpr vk::FenceCreateInfo FenceInfo = vk::FenceCreateInfo
    (
        vk::FenceCreateFlagBits::eSignaled
    );

    for (uint32_t i = 0; i < VulkanMaxFlightFrames; ++i)
    {
        VKRESERRMSG(device.createSemaphore(&semaphoreInfo, nullptr, &m_startSemaphores[i]), "Failed to create semaphore");
        VKRESERRMSG(device.createSemaphore(&semaphoreInfo, nullptr, &m_endSemaphores[i]), "Failed to create semaphore");

        VKRESERRMSG(device.createFence(&FenceInfo, nullptr, &m_fences[i]), "Failed to create fence");

#ifdef ICARIANNATIVE_ENABLE_DMA
        if (headless)
        {
#ifdef WIN32
            const vk::SemaphoreGetWin32HandleInfoKHR startHandleInfo = vk::SemaphoreGetWin32HandleInfoKHR
            (
                m_startSemaphores[i],
                vk::ExternalSemaphoreHandleTypeFlagBits::eOpaqueWin32
            );

            const vk::SemaphoreGetWin32HandleInfoKHR endHandleInfo = vk::SemaphoreGetWin32HandleInfoKHR
            (
                m_endSemaphores[i],
                vk::ExternalSemaphoreHandleTypeFlagBits::eOpaqueWin32
            );

            VKRESERR(device.getSemaphoreWin32HandleKHR(&startHandleInfo, &m_startSemaphoreHandle[i]));
            IVERIFY(m_startSemaphoreHandle[i] != INVALID_HANDLE_VALUE);

            VKRESERR(device.getSemaphoreWin32HandleKHR(&endHandleInfo, &m_endSemaphoreHandle[i]));
            IVERIFY(m_endSemaphoreHandle[i] != INVALID_HANDLE_VALUE);
#else
            const vk::SemaphoreGetFdInfoKHR startHandleInfo = vk::SemaphoreGetFdInfoKHR
            (
                m_startSemaphores[i],
                vk::ExternalSemaphoreHandleTypeFlagBits::eOpaqueFd
            );
            const vk::SemaphoreGetFdInfoKHR endHandleInfo = vk::SemaphoreGetFdInfoKHR
            (
                m_endSemaphores[i],
                vk::ExternalSemaphoreHandleTypeFlagBits::eOpaqueFd
            );

            VKRESERR(device.getSemaphoreFdKHR(&startHandleInfo, &m_startSemaphoreFD[i]));
            IVERIFY(m_startSemaphoreFD[i] >= 0);

            VKRESERR(device.getSemaphoreFdKHR(&endHandleInfo, &m_endSemaphoreFD[i]));
            IVERIFY(m_endSemaphoreFD[i] >= 0);
#endif
        }
#endif
    }

    const vk::ImageLayout imageLayout = GetImageLayout();

    const vk::AttachmentDescription colorAttachment = vk::AttachmentDescription
    (
        { },
        m_surfaceFormat.format,
        vk::SampleCountFlagBits::e1,
        vk::AttachmentLoadOp::eClear,
        vk::AttachmentStoreOp::eStore,
        vk::AttachmentLoadOp::eDontCare,
        vk::AttachmentStoreOp::eDontCare,
        vk::ImageLayout::eUndefined,
        imageLayout
    );

    const vk::AttachmentDescription colorNoClearAttachment = vk::AttachmentDescription
    (
        { },
        m_surfaceFormat.format,
        vk::SampleCountFlagBits::e1,
        vk::AttachmentLoadOp::eLoad,
        vk::AttachmentStoreOp::eStore,
        vk::AttachmentLoadOp::eDontCare,
        vk::AttachmentStoreOp::eDontCare,
        imageLayout,
        imageLayout
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

    Array<vk::SubpassDependency> dependencies;
    if (headless)
    {
        dependencies.Push(vk::SubpassDependency
        (
            VK_SUBPASS_EXTERNAL,
            0,
            vk::PipelineStageFlagBits::eBottomOfPipe,
            vk::PipelineStageFlagBits::eColorAttachmentOutput,
            vk::AccessFlagBits::eMemoryRead,
            vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite,
            vk::DependencyFlagBits::eByRegion
        ));
        dependencies.Push(vk::SubpassDependency
        (
            0,
            VK_SUBPASS_EXTERNAL,
            vk::PipelineStageFlagBits::eColorAttachmentOutput,
            vk::PipelineStageFlagBits::eBottomOfPipe,
            vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite,
            vk::AccessFlagBits::eMemoryRead,
            vk::DependencyFlagBits::eByRegion
        ));
    }
    else
    {
        dependencies.Push(vk::SubpassDependency
        (
            VK_SUBPASS_EXTERNAL,
            0,
            vk::PipelineStageFlagBits::eColorAttachmentOutput,
            vk::PipelineStageFlagBits::eColorAttachmentOutput,
            vk::AccessFlags(),
            vk::AccessFlagBits::eColorAttachmentWrite
        ));
    }

    const vk::RenderPassCreateInfo renderPassInfo = vk::RenderPassCreateInfo
    (
        { },
        1,
        &colorAttachment,
        1,
        &subpass,
        dependencies.Size(),
        dependencies.Data()
    );
    const vk::RenderPassCreateInfo renderPassNoClearInfo = vk::RenderPassCreateInfo
    (
        { },
        1,
        &colorNoClearAttachment,
        1,
        &subpass,
        dependencies.Size(),
        dependencies.Data()
    );

    VKRESERRMSG(device.createRenderPass(&renderPassInfo, nullptr, &m_renderPass), "Failed to create swapchain renderpass");
    VKRESERRMSG(device.createRenderPass(&renderPassNoClearInfo, nullptr, &m_renderPassNoClear), "Failed to create swapchain NC renderpass");

    TRACE("Created Vulkan Swapchain Renderpass");

    if (headless)
    {
        InitHeadless(winWidth, winHeight);
    }
    else
    {
        Init(winWidth, winHeight);
    }

    void* args[] =
    {
        &m_width,
        &m_height
    };

    m_resizeFunc->Exec(args);
}
VulkanSwapchain::~VulkanSwapchain()
{
    const vk::Device device = m_engine->GetLogicalDevice();

    TRACE("Destroying RenderPass");
    device.destroyRenderPass(m_renderPass);
    device.destroyRenderPass(m_renderPassNoClear);

    delete m_resizeFunc;

    Destroy();

    for (uint32_t i = 0; i < VulkanMaxFlightFrames; ++i)
    {
        device.destroySemaphore(m_startSemaphores[i]);
        device.destroySemaphore(m_endSemaphores[i]);

        device.destroyFence(m_fences[i]);
    }

#ifdef ICARIANNATIVE_ENABLE_DMA
    const bool headless = m_window->IsHeadless() || ForceHeadless;
    if (headless)
    {
        const VmaAllocator allocator = m_engine->GetAllocator();

        vmaDestroyPool(allocator, m_pool);
    }
#endif
}

SwapChainSupportInfo VulkanSwapchain::QuerySwapChainSupport(const vk::PhysicalDevice& a_device, const vk::SurfaceKHR& a_surface)
{
    SwapChainSupportInfo info = { };

    VKRESERR(a_device.getSurfaceCapabilitiesKHR(a_surface, &info.Capabilites));

    uint32_t formatCount;
    VKRESERR(a_device.getSurfaceFormatsKHR(a_surface, &formatCount, nullptr));
    if (formatCount > 0)
    {
        vk::SurfaceFormatKHR* formats = new vk::SurfaceFormatKHR[formatCount];
        IDEFER(delete[] formats);

        VKRESERR(a_device.getSurfaceFormatsKHR(a_surface, &formatCount, formats));

        info.Formats = Array<vk::SurfaceFormatKHR>(formats, formatCount);
    }

    uint32_t presentModeCount;
    VKRESERR(a_device.getSurfacePresentModesKHR(a_surface, &presentModeCount, nullptr));
    if (presentModeCount > 0)
    {
        vk::PresentModeKHR* modes = new vk::PresentModeKHR[presentModeCount];
        IDEFER(delete[] modes);

        VKRESERR(a_device.getSurfacePresentModesKHR(a_surface, &presentModeCount, modes));

        info.PresentModes = Array<vk::PresentModeKHR>(modes, presentModeCount);
    }

    return info;
}

vk::Image VulkanSwapchain::GetTexture() const
{
    const uint32_t imageIndex = m_engine->GetImageIndex();

    return m_images[imageIndex].Image;
}
vk::ImageLayout VulkanSwapchain::GetImageLayout() const
{
    const bool headless = m_window->IsHeadless() || ForceHeadless;
    if (headless)
    {
        return vk::ImageLayout::eTransferSrcOptimal;
    }

    return vk::ImageLayout::ePresentSrcKHR;
}

bool VulkanSwapchain::StartFrame(uint32_t* a_imageIndex, vk::Semaphore* a_semaphore, double a_delta, double a_time)
{
    *a_semaphore = nullptr;

    const VmaAllocator allocator = m_engine->GetAllocator();
    const vk::Device device = m_engine->GetLogicalDevice();
    const uint32_t flightFrame = m_engine->GetCurrentFlightFrame();
    const uint32_t winWidth = m_window->GetWidth();
    const uint32_t winHeight = m_window->GetHeight();

    const vk::Fence fence = m_fences[flightFrame];
    *a_semaphore = m_startSemaphores[flightFrame];

    {
        PROFILESTACK("Fence");

        const vk::Result result = device.waitForFences(1, &fence, vk::True, 1000000000);
        if (result != vk::Result::eSuccess)
        {
            VKRESWARNMSG(result, "Could not wait for fence");

            return false;
        }
    }
    
    const bool headless = m_window->IsHeadless() || ForceHeadless;
    if (headless)
    {
        if (m_width != winWidth || m_height != winHeight)
        {
            Destroy();
            InitHeadless(winWidth, winHeight);

            void* args[] =
            {
                &m_width,
                &m_height
            };

            m_resizeFunc->Exec(args);   
        }

        HeadlessAppWindow* window = (HeadlessAppWindow*)m_window;

        *a_imageIndex = (*a_imageIndex + 1) % VulkanMaxFlightFrames;

#ifndef ICARIANNATIVE_ENABLE_DMA
        if (!IISBITSET(m_init, *a_imageIndex))
        {
            if (fence != nullptr)
            {
                VKRESERR(device.resetFences(1, &fence));
            }

            return true;
        }

        *a_semaphore = m_startSemaphores[*a_imageIndex];

        char* dat;
        VKRESERR((vk::Result)vmaMapMemory(allocator, m_allocBuffer, (void**)&dat));
        IDEFER(vmaUnmapMemory(allocator, m_allocBuffer));

        window->PushFrameData(m_width, m_height, dat);
#endif
        window->PushFrameInfo(a_delta, a_time);
    }
    else
    {
        const vk::Result res = device.acquireNextImageKHR(m_swapchain, std::numeric_limits<uint64_t>::max(), *a_semaphore, nullptr, a_imageIndex);

        switch (res)
        {
        case vk::Result::eErrorOutOfDateKHR:
        {
            // Should not occur but does not mean will not so just incase
            const uint32_t newWidth = glm::max(2U, winWidth);
            const uint32_t newHeight = glm::max(2U, winHeight);

            Destroy();
            Init(newWidth, newHeight);

            void* args[] = 
            {
                &m_width, 
                &m_height
            };

            m_resizeFunc->Exec(args);

            return false;
        }
        case vk::Result::eSuccess:
        case vk::Result::eSuboptimalKHR:
        {
            const bool sizeEqual = m_width == winWidth && m_height == winHeight;
            const bool sizeValid = winWidth >= 2 && winHeight >= 2;

            if (!sizeEqual && sizeValid)
            {
                Destroy();
                Init(winWidth, winHeight);

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
    }

    {
        PROFILESTACK("Reset Fence");
        
        VKRESERR(device.resetFences(1, &fence));
    }

    return true;
}
void VulkanSwapchain::EndFrame(uint32_t a_imageIndex)
{
    const vk::Device device = m_engine->GetLogicalDevice();
    const vk::Queue presentQueue = m_engine->GetPresentQueue();
    const vk::Queue graphicsQueue = m_engine->GetGraphicsQueue();
    const uint32_t flightFrame = m_engine->GetCurrentFlightFrame();

    const bool headless = m_window->IsHeadless() || ForceHeadless;
    if (headless)
    {        
#ifdef ICARIANNATIVE_ENABLE_DMA
        HeadlessAppWindow* window = (HeadlessAppWindow*)m_window;

        window->DMASwap();
#else
        if (!IISBITSET(m_init, a_imageIndex))
        {
            ISETBIT(m_init, a_imageIndex);

            return;
        }

        TLockObj<vk::CommandBuffer, SpinLock>* buffer = m_engine->CreateCommandBuffer(vk::CommandBufferLevel::ePrimary);
        IDEFER(m_engine->DestroyCommandBuffer(buffer));
        const vk::CommandBuffer cmdBuffer = buffer->Get();
        
        constexpr vk::CommandBufferBeginInfo BufferBeginInfo = vk::CommandBufferBeginInfo
        (
            vk::CommandBufferUsageFlagBits::eOneTimeSubmit
        );
        VKRESERR(cmdBuffer.begin(&BufferBeginInfo));

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
            {0, 0, 0},
            { m_width, m_height, 1 }
        );

        cmdBuffer.copyImageToBuffer(m_images[a_imageIndex].Image, vk::ImageLayout::eTransferSrcOptimal, m_buffer, 1, &imageCopy);

        cmdBuffer.end();

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
#endif
    }
    else
    {
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
    }
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