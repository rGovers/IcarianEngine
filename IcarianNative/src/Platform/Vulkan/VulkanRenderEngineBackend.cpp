// Icarian Engine - C# Game Engine
// 
// License at end of file.

#ifdef ICARIANNATIVE_ENABLE_GRAPHICS_VULKAN

#include "Rendering/Vulkan/VulkanRenderEngineBackend.h"

#include <set>

#include "AppWindow/AppWindow.h"
#include "Config.h"
#include "Core/IcarianAssert.h"
#include "Core/IcarianDefer.h"
#include "Logger.h"
#include "Profiler.h"
#include "Rendering/RenderEngine.h"
#include "Rendering/Vulkan/LibVulkan.h"
#include "Rendering/Vulkan/VulkanCommandBuffer.h"
#include "Rendering/Vulkan/VulkanComputeEngine.h"
#include "Rendering/Vulkan/VulkanGraphicsEngine.h"
#include "Rendering/Vulkan/VulkanPushPool.h"
#include "Rendering/Vulkan/VulkanSwapchain.h"
#include "Runtime/RuntimeManager.h"
#include "Trace.h"

#ifndef NDEBUG
// #define VMA_DEBUG_LOG(str) Logger::Message(str)
// #define VMA_DEBUG_LOG_FORMAT(format, ...) do { char buffer[4096]; sprintf(buffer, format, __VA_ARGS__); Logger::Message(buffer); } while (0)
#endif

#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>

constexpr const char* ValidationLayers[] = 
{
    "VK_LAYER_KHRONOS_validation"
};

constexpr const char* InstanceExtensions[] = 
{
    
};
constexpr const char* DeviceExtensions[] = 
{
    VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME
};

constexpr const char* StandaloneDeviceExtensions[] =
{
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

constexpr const char* OptionalDeviceExtensions[] = 
{
    VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME,
    VK_KHR_VIDEO_QUEUE_EXTENSION_NAME,
    VK_KHR_VIDEO_DECODE_QUEUE_EXTENSION_NAME,
    VK_KHR_VIDEO_DECODE_H264_EXTENSION_NAME,

    // Seem to be having weirdness with Nvidia on Linux where I am getting a driver error but the Validation layer is quiet about this and it works despite throwing an error?
    // I am disabling because I do not know why it is not happy and not sure why it is throwing an error with OpCopyLogical
    // Need to know for certain what is happening before I enable it do not need it at this point in time
    // This is the first occurance that I got a error from the driver without a crash so I am a bit wary I am not used to graceful failures with Vulkan normally starts kicking and screaming
    // VK_KHR_SHADER_FLOAT_CONTROLS_EXTENSION_NAME,
    // VK_KHR_SPIRV_1_4_EXTENSION_NAME,
};
constexpr uint32_t OptionalDeviceExtensionCount = sizeof(OptionalDeviceExtensions) / sizeof(*OptionalDeviceExtensions);

constexpr static uint64_t MakeDeviceID(uint32_t a_vendorID, uint32_t a_deviceID)
{
    return (uint64_t)a_vendorID | (uint64_t)a_deviceID << 31;
}

static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT a_msgSeverity, VkDebugUtilsMessageTypeFlagsEXT a_msgType, const VkDebugUtilsMessengerCallbackDataEXT* a_callbackData, void* a_userData)
{
    constexpr static const char* ValidationPrefix = "Vulkan Validation Layer: ";

    switch (a_msgSeverity)
    {
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
    {
        Logger::Message(std::string(ValidationPrefix) + a_callbackData->pMessage);

        break;
    }
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
    {
        Logger::Warning(std::string(ValidationPrefix) + a_callbackData->pMessage);

        break;
    }
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
    {
        Logger::Error(std::string(ValidationPrefix) + a_callbackData->pMessage);

        // return VK_TRUE;
        break;
    }
    default:
    {
        break;
    }
    }

    return VK_FALSE;
}

static Array<bool> GetDeviceExtensionSupport(const vk::PhysicalDevice& a_device, const Array<const char*>& a_extensions)
{
    const uint32_t size = a_extensions.Size();

    Array<bool> mask;
    // Array zeros memory so defaults to false
    mask.Resize(size);

    uint32_t extensionCount;
    VKRESERR(a_device.enumerateDeviceExtensionProperties(nullptr, &extensionCount, nullptr));

    vk::ExtensionProperties* availableExtensions = new vk::ExtensionProperties[extensionCount];
    IDEFER(delete[] availableExtensions);
    VKRESERR(a_device.enumerateDeviceExtensionProperties(nullptr, &extensionCount, availableExtensions));

    for (uint32_t i = 0; i < size; ++i)
    {
        for (uint32_t j = 0; j < extensionCount; ++j)
        {
            if (strcmp(a_extensions[i], availableExtensions[j].extensionName) == 0)
            {
                mask[i] = true;

                break;
            }
        }
    }

    return mask;
}

