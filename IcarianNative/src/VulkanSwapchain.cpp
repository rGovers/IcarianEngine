#include "Rendering/Vulkan/VulkanSwapchain.h"

#include "AppWindow/AppWindow.h"
#include "AppWindow/HeadlessAppWindow.h"
#include "Flare/IcarianAssert.h"
#include "Logger.h"
#include "Rendering/Vulkan/VulkanConstants.h"
#include "Rendering/Vulkan/VulkanRenderEngineBackend.h"
#include "Runtime/RuntimeFunction.h"
#include "Runtime/RuntimeManager.h"
#include "Trace.h"

// Fixes error on Windows
#undef min

static vk::SurfaceFormatKHR GetSurfaceFormatFromFormats(const std::vector<vk::SurfaceFormatKHR>& a_formats)
{
    for (const vk::SurfaceFormatKHR& format : a_formats)
    {
        if (format.format == vk::Format::eB8G8R8A8Srgb && format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear)
        {
            return format;
        }
    }

    return a_formats[0];
}

static constexpr vk::Extent2D GetSwapExtent(const vk::SurfaceCapabilitiesKHR& a_capabilities, const glm::ivec2& a_size)
{
    const vk::Extent2D minExtent = a_capabilities.minImageExtent;
    const vk::Extent2D maxExtent = a_capabilities.maxImageExtent;

    return vk::Extent2D(glm::clamp((uint32_t)a_size.x, minExtent.width, maxExtent.width), glm::clamp((uint32_t)a_size.y, minExtent.height, maxExtent.height));
}

void VulkanSwapchain::Init(const glm::ivec2& a_size)
{
    m_size = a_size;

    const vk::Instance instance = m_engine->GetInstance();
    const vk::PhysicalDevice pDevice = m_engine->GetPhysicalDevice();
    const vk::SurfaceKHR surface = m_window->GetSurface(instance);
    const vk::Device lDevice = m_engine->GetLogicalDevice();

    const SwapChainSupportInfo info = QuerySwapChainSupport(pDevice, surface);

    constexpr vk::PresentModeKHR PresentMode = vk::PresentModeKHR::eFifo;
    const vk::Extent2D extents = GetSwapExtent(info.Capabilites, m_size);

    uint32_t imageCount = info.Capabilites.minImageCount + 1;
    if (info.Capabilites.maxImageCount > 0)
    {
        imageCount = glm::min(imageCount, info.Capabilites.maxImageCount);
    }

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
        VK_TRUE,
        m_swapchain
    );

    const uint32_t queueFamilyIndices[] = { m_engine->GetGraphicsQueueIndex(), m_engine->GetPresentQueueIndex() };

    if (m_engine->GetGraphicsQueue() != m_engine->GetPresentQueue())
    {
        createInfo.imageSharingMode = vk::SharingMode::eConcurrent;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    }

    ICARIAN_ASSERT_MSG_R(lDevice.createSwapchainKHR(&createInfo, nullptr, &m_swapchain) == vk::Result::eSuccess, "Failed to create Vulkan Swapchain");
    TRACE("Created Vulkan Swapchain");

    ICARIAN_ASSERT_R(lDevice.getSwapchainImagesKHR(m_swapchain, &imageCount, nullptr) == vk::Result::eSuccess);
    m_colorImage.resize(imageCount);
    ICARIAN_ASSERT_R(lDevice.getSwapchainImagesKHR(m_swapchain, &imageCount, m_colorImage.data()) == vk::Result::eSuccess);

    m_imageViews.resize(imageCount);
    for (uint32_t i = 0; i < imageCount; ++i)
    {
        const vk::ImageViewCreateInfo createInfo = vk::ImageViewCreateInfo
        (
            { }, 
            m_colorImage[i], 
            vk::ImageViewType::e2D, 
            m_surfaceFormat.format, 
            { vk::ComponentSwizzle::eIdentity, vk::ComponentSwizzle::eIdentity, vk::ComponentSwizzle::eIdentity, vk::ComponentSwizzle::eIdentity },
            vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1)
        );

        ICARIAN_ASSERT_MSG_R(lDevice.createImageView(&createInfo, nullptr, &m_imageViews[i]) == vk::Result::eSuccess, "Failed to create Swapchain ImageView");
    }
    TRACE("Created Vulkan Swap Images");

    m_framebuffers.resize(imageCount);
    for (uint32_t i = 0; i < imageCount; ++i)
    {
        const vk::ImageView attachments[] =
        {
            m_imageViews[i]
        };

        const vk::FramebufferCreateInfo framebufferInfo = vk::FramebufferCreateInfo
        (
            { },
            m_renderPass,
            1,
            attachments,
            (uint32_t)m_size.x,
            (uint32_t)m_size.y,
            1
        );

        ICARIAN_ASSERT_MSG_R(lDevice.createFramebuffer(&framebufferInfo, nullptr, &m_framebuffers[i]) == vk::Result::eSuccess, "Failed to create Swapchain Framebuffer");
    }
}
void VulkanSwapchain::InitHeadless(const glm::ivec2& a_size)
{
    m_size = a_size;
    m_init = 0;

    const VmaAllocator allocator = m_engine->GetAllocator();
    const vk::Device device = m_engine->GetLogicalDevice();

    VkImageCreateInfo imageInfo = { };
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
    imageInfo.extent.width = (uint32_t)m_size.x;
    imageInfo.extent.height = (uint32_t)m_size.y;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;

    VmaAllocationCreateInfo allocInfo = { 0 };
    allocInfo.usage = VMA_MEMORY_USAGE_AUTO;
    allocInfo.preferredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    allocInfo.flags = 0;

    m_colorImage.resize(VulkanMaxFlightFrames);
    m_imageViews.resize(VulkanMaxFlightFrames);
    m_framebuffers.resize(VulkanMaxFlightFrames);

    VkImage image;

    TRACE("Creating Swapchain Headless Images");
    for (uint32_t i = 0; i < VulkanMaxFlightFrames; ++i)
    {
        ICARIAN_ASSERT_MSG_R(vmaCreateImage(allocator, &imageInfo, &allocInfo, &image, &m_colorAllocation[i], nullptr) == VK_SUCCESS, "Failed to create Swapchain Image");
        m_colorImage[i] = image;

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
            m_colorImage[i],
            vk::ImageViewType::e2D,
            vk::Format::eR8G8B8A8Unorm,
            vk::ComponentMapping(),
            SubresourceRange
        );
        ICARIAN_ASSERT_MSG_R(device.createImageView(&colorImageView, nullptr, &m_imageViews[i]) == vk::Result::eSuccess, "Failed to create Swapchain ImageView");

        const vk::ImageView attachments[] = 
        {
            m_imageViews[i]
        };

        const vk::FramebufferCreateInfo framebufferInfo = vk::FramebufferCreateInfo
        (
            { },
            m_renderPass,
            1,
            attachments,
            (uint32_t)m_size.x,
            (uint32_t)m_size.y,
            1
        );

        ICARIAN_ASSERT_MSG_R(device.createFramebuffer(&framebufferInfo, nullptr, &m_framebuffers[i]) == vk::Result::eSuccess, "Failed to create Swapchain Framebuffer");
    }
    TRACE("Created Swapchain Headless Images");

    VkBufferCreateInfo buffCreateInfo = { };
    buffCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buffCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    buffCreateInfo.size = (uint32_t)m_size.x * (uint32_t)m_size.y * 4;

    VmaAllocationCreateInfo allocCreateInfo = { };
    allocCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;
    allocCreateInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;

    VkBuffer buff;
    vmaCreateBuffer(allocator, &buffCreateInfo, &allocCreateInfo, &buff, &m_allocBuffer, nullptr);
    m_buffer = buff;

    TRACE("Created Swapchain Buffer");
}
void VulkanSwapchain::Destroy()
{
    const VmaAllocator allocator = m_engine->GetAllocator();
    const vk::Device device = m_engine->GetLogicalDevice();

    TRACE("Destroying ImageViews");
    for (const vk::ImageView& imageView : m_imageViews)
    {
        device.destroyImageView(imageView);
    }

    TRACE("Destroying Framebuffers");
    for (const vk::Framebuffer& framebuffer : m_framebuffers)
    {
        device.destroyFramebuffer(framebuffer);
    }

    if (m_window->IsHeadless())
    {
        for (uint32_t i = 0; i < VulkanMaxFlightFrames; ++i)
        {
            vmaDestroyImage(allocator, m_colorImage[i], m_colorAllocation[i]);
        }
        
        vmaDestroyBuffer(allocator, m_buffer, m_allocBuffer);
    }
    else
    {
        TRACE("Destroying Swapchain");
        device.destroySwapchainKHR(m_swapchain);
    }
}