static bool CheckDeviceExtensionSupport(const vk::PhysicalDevice& a_device, const Array<const char*>& a_extensions)
{
    const Array<bool> support = GetDeviceExtensionSupport(a_device, a_extensions);

    for (const bool s : support)
    {
        if (!s)
        {
            return false;
        }
    }

    return true;
}
static uint32_t GetDeviceExtensionScore(const vk::PhysicalDevice& a_device)
{
    const Array<bool> support = GetDeviceExtensionSupport(a_device, Array<const char*>(OptionalDeviceExtensions, OptionalDeviceExtensionCount));

    uint32_t score = 0;
    for (const bool s : support)
    {
        score += s * 20;
    }

    return score;
}

static bool IsDeviceSuitable(const vk::Instance& a_instance, const vk::PhysicalDevice& a_device, const Array<const char*>& a_extensions, AppWindow* a_window)
{
    constexpr uint32_t VersionMajor = vk::apiVersionMajor(ICARIAN_VULKAN_VERSION);
    constexpr uint32_t VersionMinor = vk::apiVersionMinor(ICARIAN_VULKAN_VERSION);
    
    const vk::PhysicalDeviceProperties properties = a_device.getProperties();

    const uint32_t deviceMajorVersion = vk::apiVersionMajor(properties.apiVersion);

    if (deviceMajorVersion == VersionMajor)
    {
        const uint32_t deviceMinorVersion = vk::apiVersionMinor(properties.apiVersion);

        if (deviceMinorVersion < VersionMinor)
        {
            return false;
        }
    }
    else if (deviceMajorVersion < VersionMajor)
    {
        return false;
    }

    uint64_t memTotal = 0;

    // Ignore devices with less then 1/4 GiB of memory
    // TODO: Have to change down the line but for now too much of a headache to deal with
    const vk::PhysicalDeviceMemoryProperties memProp = a_device.getMemoryProperties();
    for (uint32_t i = 0; i < memProp.memoryHeapCount; ++i)
    {
        const vk::MemoryHeap heap = memProp.memoryHeaps[i];

        if (heap.flags & vk::MemoryHeapFlagBits::eDeviceLocal)
        {
            memTotal += heap.size;
        }
    }
    
    if (memTotal <= (0b1 << 28))
    {
        return false;
    }

    if (!CheckDeviceExtensionSupport(a_device, a_extensions))
    {
        return false;
    }

    if (!a_window->IsHeadless())
    {
        const SwapChainSupportInfo info = VulkanSwapchain::QuerySwapChainSupport(a_device, a_window->GetSurface(a_instance));
        if (info.Formats.empty() || info.PresentModes.empty())
        {
            return false;
        }
    }

    const vk::PhysicalDeviceFeatures features = a_device.getFeatures();

    return features.samplerAnisotropy;
}

static uint32_t GetDeviceScore(const vk::PhysicalDevice& a_device)
{
    uint32_t score = 0;

    const vk::PhysicalDeviceProperties properties = a_device.getProperties();
    // Weighting the score
    // While there are situations that one type can be better then the other generally in this order
    switch (properties.deviceType) 
    {
    case vk::PhysicalDeviceType::eDiscreteGpu:
    {
        score += 200;

        break;
    }
    case vk::PhysicalDeviceType::eIntegratedGpu:
    {
        score += 100;

        break;
    }
    // Not really a good way to determine which is better so weight the same
    case vk::PhysicalDeviceType::eCpu:
    case vk::PhysicalDeviceType::eVirtualGpu:
    {
        score += 50;

        break;
    }
    // Unknown so no weighting
    default:
    {
        break;
    }
    }

    score += vk::apiVersionMajor(properties.apiVersion) * 100;
    score += vk::apiVersionMinor(properties.apiVersion) * 10;

    score += GetDeviceExtensionScore(a_device);

    const vk::PhysicalDeviceMemoryProperties memProp = a_device.getMemoryProperties();
    for (uint32_t i = 0; i < memProp.memoryHeapCount; ++i)
    {
        const vk::MemoryHeap heap = memProp.memoryHeaps[i];

        if (heap.flags & vk::MemoryHeapFlagBits::eDeviceLocal)
        {
            // Rate the device by the 1/4 GiB of memory
            const uint32_t memScore = (uint32_t)(heap.size / (0b1 << 28));

            score += memScore;
        }
    }

    return score;
}

static bool CheckValidationLayerSupport()
{
    uint32_t layerCount = 0;
    VKRESERR(vk::enumerateInstanceLayerProperties(&layerCount, nullptr));

    vk::LayerProperties* availableLayers = new vk::LayerProperties[layerCount];
    IDEFER(delete[] availableLayers);
    VKRESERR(vk::enumerateInstanceLayerProperties(&layerCount, availableLayers));

    for (const char* layerName : ValidationLayers)
    {
        for (uint32_t i = 0; i < layerCount; ++i)
        {
            if (strcmp(layerName, availableLayers[i].layerName) == 0)
            {
                goto NextIter;
            }
        }

        return false;

NextIter:;
    }

    return true;
} 

static Array<const char*> GetRequiredExtensions(const AppWindow* a_window)
{
    Array<const char*> extensions = a_window->GetRequiredVulkanExtenions();

    if constexpr (VulkanEnableValidationLayers)
    {
        extensions.Push(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    for (const char* ext : InstanceExtensions)
    {
        extensions.Push(ext);
    }

    return extensions;
}

#ifdef VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE
VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE
#endif

VulkanRenderEngineBackend::VulkanRenderEngineBackend(RenderEngine* a_engine) : RenderEngineBackend(a_engine)
{
    m_vulkanLib = new LibVulkan();

    VULKAN_HPP_DEFAULT_DISPATCHER.init((PFN_vkGetInstanceProcAddr)m_vulkanLib->vkGetInstanceProcAddr);

    const RenderEngine* renderEngine = GetRenderEngine();
    AppWindow* window = renderEngine->m_window;

    Array<const char*> enabledLayers;

    const bool headless = window->IsHeadless();

    if constexpr (VulkanEnableValidationLayers)
    {
        ICARIAN_ASSERT_R(CheckValidationLayerSupport());

        for (const char* v : ValidationLayers)
        {
            enabledLayers.Push(v);
        }
    }

    const std::string applicationName = renderEngine->m_config->GetApplicationName();

    const vk::ApplicationInfo appInfo = vk::ApplicationInfo
    (
        applicationName.c_str(), 
        0U, 
        "IcarianEngine", 
        VK_MAKE_API_VERSION(0, ICARIANNATIVE_VERSION_MAJOR, ICARIANNATIVE_VERSION_MINOR, ICARIANNATIVE_VERSION_PATCH),
        ICARIAN_VULKAN_VERSION, 
        nullptr
    );

    const Array<const char*> reqExtensions = GetRequiredExtensions(window);

    constexpr vk::DebugUtilsMessengerCreateInfoEXT DebugCreateInfo = vk::DebugUtilsMessengerCreateInfoEXT
    (
        vk::DebugUtilsMessengerCreateFlagsEXT(), 
        vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose | vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning | vk::DebugUtilsMessageSeverityFlagBitsEXT::eError,
        vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance,
        DebugCallback
    );

    vk::InstanceCreateInfo createInfo = vk::InstanceCreateInfo
    (
        vk::InstanceCreateFlags(),
        &appInfo, 
        enabledLayers.Size(), 
        enabledLayers.Data(), 
        reqExtensions.Size(), 
        reqExtensions.Data(),
        nullptr
    );

    if constexpr (VulkanEnableValidationLayers)
    {
        createInfo.pNext = &DebugCreateInfo;
    }

    TRACE("Creating Vulkan Instance");
    VKRESERRMSG(vk::createInstance(&createInfo, nullptr, &m_instance), "Failed to create Vulkan Instance");
    VULKAN_HPP_DEFAULT_DISPATCHER.init(m_instance);

    if constexpr (VulkanEnableValidationLayers)
    {
        TRACE("Creating Vulkan Debug Layer");
        VKRESERRMSG(m_instance.createDebugUtilsMessengerEXT(&DebugCreateInfo, nullptr, &m_messenger), "Failed to create Vulkan Debug Printing");
    }

    Array<const char*> extensions = Array<const char*>(DeviceExtensions, sizeof(DeviceExtensions) / sizeof(*DeviceExtensions));
    if (!headless)
    {
        for (const char* ext : StandaloneDeviceExtensions)
        {
            extensions.Push(ext);
        }
    }

    uint32_t deviceCount = 0;
    VKRESERR(m_instance.enumeratePhysicalDevices(&deviceCount, nullptr));

    IVERIFY(deviceCount > 0);

    // TODO: Should probably skip device selection if the user specifies a override
    vk::PhysicalDevice* devices = new vk::PhysicalDevice[deviceCount];
    IDEFER(delete[] devices);
    VKRESERR(m_instance.enumeratePhysicalDevices(&deviceCount, devices));

    uint32_t deviceScore = -1;
    m_pDevice = vk::PhysicalDevice(nullptr);

    for (uint32_t i = 0; i < deviceCount; ++i)
    {
        const vk::PhysicalDevice device = devices[i]; 

        if (IsDeviceSuitable(m_instance, device, extensions, window))
        {
            const uint32_t score = GetDeviceScore(device);
            if (score < deviceScore && deviceScore != -1)
            {
                continue;
            }

            deviceScore = score;
            m_pDevice = device;
        }
    }

    if (m_pDevice == vk::PhysicalDevice(nullptr))
    {
        IcarianError
        (
"No suitable GPU found to run. \
\
Please ensure you have a Vulkan 1.1 capable GPU with greater then 256MB of VRAM"
        );
    }

    TRACE("Found Vulkan Physical Device");

    m_optionalExtensionMask = GetDeviceExtensionSupport(m_pDevice, Array<const char*>(OptionalDeviceExtensions, OptionalDeviceExtensionCount));
    for (uint32_t i = 0; i < OptionalDeviceExtensionCount; ++i)
    {
        if (m_optionalExtensionMask[i])
        {
            extensions.Push(OptionalDeviceExtensions[i]);
        }
    }

    vk::PhysicalDeviceProperties props;
    m_pDevice.getProperties(&props);

    const uint64_t id = MakeDeviceID(props.vendorID, props.deviceID);

    constexpr uint32_t AMDVendorID = 0x1002;

    if constexpr (AMDDebuggerFix)
    {
        m_graphicsQueueIndex = 0;
        m_computeQueueIndex = 0;
        m_presentQueueIndex = 0;
    }
    else
    {
        switch (id) 
        {
        // Bug specifically with AMD Polaris cards that we are working around
        // Checking specifically if we are running on a Polaris GPU
        // If we do not do this get black lines running down the screen when we go fullscreen
        // They take a performance hit but it is better then the alternative
        // RX 460/ Pro 560X 
        case MakeDeviceID(AMDVendorID, 0x67EF):
        // RX 550/550X
        case MakeDeviceID(AMDVendorID, 0x699F):
        // RX 560
        case MakeDeviceID(AMDVendorID, 0x67FF):
        // RX 470/480/570/580/590/590GME/ Pro 580
        case MakeDeviceID(AMDVendorID, 0x67DF):
        {
            m_graphicsQueueIndex = 0;
            m_computeQueueIndex = 0;
            m_presentQueueIndex = 0;

            break;
        }
        default:
        {
            uint32_t queueFamilyCount = 0;
            m_pDevice.getQueueFamilyProperties(&queueFamilyCount, nullptr);

            vk::QueueFamilyProperties* queueFamilies = new vk::QueueFamilyProperties[queueFamilyCount];
            IDEFER(delete[] queueFamilies);
            m_pDevice.getQueueFamilyProperties(&queueFamilyCount, queueFamilies);

            for (uint32_t i = 0; i < queueFamilyCount; ++i)
            {
                if (queueFamilies[i].queueFlags & vk::QueueFlagBits::eGraphics)
                {
                    m_graphicsQueueIndex = i;
                }

                if (queueFamilies[i].queueFlags & vk::QueueFlagBits::eVideoDecodeKHR)
                {
                    m_videoDecodeQueueIndex = i;
                }

                if (!headless)
                {
                    vk::Bool32 presentSupport = VK_FALSE;
                    VKRESERR(m_pDevice.getSurfaceSupportKHR(i, window->GetSurface(m_instance), &presentSupport));

                    if (presentSupport)
                    {
                        // Want graphics queue to be last resort
                        if (i == m_graphicsQueueIndex)
                        {
                            if (m_presentQueueIndex == -1)
                            {
                                m_presentQueueIndex = i;
                            }
                        }
                        else
                        {
                            m_presentQueueIndex = i;
                        }
                    }
                }

                if (queueFamilies[i].queueFlags & vk::QueueFlagBits::eCompute)
                {
                    // Want present queue to be last resort
                    // Have it wanting to use the present queue on NVIDIA cards so this is needed
                    if (i == m_presentQueueIndex)
                    {
                        if (m_computeQueueIndex == -1)
                        {
                            m_computeQueueIndex = i;
                        }
                    }
                    else
                    {
                        m_computeQueueIndex = i;
                    }
                }
            }

            break;
        }
        }
    }

    std::set<uint32_t> uniqueQueueFamilies;
    if (m_computeQueueIndex != -1)
    {
        uniqueQueueFamilies.emplace(m_computeQueueIndex);
    }
    if (m_videoDecodeQueueIndex != -1)
    {
        uniqueQueueFamilies.emplace(m_videoDecodeQueueIndex);
    }
    if (m_graphicsQueueIndex != -1)
    {
        uniqueQueueFamilies.emplace(m_graphicsQueueIndex);
    }
    if (m_presentQueueIndex != -1)
    {
        uniqueQueueFamilies.emplace(m_presentQueueIndex);
    }

    IVERIFY(!uniqueQueueFamilies.empty());

    Array<vk::DeviceQueueCreateInfo> queueCreateInfos;

    constexpr float QueuePriority = 1.0f;
    for (const uint32_t queueFamily : uniqueQueueFamilies)
    {
        queueCreateInfos.Push(vk::DeviceQueueCreateInfo({ }, queueFamily, 1, &QueuePriority));
    }

    vk::PhysicalDeviceFeatures deviceFeatures;
    deviceFeatures.samplerAnisotropy = VK_TRUE;

    vk::DeviceCreateInfo deviceCreateInfo = vk::DeviceCreateInfo
    (
        { }, 
        queueCreateInfos.Size(), 
        queueCreateInfos.Data(), 
        0, 
        nullptr, 
        extensions.Size(), 
        extensions.Data(),
        &deviceFeatures
    );

    if constexpr (VulkanEnableValidationLayers)
    {
        deviceCreateInfo.enabledLayerCount = sizeof(ValidationLayers) / sizeof(*ValidationLayers);
        deviceCreateInfo.ppEnabledLayerNames = ValidationLayers;
    }

    TRACE("Creating Vulkan Device");
    VKRESERRMSG(m_pDevice.createDevice(&deviceCreateInfo, nullptr, &m_lDevice), "Failed to create Vulkan Logic Device");
    VULKAN_HPP_DEFAULT_DISPATCHER.init(m_lDevice);

    const VmaVulkanFunctions vulkanFunctions = 
    {
        .vkGetInstanceProcAddr = (PFN_vkGetInstanceProcAddr)m_vulkanLib->vkGetInstanceProcAddr,
        .vkGetDeviceProcAddr = (PFN_vkGetDeviceProcAddr)m_vulkanLib->vkGetDeviceProcAddr
    };

    const VmaAllocatorCreateInfo allocatorCreateInfo =
    {
        .physicalDevice = m_pDevice,
        .device = m_lDevice,
        .pVulkanFunctions = &vulkanFunctions,
        .instance = m_instance,
        .vulkanApiVersion = ICARIAN_VULKAN_VERSION
    };

    TRACE("Creating Vulkan Allocator");
    VKRESERRMSG(vmaCreateAllocator(&allocatorCreateInfo, &m_allocator), "Failed to create Vulkan Allocator");

    // By what I can tell most devices use the same queue for graphics and compute this is for correctness shold not affect much
    // Not fussed if it shares with graphics as long as it is not the present queue
    if (m_computeQueueIndex != -1)
    {
        m_lDevice.getQueue(m_computeQueueIndex, 0, &m_computeQueue);
    }
    if (m_videoDecodeQueueIndex != -1)
    {
        m_lDevice.getQueue(m_videoDecodeQueueIndex, 0, &m_videoDecodeQueue);
    }
    if (m_graphicsQueueIndex != -1)
    {
        m_lDevice.getQueue(m_graphicsQueueIndex, 0, &m_graphicsQueue);    
    }
    if (m_presentQueueIndex != -1)
    {
        m_lDevice.getQueue(m_presentQueueIndex, 0, &m_presentQueue);
    }

    TRACE("Got Vulkan Queues");

    constexpr vk::SemaphoreCreateInfo SemaphoreInfo;
    constexpr vk::FenceCreateInfo FenceInfo = vk::FenceCreateInfo
    (
        vk::FenceCreateFlagBits::eSignaled
    );

    TRACE("Creating Vulkan sync objects");
    for (uint32_t i = 0; i < VulkanMaxFlightFrames; ++i)
    {
        VKRESERRMSG(m_lDevice.createSemaphore(&SemaphoreInfo, nullptr, &m_imageAvailable[i]), "Failed to create image semaphore");
        VKRESERRMSG(m_lDevice.createFence(&FenceInfo, nullptr, &m_inFlight[i]), "Failed to create fence");
    }
    
    const vk::CommandPoolCreateInfo poolInfo = vk::CommandPoolCreateInfo
    (
        vk::CommandPoolCreateFlagBits::eTransient,
        m_graphicsQueueIndex
    );  

    VKRESERRMSG(m_lDevice.createCommandPool(&poolInfo, nullptr, &m_commandPool), "Failed to create command pool");

    if (IsExtensionEnabled(VK_KHR_VIDEO_DECODE_H264_EXTENSION_NAME))
    {
        m_videoDecodeCapabilities.VideoProfile = vk::VideoProfileInfoKHR
        (
            vk::VideoCodecOperationFlagBitsKHR::eDecodeH264,
            vk::VideoChromaSubsamplingFlagBitsKHR::e420,
            vk::VideoComponentBitDepthFlagBitsKHR::e8,
            vk::VideoComponentBitDepthFlagBitsKHR::e8,
            &DecodeProfile
        );

        m_videoDecodeCapabilities.VideoCapabilities.pNext = &m_videoDecodeCapabilities.DecodeCapabilities;
        m_videoDecodeCapabilities.DecodeCapabilities.pNext = &m_videoDecodeCapabilities.DecodeH264Capabilities;

        VKRESERR(m_pDevice.getVideoCapabilitiesKHR(&m_videoDecodeCapabilities.VideoProfile, &m_videoDecodeCapabilities.VideoCapabilities));
    }

    m_pushPool = new VulkanPushPool(this);
    m_computeEngine = new VulkanComputeEngine(this);
    m_graphicsEngine = new VulkanGraphicsEngine(this);
}
VulkanRenderEngineBackend::~VulkanRenderEngineBackend()
{
    AppWindow* window = GetRenderEngine()->m_window;

    TRACE("Begin Vulkan clean up");
    m_lDevice.waitIdle();

    delete m_computeEngine;
    delete m_pushPool;

    m_graphicsEngine->Cleanup();

    TRACE("Destroy Vulkan Deletion Objects");
    for (uint32_t i = 0; i < VulkanDeletionQueueSize; ++i)
    {
        const TLockArray a = m_deletionObjects[i].ToLockArray();

        for (VulkanDeletionObject* obj : a)
        {
            if (obj != nullptr)
            {
                obj->Destroy();

                delete obj;
            }
        }

        m_deletionObjects[i].UClear();
    }

    delete m_graphicsEngine;

    TRACE("Destroy Command Pool");
    m_lDevice.destroyCommandPool(m_commandPool);

    if (m_swapchain != nullptr)
    {
        delete m_swapchain;
        m_swapchain = nullptr;
    }

    TRACE("Destroy Vulkan Sync Objects");
    for (uint32_t i = 0; i < VulkanMaxFlightFrames; ++i)
    {
        m_lDevice.destroySemaphore(m_imageAvailable[i]);
        m_lDevice.destroyFence(m_inFlight[i]);

        for (uint32_t j = 0; j < m_interSemaphore[i].Size(); ++j)
        {
            m_lDevice.destroySemaphore(m_interSemaphore[i][j]);
        }
    }

    TRACE("Destroy Vulkan Allocator");
    vmaDestroyAllocator(m_allocator);
    
    vk::SurfaceKHR surface = window->GetSurface(m_instance);
    if (surface != vk::SurfaceKHR(nullptr))
    {
        TRACE("Destroying Surface");
        m_instance.destroySurfaceKHR(surface);
    }

    TRACE("Destroying Devices");
    m_lDevice.destroy();

    if constexpr (VulkanEnableValidationLayers)
    {
        TRACE("Cleaning Vulkan Diagnostics");
        m_instance.destroyDebugUtilsMessengerEXT(m_messenger);
    }

    TRACE("Destroying Vulkan Instance");
    m_instance.destroy();

    delete m_vulkanLib;

    TRACE("Vulkan cleaned up");
}

bool VulkanRenderEngineBackend::IsExtensionEnabled(const std::string_view& a_extension) const
{
    for (uint32_t i = 0; i < OptionalDeviceExtensionCount; ++i)
    {
        if (a_extension == OptionalDeviceExtensions[i])
        {
            return m_optionalExtensionMask[i];
        }
    }

    return false;
}

void VulkanRenderEngineBackend::Update(double a_delta, double a_time)
{
    // TODO: Bump DMA buffers up in priority to allow GPU->GPU memory sharing between processes instead of GPU->CPU->GPU. 
    // RAM clock now effects even the linux build also probably want to improve locality of rendering data to allow more efficient use of the cache instead of RAM.
    // Also investigate seeing if you can squezee some extra frames from a buffer scavenging system for GPU memory. A little bit of wasted memory for a few extra frames is probably worth it.
    // Constant allocation is killing performance on the GPU side. 
    // TODO: Can probably better manage semaphores.
    const RenderEngine* renderEngine = GetRenderEngine();
    AppWindow* window = renderEngine->m_window;

    const bool isHeadless = window->IsHeadless();
    const bool init = m_swapchain != nullptr;

    {
        PROFILESTACK("Swap Setup");

        if (!init)
        {
            m_swapchain = new VulkanSwapchain(this, window);
            m_graphicsEngine->SetSwapchain(m_swapchain);
        }

        if (!m_swapchain->StartFrame(&m_imageIndex, a_delta, a_time))
        {
            return;
        }
    }
    
    Profiler::StartFrame("Render Update");

    m_pushPool->Reset(m_currentFrame);

    Array<VulkanCommandBuffer> commandBuffers;

    // TODO: Down the line setup the compute and graphics engine to return VulkanCommandBuffers
    const VulkanCommandBuffer computeCommandBuffer = m_computeEngine->Update(a_delta, a_time, m_currentFrame);
    commandBuffers.Push(computeCommandBuffer);

    {
        const Array<VulkanCommandBuffer> buffers = m_graphicsEngine->Update(a_delta, a_time, m_currentFrame);
        for (const VulkanCommandBuffer& b : buffers)
        {
            commandBuffers.Push(b);
        }
    }
    
    Profiler::StartFrame("Render Setup");

    const uint32_t buffersSize = commandBuffers.Size();

    const uint32_t semaphoreCount = m_interSemaphore[m_currentFlightFrame].Size();
    const uint32_t endBuffer = buffersSize - 1;

    if (buffersSize > semaphoreCount)
    {
        TRACE("Allocating inter semaphores");
        const uint32_t diff = buffersSize - semaphoreCount;

        constexpr vk::SemaphoreCreateInfo SemaphoreInfo;

        for (uint32_t i = 0; i < diff; ++i)
        {
            vk::Semaphore semaphore;

            VKRESERRMSG(m_lDevice.createSemaphore(&SemaphoreInfo, nullptr, &semaphore), "Failed to create inter semaphore");
            m_interSemaphore[m_currentFlightFrame].Push(semaphore);
        }
    }

    Profiler::StopFrame();

    Profiler::StartFrame("Render Submit");

    vk::Semaphore lastSemaphore = nullptr;
    if (isHeadless)
    {
        if (init)
        {
            lastSemaphore = m_imageAvailable[m_currentFlightFrame];
        }
    }
    else
    {
        lastSemaphore = m_imageAvailable[m_currentFlightFrame];
    }

    {
        // TODO: Redo command buffer submission, can probably get benefits from allowing GPUs with multli queue to do stuff at the same time.
        // Also just generally a bit of a mess
        // Alot of benefit comes from allowing video at the same time.
        // Not a lot of benefit atleast for the GPUs I have compute and graphics is the same queue and it they do have seperate queues compute is shared with the present queue annoyingly
        // May consider allowing present and compute to share need to do further investigation
        const ThreadGuard l = ThreadGuard(m_graphicsQueueLock);

        for (uint32_t i = 0; i < buffersSize; ++i)
        {
            const vk::Semaphore curSemaphore = m_interSemaphore[m_currentFlightFrame][i];
            IDEFER(lastSemaphore = curSemaphore);

            const VulkanCommandBuffer& buffer = commandBuffers[i];
            const vk::CommandBuffer cmdBuffer = buffer.GetCommandBuffer();
            
            vk::PipelineStageFlags waitStages;
            vk::Queue queue;

            switch (buffer.GetBufferType())
            {
            case VulkanCommandBufferType_Compute:
            {
                queue = m_computeQueue;
                waitStages = vk::PipelineStageFlagBits::eComputeShader;

                break;
            }
            case VulkanCommandBufferType_VideoDecode:
            {
                queue = m_videoDecodeQueue;
                waitStages = vk::PipelineStageFlagBits::eAllCommands;

                break;
            }
            case VulkanCommandBufferType_Graphics:
            {
                queue = m_graphicsQueue;
                waitStages = vk::PipelineStageFlagBits::eAllGraphics;

                break;
            }
            default:
            {
                IERROR("Invalid command buffer type");

                break;
            }
            }

            vk::SubmitInfo submitInfo = vk::SubmitInfo
            (
                0,
                nullptr,
                &waitStages,
                1,
                &cmdBuffer,
                1,
                &curSemaphore
            );

            if (lastSemaphore != vk::Semaphore(nullptr))
            {
                submitInfo.waitSemaphoreCount = 1;
                submitInfo.pWaitSemaphores = &lastSemaphore;
            }

            if (i == endBuffer)
            {
                if (isHeadless)
                {
                    if (!m_swapchain->IsInitialized(m_imageIndex))
                    {
                        submitInfo.pSignalSemaphores = &m_imageAvailable[(m_currentFlightFrame + 1) % VulkanMaxFlightFrames];

                        VKRESERRMSG(queue.submit(1, &submitInfo, m_inFlight[m_currentFlightFrame]), "Failed to submit command");
                    }
                    else
                    {
                        VKRESERRMSG(queue.submit(1, &submitInfo, nullptr), "Failed to submit command");
                    }
                }
                else
                {
                    VKRESERRMSG(queue.submit(1, &submitInfo, m_inFlight[m_currentFlightFrame]), "Failed to submit command");
                }
            }
            else
            {
                VKRESERRMSG(queue.submit(1, &submitInfo, nullptr), "Failed to submit command");
            }
        }    
    }

    Profiler::StopFrame();

    Profiler::StopFrame();

    // I have this queue setup because resources may be used as they are being deleted
    // The proper way to do this seem to be doing elaborate semaphore/fence setups
    // Followed the KISS philosophy so just queue resources to be deleted on the next time round on the flight frame
    // Seems to be working fine without having to fuck around more with fences have not had a driver crash so far
    // Only problem is have to wait the number of flight frames for resources to be de-allocated
    {
        PROFILESTACK("Queue Cleanup");

        const uint32_t nextIndex = (m_dQueueIndex + 1) % VulkanDeletionQueueSize;
        IDEFER(m_dQueueIndex = nextIndex);

        const TLockArray a = m_deletionObjects[nextIndex].ToLockArray();

        for (VulkanDeletionObject* obj : a)
        {
            if (obj != nullptr)
            {
                obj->Destroy();

                delete obj;
            }
        }

        m_deletionObjects[nextIndex].UClear();
    }   

    {
        PROFILESTACK("Swap Present");

        m_swapchain->EndFrame(lastSemaphore, m_imageIndex);

        m_currentFrame = (m_currentFrame + 1) % VulkanFlightPoolSize;
        m_currentFlightFrame = (m_currentFlightFrame + 1) % VulkanMaxFlightFrames;
    }
}

class VulkanCommandBufferDeletionObject : public VulkanDeletionObject
{
private:
    vk::Device        m_device;
    vk::CommandPool   m_pool;
    vk::CommandBuffer m_buffer;

protected:

public:
    VulkanCommandBufferDeletionObject(vk::Device a_device, vk::CommandPool a_pool, vk::CommandBuffer a_buffer)
    {
        m_device = a_device;
        m_pool = a_pool;
        m_buffer = a_buffer;
    }
    virtual ~VulkanCommandBufferDeletionObject() 
    {
        
    }

    virtual void Destroy()
    {
        m_device.freeCommandBuffers(m_pool, 1, &m_buffer);
    }
};

// TODO: Down the line setup return VulkanCommandBuffers as TLockObj
TLockObj<vk::CommandBuffer, SpinLock>* VulkanRenderEngineBackend::CreateCommandBuffer(vk::CommandBufferLevel a_level)
{   
    const vk::CommandBufferAllocateInfo allocInfo = vk::CommandBufferAllocateInfo
    (
        m_commandPool,
        a_level,
        1
    );

    TLockObj<vk::CommandBuffer, SpinLock>* lockObj = new TLockObj<vk::CommandBuffer, SpinLock>(&m_graphicsQueueLock); 

    vk::CommandBuffer cmdBuffer;

    VKRESERRMSG(m_lDevice.allocateCommandBuffers(&allocInfo, &cmdBuffer), "Failed to Allocate Command Buffer");

    lockObj->Set(cmdBuffer);

    return lockObj;
}
void VulkanRenderEngineBackend::DestroyCommandBuffer(TLockObj<vk::CommandBuffer, SpinLock>* a_buffer)
{
    IDEFER(delete a_buffer);

    const vk::CommandBuffer buffer = a_buffer->Get();

    PushDeletionObject(new VulkanCommandBufferDeletionObject(m_lDevice, m_commandPool, buffer));
}

TLockObj<vk::CommandBuffer, SpinLock>* VulkanRenderEngineBackend::BeginSingleCommand()
{
    TLockObj<vk::CommandBuffer, SpinLock>* buffer = CreateCommandBuffer(vk::CommandBufferLevel::ePrimary);
    const vk::CommandBuffer cmdBuffer = buffer->Get();

    constexpr vk::CommandBufferBeginInfo BufferBeginInfo = vk::CommandBufferBeginInfo
    (
        vk::CommandBufferUsageFlagBits::eOneTimeSubmit
    );

    VKRESERR(cmdBuffer.begin(&BufferBeginInfo));

    return buffer;
}
void VulkanRenderEngineBackend::EndSingleCommand(TLockObj<vk::CommandBuffer, SpinLock>* a_buffer)
{
    IDEFER(DestroyCommandBuffer(a_buffer));
    
    const vk::CommandBuffer cmdBuffer = a_buffer->Get();
    cmdBuffer.end();

    const vk::SubmitInfo submitInfo = vk::SubmitInfo
    (
        0, 
        nullptr, 
        nullptr,
        1, 
        &cmdBuffer
    );

    VKRESERRMSG(m_graphicsQueue.submit(1, &submitInfo, nullptr), "Failed to Submit Command");
}

e_RenderDeviceType VulkanRenderEngineBackend::GetDeviceType() const
{
    vk::PhysicalDeviceProperties props;
    m_pDevice.getProperties(&props);

    switch (props.deviceType) 
    {
    case vk::PhysicalDeviceType::eDiscreteGpu:
    {
        return RenderDeviceType_DiscreteGPU;
    }
    case vk::PhysicalDeviceType::eIntegratedGpu:
    {
        return RenderDeviceType_IntergratedGPU;
    }
    case vk::PhysicalDeviceType::eCpu:
    case vk::PhysicalDeviceType::eVirtualGpu:
    {
        return RenderDeviceType_Software;
    }
    default:
    {
        break;
    }
    }

    return RenderDeviceType_Unknown;
}

uint64_t VulkanRenderEngineBackend::GetUsedDeviceMemory() const
{
    VmaBudget bugets[VK_MAX_MEMORY_HEAPS];
    vmaGetHeapBudgets(m_allocator, bugets);
    const VkPhysicalDeviceMemoryProperties* properties;
    vmaGetMemoryProperties(m_allocator, &properties);

    uint64_t used = 0;
    for (uint32_t i = 0; i < properties->memoryTypeCount; ++i)
    {
        const vk::MemoryType type = properties->memoryTypes[i];

        if (type.propertyFlags & vk::MemoryPropertyFlagBits::eDeviceLocal)
        {
            const VmaBudget& b = bugets[type.heapIndex];
            used += b.usage;
        }
    }

    return used;
}
uint64_t VulkanRenderEngineBackend::GetTotalDeviceMemory() const
{
    VmaBudget bugets[VK_MAX_MEMORY_HEAPS];
    vmaGetHeapBudgets(m_allocator, bugets);
    const VkPhysicalDeviceMemoryProperties* properties;
    vmaGetMemoryProperties(m_allocator, &properties);

    uint64_t used = 0;
    for (uint32_t i = 0; i < properties->memoryTypeCount; ++i)
    {
        const vk::MemoryType type = properties->memoryTypes[i];

        if (type.propertyFlags & vk::MemoryPropertyFlagBits::eDeviceLocal)
        {
            const VmaBudget& b = bugets[type.heapIndex];
            used += b.budget;
        }
    }

    return used;
}

uint32_t VulkanRenderEngineBackend::GenerateModel(const void* a_vertices, uint32_t a_vertexCount, uint16_t a_vertexStride, const uint32_t* a_indices, uint32_t a_indexCount, float a_radius)
{
    return m_graphicsEngine->GenerateModel(a_vertices, a_vertexCount, a_vertexStride, a_indices, a_indexCount, a_radius);
}
void VulkanRenderEngineBackend::DestroyModel(uint32_t a_addr)
{
    m_graphicsEngine->DestroyModel(a_addr);
}

uint32_t VulkanRenderEngineBackend::GenerateTexture(uint32_t a_width, uint32_t a_height, e_TextureFormat a_format, const void* a_data)
{
    return m_graphicsEngine->GenerateTexture(a_width, a_height, a_format, a_data);
}
uint32_t VulkanRenderEngineBackend::GenerateTextureMipMapped(uint32_t a_width, uint32_t a_height, uint32_t a_levels, uint64_t* a_offsets, e_TextureFormat a_format, const void* a_data, uint64_t a_dataSize)
{
    return m_graphicsEngine->GenerateMipMappedTexture(a_width, a_height, a_levels, a_offsets, a_format, a_data, a_dataSize);
}
void VulkanRenderEngineBackend::DestroyTexture(uint32_t a_addr)
{
    m_graphicsEngine->DestroyTexture(a_addr);
}

uint32_t VulkanRenderEngineBackend::GenerateTextureSampler(uint32_t a_textureAddr, e_TextureMode a_textureMode, e_TextureFilter a_filterMode, e_TextureAddress a_addressMode, uint32_t a_slot)
{
    return m_graphicsEngine->GenerateTextureSampler(a_textureAddr, a_textureMode, a_filterMode, a_addressMode, a_slot);
}
void VulkanRenderEngineBackend::DestroyTextureSampler(uint32_t a_addr)
{
    m_graphicsEngine->DestroyTextureSampler(a_addr);
}

void VulkanRenderEngineBackend::PushDeletionObject(VulkanDeletionObject* a_object)
{
    m_deletionObjects[m_dQueueIndex].Push(a_object);
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