VulkanSwapchain::VulkanSwapchain(VulkanRenderEngineBackend* a_engine, AppWindow* a_window, RuntimeManager* a_runtime)
{
    m_window = a_window;
    m_engine = a_engine;
    
    m_resizeFunc = a_runtime->GetFunction("IcarianEngine.Rendering", "RenderPipeline", ":ResizeS(uint,uint)");

    for (uint32_t i = 0; i < VulkanMaxFlightFrames; ++i)
    {
        m_lastCmd[i] = nullptr;
    }

    const vk::Instance instance = m_engine->GetInstance();
    const vk::Device device = m_engine->GetLogicalDevice();
    const vk::PhysicalDevice pDevice = m_engine->GetPhysicalDevice();
    const vk::SurfaceKHR surface = m_window->GetSurface(instance);

    const glm::ivec2 winSize = m_window->GetSize();

    const bool headless = a_window->IsHeadless();

    if (!headless)
    {
        const SwapChainSupportInfo info = QuerySwapChainSupport(pDevice, surface);
        
        m_surfaceFormat = GetSurfaceFormatFromFormats(info.Formats);
    }

    vk::AttachmentDescription colorAttachment = vk::AttachmentDescription
    (
        vk::AttachmentDescriptionFlags(),
        m_surfaceFormat.format,
        vk::SampleCountFlagBits::e1,
        vk::AttachmentLoadOp::eClear,
        vk::AttachmentStoreOp::eStore,
        vk::AttachmentLoadOp::eDontCare,
        vk::AttachmentStoreOp::eDontCare,
        vk::ImageLayout::eUndefined,
        GetImageLayout()
    );
    vk::AttachmentDescription colorNoClearAttachment = vk::AttachmentDescription
    (
        { },
        m_surfaceFormat.format,
        vk::SampleCountFlagBits::e1,
        vk::AttachmentLoadOp::eLoad,
        vk::AttachmentStoreOp::eStore,
        vk::AttachmentLoadOp::eDontCare,
        vk::AttachmentStoreOp::eDontCare,
        vk::ImageLayout::eUndefined,
        GetImageLayout()
    );
    if (headless)
    {
        colorAttachment.format = vk::Format::eR8G8B8A8Unorm;
        colorNoClearAttachment.format = vk::Format::eR8G8B8A8Unorm;
    }

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

    std::vector<vk::SubpassDependency> dependencies;
    if (headless)
    {
        dependencies.emplace_back(vk::SubpassDependency
        (
            VK_SUBPASS_EXTERNAL,
            0,
            vk::PipelineStageFlagBits::eBottomOfPipe,
            vk::PipelineStageFlagBits::eColorAttachmentOutput,
            vk::AccessFlagBits::eMemoryRead,
            vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite,
            vk::DependencyFlagBits::eByRegion
        ));
        dependencies.emplace_back(vk::SubpassDependency
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
        dependencies.emplace_back(vk::SubpassDependency
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
        (uint32_t)dependencies.size(),
        dependencies.data()
    );
    const vk::RenderPassCreateInfo renderPassNoClearInfo = vk::RenderPassCreateInfo
    (
        { },
        1,
        &colorNoClearAttachment,
        1,
        &subpass,
        (uint32_t)dependencies.size(),
        dependencies.data()
    );

    ICARIAN_ASSERT_MSG(device.createRenderPass(&renderPassInfo, nullptr, &m_renderPass) == vk::Result::eSuccess, "Failed to create Swapchain Renderpass");
    ICARIAN_ASSERT_MSG(device.createRenderPass(&renderPassNoClearInfo, nullptr, &m_renderPassNoClear) == vk::Result::eSuccess, "Failed to create Swapchain Renderpass");

    TRACE("Created Vulkan Swapchain Renderpass");

    if (headless)
    {
        InitHeadless(winSize);
    }
    else
    {
        Init(winSize);
    }

    uint32_t width = (uint32_t)winSize.x;
    uint32_t height = (uint32_t)winSize.y;

    void* args[] =
    {
        &width,
        &height
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
}

SwapChainSupportInfo VulkanSwapchain::QuerySwapChainSupport(const vk::PhysicalDevice& a_device, const vk::SurfaceKHR& a_surface)
{
    SwapChainSupportInfo info;

    std::ignore = a_device.getSurfaceCapabilitiesKHR(a_surface, &info.Capabilites);

    uint32_t formatCount;
    std::ignore = a_device.getSurfaceFormatsKHR(a_surface, &formatCount, nullptr);
    if (formatCount != 0)
    {
        info.Formats.resize(formatCount);
        std::ignore = a_device.getSurfaceFormatsKHR(a_surface, &formatCount, info.Formats.data());
    }

    uint32_t presentModeCount;
    std::ignore = a_device.getSurfacePresentModesKHR(a_surface, &presentModeCount, nullptr);
    if (presentModeCount != 0)
    {
        info.PresentModes.resize(presentModeCount);
        std::ignore = a_device.getSurfacePresentModesKHR(a_surface, &presentModeCount, info.PresentModes.data());
    }

    return info;
}

vk::Image VulkanSwapchain::GetTexture() const
{
    return m_colorImage[m_engine->GetImageIndex()];
}
vk::ImageLayout VulkanSwapchain::GetImageLayout() const
{
    if (m_window->IsHeadless())
    {
        return vk::ImageLayout::eTransferSrcOptimal;
    }

    return vk::ImageLayout::ePresentSrcKHR;
}

bool VulkanSwapchain::StartFrame(const vk::Semaphore& a_semaphore, const vk::Fence& a_fence, uint32_t* a_imageIndex, double a_delta, double a_time)
{
    const VmaAllocator allocator = m_engine->GetAllocator();
    const vk::Device device = m_engine->GetLogicalDevice();
    const glm::ivec2 size = m_window->GetSize();
    
    {
        PROFILESTACK("Fence");
        vk::Result r = device.waitForFences(1, &a_fence, VK_TRUE, UINT64_MAX);
        if (r != vk::Result::eSuccess)
        {
            Logger::Warning("IcarianEngine: Could not wait for fence");

            return false;
        }
    }
    
    if (m_window->IsHeadless())
    {
        if (size != m_size)
        {
            device.waitIdle();
            
            Destroy();
            InitHeadless(size);

            uint32_t width = (uint32_t)size.x;
            uint32_t height = (uint32_t)size.y;

            void* args[] =
            {
                &width,
                &height
            };

            m_resizeFunc->Exec(args);   
        }

        *a_imageIndex = (*a_imageIndex + 1) % VulkanMaxFlightFrames;
        
        if ((m_init & (0b1 << *a_imageIndex)) == 0)
        {
            return true;
        }

        if (m_lastCmd[*a_imageIndex] != vk::CommandBuffer(nullptr))
        {
            m_engine->DestroyCommandBuffer(m_lastCmd[*a_imageIndex]);
            m_lastCmd[*a_imageIndex] = nullptr;
        }

        char* dat;
        if (vmaMapMemory(allocator, m_allocBuffer, (void**)&dat) != VK_SUCCESS)
        {
            Logger::Error("Failed mapping frame buffer");

            assert(0);
        }

        HeadlessAppWindow* window = (HeadlessAppWindow*)m_window;
        window->PushFrameData((uint32_t)m_size.x, (uint32_t)m_size.y, dat, a_delta, a_time);

        vmaUnmapMemory(allocator, m_allocBuffer);
    }
    else
    {
        switch (device.acquireNextImageKHR(m_swapchain, UINT64_MAX, a_semaphore, nullptr, a_imageIndex))
        {
        case vk::Result::eErrorOutOfDateKHR:
        {
            Destroy();

            return false;
        }
        case vk::Result::eSuccess:
        case vk::Result::eSuboptimalKHR:
        {
            if (size != m_size && size.x > 0 && size.y > 0)
            {
                Destroy();
                Init(size);

                uint32_t width = (uint32_t)size.x;
                uint32_t height = (uint32_t)size.y;

                void* args[] =
                {
                    &width,
                    &height
                };

                m_resizeFunc->Exec(args);   
            }

            break;
        }
        default:
        {
            Logger::Error("Failed to aquire swapchain image");

            assert(0);

            break;
        }
        }
    }

    {
        PROFILESTACK("Reset Fence");
        
        ICARIAN_ASSERT_R(device.resetFences(1, &a_fence) == vk::Result::eSuccess);
    }

    return true;
}
void VulkanSwapchain::EndFrame(const vk::Semaphore& a_semaphore, const vk::Fence& a_fence, uint32_t a_imageIndex)
{
    const vk::Device device = m_engine->GetLogicalDevice();
    const vk::Queue presentQueue = m_engine->GetPresentQueue();
    const vk::Queue graphicsQueue = m_engine->GetGraphicsQueue();

    if (m_window->IsHeadless())
    {        
        if ((m_init & 0b1 << a_imageIndex) == 0)
        {
            m_init |= 0b1 << a_imageIndex;

            return;
        }
        
        const vk::CommandBuffer cmdBuffer = m_engine->CreateCommandBuffer(vk::CommandBufferLevel::ePrimary);

        constexpr vk::CommandBufferBeginInfo BufferBeginInfo = vk::CommandBufferBeginInfo
        (
            vk::CommandBufferUsageFlagBits::eOneTimeSubmit
        );
        std::ignore = cmdBuffer.begin(&BufferBeginInfo);
        
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
            { (uint32_t)m_size.x, (uint32_t)m_size.y, 1 }
        );

        cmdBuffer.copyImageToBuffer(m_colorImage[a_imageIndex], vk::ImageLayout::eTransferSrcOptimal, m_buffer, 1, &imageCopy);

        cmdBuffer.end();

        constexpr vk::PipelineStageFlags WaitStages[] = { vk::PipelineStageFlagBits::eColorAttachmentOutput };

        const vk::SubmitInfo submitInfo = vk::SubmitInfo
        (
            1, 
            &a_semaphore, 
            WaitStages,
            1, 
            &cmdBuffer
        );
        {
            const std::unique_lock l = std::unique_lock(m_engine->GetGraphicsQueueMutex());

            if (graphicsQueue.submit(1, &submitInfo, a_fence) != vk::Result::eSuccess)
            {
                Logger::Error("Failed to submit swap copy");
            }
        }   

        m_lastCmd[a_imageIndex] = cmdBuffer;
    }
    else
    {
        const vk::SwapchainKHR swapChains[] = { m_swapchain };

        const vk::PresentInfoKHR presentInfo = vk::PresentInfoKHR
        (
            1,
            &a_semaphore,
            1,
            swapChains,
            &a_imageIndex
        );

        if (presentQueue.presentKHR(&presentInfo) != vk::Result::eSuccess)
        {
            Logger::Error("Failed to present swapchain");
        }
    }
}