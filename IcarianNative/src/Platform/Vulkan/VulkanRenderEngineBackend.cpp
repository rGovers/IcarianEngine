// Icarian Engine - C# Game Engine
// 
// License at end of file.

#ifdef ICARIANNATIVE_ENABLE_GRAPHICS_VULKAN

#include "Rendering/Vulkan/VulkanRenderEngineBackend.h"

#include "AppWindow/AppWindow.h"
#include "Config.h"
#include "Core/IcarianDefer.h"
#include "Core/IcarianLambda.h"
#include "Core/IcarianPragma.h"
#include "Core/StringUtils.h"
#include "DataTypes/Allocators/LeakAllocator.h"
#include "DataTypes/Allocators/MallocAllocator.h"
#include "DataTypes/Allocators/MultiSourceAllocator.h"
#include "DataTypes/Allocators/OSAllocator.h"
#include "DataTypes/Allocators/StackAllocator.h"
#include "DataTypes/Set.h"
#include "Logger.h"
#include "Profiler.h"
#include "Rendering/LibRenderDoc.h"
#include "Rendering/RenderEngine.h"
#include "Rendering/Vulkan/LibVulkan.h"
#include "Rendering/Vulkan/VulkanCommandBuffer.h"
#include "Rendering/Vulkan/VulkanComputeEngine.h"
#include "Rendering/Vulkan/VulkanGraphicsEngine.h"
#include "Rendering/Vulkan/VulkanPushPool.h"
#include "Rendering/Vulkan/VulkanSwapchain.h"
#include "Runtime/RuntimeManager.h"
#include "Trace.h"

// Now that I better understand memory allocators I am tempted to rip out VMA and implement my own
// Yes VMA can defrag but can have tighter integration if I roll my own
// Can also do less hacky stuff for DMA
// Not sure still if it is something I want to commit to but
// Upside if I roll my own I can make it graphics API agnostic so it can work with DX12 aswell possibly
// Not mine so not much I can do have to suppress the warnings in the library for it to compile
ICARIAN_WARNINGPUSH
ICARIAN_WARNINGSUPPRESS("-Wunused-variable")
#ifdef DEBUG
ICARIAN_WARNINGSUPPRESS("-Wformat");
// #define VMA_DEBUG_LOG(str) Logger::Message(str)
// #define VMA_DEBUG_LOG_FORMAT(format, ...) do { char buffer[4096]; sprintf(buffer, format, __VA_ARGS__); Logger::Message(buffer); } while (0)
#endif
#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>
ICARIAN_WARNINGPOP

constexpr const char* ValidationLayers[] = 
{
    "VK_LAYER_KHRONOS_validation"
};

constexpr const char* StandaloneDeviceExtensions[] =
{
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};
constexpr const char* HeadlessDeviceExtensions[] =
{
#ifdef ICARIANNATIVE_ENABLE_DMA
    VK_KHR_EXTERNAL_MEMORY_EXTENSION_NAME,
    VK_KHR_EXTERNAL_SEMAPHORE_EXTENSION_NAME,
#ifdef WIN32
    VK_KHR_EXTERNAL_MEMORY_WIN32_EXTENSION_NAME,
    VK_KHR_EXTERNAL_SEMAPHORE_WIN32_EXTENSION_NAME
#else
    VK_KHR_EXTERNAL_MEMORY_FD_EXTENSION_NAME,
    VK_KHR_EXTERNAL_SEMAPHORE_FD_EXTENSION_NAME,
#endif
#endif
};

constexpr const char* OptionalDeviceExtensions[] = 
{
    VK_EXT_MESH_SHADER_EXTENSION_NAME,
    VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME,
    VK_KHR_VIDEO_QUEUE_EXTENSION_NAME,
    VK_KHR_VIDEO_MAINTENANCE_1_EXTENSION_NAME,
    VK_KHR_VIDEO_DECODE_QUEUE_EXTENSION_NAME,
    VK_KHR_VIDEO_DECODE_H264_EXTENSION_NAME,

#ifdef ICARIANNATIVE_ENABLE_MARKERS
    VK_EXT_DEBUG_MARKER_EXTENSION_NAME
#endif
};
constexpr uint32_t OptionalDeviceExtensionCount = sizeof(OptionalDeviceExtensions) / sizeof(*OptionalDeviceExtensions);

constexpr struct OptionalHashes
{
    typedef uint32_t HashType;

    HashType Data[OptionalDeviceExtensionCount];

    constexpr OptionalHashes() : Data()
    {
        for (uint32_t i = 0; i < OptionalDeviceExtensionCount; ++i)
        {
            Data[i] = StringHash<HashType>(OptionalDeviceExtensions[i]);
        }
    }

} OptionalDeviceExtensionHashes;

static VulkanRenderEngineBackend* Instance = nullptr;

struct ScratchData
{
    uint32_t Index;
    StackAllocator* Allocator;
};

static TStatic<ScratchData> ScratchAllocator = TStatic<ScratchData>();

static void InitScratchData()
{
    if (!ScratchAllocator.Exists())
    {
        uint32_t index;
        StackAllocator* allocator = Instance->GetStackAllocator(&index);

        const ScratchData data =
        {
            .Index = index,
            .Allocator = allocator,
        };

        ScratchAllocator.Push(data);
    }
}

StackAllocator* RenderScratchAlloc::GetAllocator()
{
    InitScratchData();

    return ScratchAllocator->Allocator;
}

void* RenderScratchAlloc::Allocate(uint64_t a_value, uint64_t a_alignment)
{
    InitScratchData();

    return ScratchAllocator->Allocator->Allocate(a_value, a_alignment);
}
void RenderScratchAlloc::Free(void* a_ptr)
{

}
void RenderScratchAlloc::PushFrame()
{
    InitScratchData();

    Instance->IncrementScratchFrame(ScratchAllocator->Index);

    ScratchAllocator->Allocator->PushStackPointer();
}
void RenderScratchAlloc::PopFrame()
{
    IVERIFY(ScratchAllocator.Exists());

    ScratchAllocator->Allocator->PopStackPointer();

    Instance->DecrementScratchFrame(ScratchAllocator->Index);
}

void* RenderBlockAlloc::Allocate(uint64_t a_value, uint32_t a_alignment)
{
    Allocator* allocator = Instance->GetAllocator();

    return allocator->Allocate(a_value, a_alignment);
}
void RenderBlockAlloc::Free(void* a_ptr)
{
    Allocator* allocator = Instance->GetAllocator();

    allocator->Free(a_ptr);
}

constexpr static uint64_t MakeDeviceID(uint32_t a_vendorID, uint32_t a_deviceID)
{
    return (uint64_t)a_vendorID | (uint64_t)a_deviceID << 31;
}

static VKAPI_ATTR vk::Bool32 VKAPI_CALL DebugCallback
(
    vk::DebugUtilsMessageSeverityFlagBitsEXT a_msgSeverity,
    vk::DebugUtilsMessageTypeFlagsEXT a_msgType,
    const vk::DebugUtilsMessengerCallbackDataEXT* a_callbackData,
    void* a_userData
)
{
    constexpr static const char* ValidationPrefix = "Vulkan Validation Layer: ";

    switch (a_msgSeverity)
    {
    case vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo:
    {
        Logger::Message(std::string(ValidationPrefix) + a_callbackData->pMessage);

        break;
    }
    case vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning:
    {
        Logger::Warning(std::string(ValidationPrefix) + a_callbackData->pMessage);

        break;
    }
    case vk::DebugUtilsMessageSeverityFlagBitsEXT::eError:
    {
        Logger::Error(std::string(ValidationPrefix) + a_callbackData->pMessage);

        // return vk::True;
        break;
    }
    default:
    {
        break;
    }
    }

    return vk::False;
}

static Array<uint8_t> GetDeviceExtensionSupport(const vk::PhysicalDevice& a_device, const Array<const char*>& a_extensions, Allocator* a_allocator)
{
    const uint32_t size = a_extensions.Size();

    const uint32_t arraySize = (size / 8) + 1;

    Array<uint8_t> mask = Array<uint8_t>(a_allocator);
    // Array zeros memory so defaults to false
    mask.Resize(arraySize);

    uint32_t extensionCount;
    VKRESERR(a_device.enumerateDeviceExtensionProperties(nullptr, &extensionCount, nullptr));

    vk::ExtensionProperties* availableExtensions = a_allocator->TAllocate<vk::ExtensionProperties>(extensionCount);
    IDEFER(a_allocator->Free(availableExtensions));

    VKRESERR(a_device.enumerateDeviceExtensionProperties(nullptr, &extensionCount, availableExtensions));

    for (uint32_t i = 0; i < size; ++i)
    {
        for (uint32_t j = 0; j < extensionCount; ++j)
        {
            if (strcmp(a_extensions[i], availableExtensions[j].extensionName) == 0)
            {
                const uint32_t index = i / 8;
                const uint32_t offset = i % 8;

                ISETBIT(mask[index], offset);

                break;
            }
        }
    }

    return mask;
}

static bool CheckDeviceExtensionSupport(const vk::PhysicalDevice& a_device, const Array<const char*>& a_extensions, Allocator* a_tempAllocator)
{
    const uint32_t size = a_extensions.Size();

    const Array<uint8_t> support = GetDeviceExtensionSupport(a_device, a_extensions, a_tempAllocator);
    for (uint32_t i = 0; i < size; ++i)
    {
        const uint32_t index = i / 8;
        const uint32_t offset = i % 8;

        if (!IISBITSET(support[index], offset))
        {
            return false;
        }
    }

    return true;
}
static uint32_t GetDeviceExtensionScore(const vk::PhysicalDevice& a_device, Allocator* a_tempAllocator)
{
    const Array<const char*> optionalArray = Array<const char*>(OptionalDeviceExtensions, OptionalDeviceExtensionCount, a_tempAllocator);
    const Array<uint8_t> support = GetDeviceExtensionSupport(a_device, optionalArray, a_tempAllocator);

    uint32_t score = 0;
    for (uint32_t i = 0; i < OptionalDeviceExtensionCount; ++i)
    {
        const uint32_t index = i / 8;
        const uint32_t offset = i % 8;

        score += IISBITSET(support[index], offset) * 20;
    }

    return score;
}

static bool IsDeviceSuitable
(
    const vk::Instance& a_instance,
    const vk::PhysicalDevice& a_device,
    const Array<const char*>& a_extensions,
    AppWindow* a_window,
    Allocator* a_tempAllocator
)
{
    const vk::PhysicalDeviceProperties properties = a_device.getProperties();

    if (properties.limits.maxDrawIndirectCount <= 1)
    {
        return false;
    }

    constexpr uint32_t VersionMajor = vk::apiVersionMajor(ICARIAN_VULKAN_VERSION);
    constexpr uint32_t VersionMinor = vk::apiVersionMinor(ICARIAN_VULKAN_VERSION);

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

    if (memTotal <= (256 << 20))
    {
        return false;
    }

    if (!CheckDeviceExtensionSupport(a_device, a_extensions, a_tempAllocator))
    {
        return false;
    }

    if (!a_window->IsHeadless())
    {
        const vk::SurfaceKHR surface = a_window->GetSurface(a_instance);

        const SwapChainSupportInfo info = VulkanSwapchain::QuerySwapChainSupport(a_device, surface, a_tempAllocator, a_tempAllocator);
        if (info.Formats.Empty() || info.PresentModes.Empty())
        {
            return false;
        }
    }

    const vk::PhysicalDeviceFeatures features = a_device.getFeatures();

    return features.samplerAnisotropy;
}

static uint32_t GetDeviceScore(const vk::PhysicalDevice& a_device, Allocator* a_tempAllocator)
{
    const vk::PhysicalDeviceProperties properties = a_device.getProperties();

    // This thing keeps winning need to make sure it is last resort
    if (strstr(properties.deviceName, "llvmpipe") != NULL)
    {
        return 0;
    }

    uint32_t score = 0;
    // Weighting the score
    // While there are situations that one type can be better then the other generally in this order
    // UPDATE: Had to remove CPU score and up the discrete score as a software renderer on a 5950 X was beating a 7900 XTX oops.....
    switch (properties.deviceType) 
    {
    case vk::PhysicalDeviceType::eDiscreteGpu:
    {
        score += 2000;

        break;
    }
    case vk::PhysicalDeviceType::eIntegratedGpu:
    {
        score += 1000;

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

    score += GetDeviceExtensionScore(a_device, a_tempAllocator);

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

static bool CheckValidationLayerSupport(Allocator* a_tempAllocator)
{
    uint32_t layerCount = 0;
    VKRESERR(vk::enumerateInstanceLayerProperties(&layerCount, nullptr));

    vk::LayerProperties* availableLayers = a_tempAllocator->TAllocate<vk::LayerProperties>(layerCount);
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

static Array<const char*> GetRequiredExtensions(const AppWindow* a_window, Allocator* a_allocator)
{
    Array<const char*> extensions = Array<const char*>(a_allocator);

    const Array<const char*> windowExtensions = a_window->GetRequiredVulkanExtenions();

    extensions.Reserve(windowExtensions.Size() + 32);
    for (const char* str : windowExtensions)
    {
        extensions.Push(str);
    }

    if constexpr (VulkanEnableValidationLayers)
    {
        extensions.Push(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    return extensions;
}

#ifdef VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE
VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE
#endif

VulkanRenderEngineBackend::VulkanRenderEngineBackend(RenderEngine* a_engine) : RenderEngineBackend(a_engine)
{
    Instance = this;

    m_smallAllocator = MallocAllocator::Instance->Create<BlockAllocator>(SmallAllocatorSize, OSAllocator::Instance);
    m_largeAllocator = MallocAllocator::Instance->Create<BlockAllocator>(LargeAllocatorSize, OSAllocator::Instance);

    const AllocationSource allocationSources[] =
    {
        {
            .Alloc = m_smallAllocator,
            .MaxSize = SmallAllocatorSize >> 1,
        },
        {
            .Alloc = m_largeAllocator,
            .MaxSize = LargeAllocatorSize >> 1,
        },
        {
            .Alloc = OSAllocator::Instance,
            .MaxSize = uint64_t(-1),
        }
    };

    constexpr uint32_t AllocationSourceCount = sizeof(allocationSources) / sizeof(*allocationSources);

    m_allocator = m_smallAllocator->Create<MultiSourceAllocator>(m_smallAllocator, allocationSources, AllocationSourceCount);
#ifdef DEBUG
    m_allocator = m_smallAllocator->Create<LeakAllocator>(m_allocator);
#endif

    m_deletionAllocator = m_allocator->Create<BlockAllocator>(DeletionAllocatorSize, OSAllocator::Instance);

    m_data = m_allocator->ZTAllocate<ClassData>();
    for (uint32_t i = 0; i < VulkanMaxFlightFrames; ++i)
    {
        m_data->InterSemaphore[i] = Array<vk::Semaphore>(m_allocator);
    }

    m_data->ScratchAllocators = Array<RenderScratchAllocator>(m_allocator);

    m_data->ImageIndex = -1;

    LibRenderDoc::Init();

    m_data->VulkanLib = m_allocator->Create<LibVulkan>();

    RENDERSCRATCHFRAME;

    StackAllocator* scratchAllocator = RenderScratchAlloc::GetAllocator();

    VULKAN_HPP_DEFAULT_DISPATCHER.init((PFN_vkGetInstanceProcAddr)m_data->VulkanLib->vkGetInstanceProcAddr);

    const RenderEngine* renderEngine = GetRenderEngine();
    AppWindow* window = renderEngine->m_window;

    const Config* config = renderEngine->GetConfig();
    const bool forceMesh = config->ForceMesh();

    const bool headless = window->IsHeadless();

    const Array<const char*> enabledLayers = ILAMBDA(
    {
        Array<const char*> vals = Array<const char*>(scratchAllocator);

        if constexpr (VulkanEnableValidationLayers)
        {
            if (!CheckValidationLayerSupport(scratchAllocator))
            {
                IERROR("Validation layers not supported");
            }

            for (const char* v : ValidationLayers)
            {
                vals.Push(v);
            }
        }

        ILRETURN vals;
    });

    const std::string applicationName = renderEngine->m_config->GetApplicationName();

    const vk::ApplicationInfo appInfo = vk::ApplicationInfo
    (
        applicationName.c_str(),
        0U,
        "IcarianEngine",
        VulkanEngineVersion,
        ICARIAN_VULKAN_VERSION,
        nullptr
    );

    const Array<const char*> reqExtensions = GetRequiredExtensions(window, scratchAllocator);

    constexpr vk::DebugUtilsMessengerCreateInfoEXT DebugCreateInfo = vk::DebugUtilsMessengerCreateInfoEXT
    (
        { },
        vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose | vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning | vk::DebugUtilsMessageSeverityFlagBitsEXT::eError,
        vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance,
        DebugCallback
    );

    const void* createInfoNext = ILAMBDA(
    {
        if constexpr (VulkanEnableValidationLayers)
        {
            ILRETURN (const void*)&DebugCreateInfo;
        }

        ILRETURN (const void*)nullptr;
    });

    const vk::InstanceCreateInfo createInfo = vk::InstanceCreateInfo
    (
        { },
        &appInfo,
        enabledLayers.Size(),
        enabledLayers.Data(),
        reqExtensions.Size(),
        reqExtensions.Data(),
        createInfoNext
    );

    TRACE("Creating Vulkan Instance");
    VKRESERRMSG(vk::createInstance(&createInfo, nullptr, &m_data->Instance), "Failed to create Vulkan Instance");
    VULKAN_HPP_DEFAULT_DISPATCHER.init(m_data->Instance);

    if constexpr (VulkanEnableValidationLayers)
    {
        TRACE("Creating Vulkan Debug Layer");
        VKRESERRMSG(m_data->Instance.createDebugUtilsMessengerEXT(&DebugCreateInfo, nullptr, &m_data->Messenger), "Failed to create Vulkan Debug Printing");
    }

    Array<const char*> extensions = Array<const char*>(scratchAllocator);
    if (headless)
    {
        for (const char* ext : HeadlessDeviceExtensions)
        {
            extensions.Push(ext);
        }
    }
    else
    {
        for (const char* ext : StandaloneDeviceExtensions)
        {
            extensions.Push(ext);
        }
    }

    // We have a mesh compatibility layer however may want to always run native mesh shaders so have a flag to force it
    if constexpr (!VulkanForceMeshEmulation)
    {
        if (forceMesh)
        {
            extensions.Push(VK_EXT_MESH_SHADER_EXTENSION_NAME);
        }
    }

    uint32_t deviceCount = 0;
    VKRESERR(m_data->Instance.enumeratePhysicalDevices(&deviceCount, nullptr));

    if (deviceCount <= 0)
    {
        IcarianError("No GPU found to run.");
    }

    // TODO: Should probably skip device selection if the user specifies a override
    const vk::PhysicalDevice* devices = ILAMBDA(
    {
        vk::PhysicalDevice* vals = scratchAllocator->TAllocate<vk::PhysicalDevice>(deviceCount);
        VKRESERR(m_data->Instance.enumeratePhysicalDevices(&deviceCount, vals));

        ILRETURN vals;
    });

    uint32_t deviceScore = -1;
    m_data->PhysicalDevice = vk::PhysicalDevice(nullptr);

    for (uint32_t i = 0; i < deviceCount; ++i)
    {
        RENDERSCRATCHFRAME;

        const vk::PhysicalDevice device = devices[i];

        if (IsDeviceSuitable(m_data->Instance, device, extensions, window, scratchAllocator))
        {
            const uint32_t score = GetDeviceScore(device, scratchAllocator);
            if (score < deviceScore && deviceScore != uint32_t(-1))
            {
                continue;
            }

            deviceScore = score;
            m_data->PhysicalDevice = device;
        }
    }

    if (m_data->PhysicalDevice == vk::PhysicalDevice(nullptr))
    {
        IcarianError
        (
"No suitable GPU found to run. \
\
Please ensure you have a Vulkan 1.2 capable GPU with greater then 256MB of VRAM and Mesh Shader capabilites."
        );
    }

    TRACE("Found Vulkan Physical Device");

    const Array<const char*> optionalExtensions = Array<const char*>(OptionalDeviceExtensions, OptionalDeviceExtensionCount, scratchAllocator);
    const Array<uint8_t> optionalMask = GetDeviceExtensionSupport(m_data->PhysicalDevice, optionalExtensions, scratchAllocator);

    constexpr uint32_t OptionalMaskSize = OptionalDeviceExtensionCount / 8 + 1;
    m_data->OptionalExtensionMask = m_allocator->ZTAllocate<uint8_t>(OptionalMaskSize);

    for (uint32_t i = 0; i < OptionalDeviceExtensionCount; ++i)
    {
        const uint32_t index = i / 8;
        const uint32_t offset = i % 8;

        if (IISBITSET(optionalMask[index], offset))
        {
            ISETBIT(m_data->OptionalExtensionMask[index], offset);

            for (const char* str : extensions)
            {
                if (strcmp(str, OptionalDeviceExtensions[i]) == 0)
                {
                    goto NextExtension;
                }
            }

            extensions.Push(OptionalDeviceExtensions[i]);
        }

NextExtension:;
    }

    const vk::PhysicalDeviceProperties props = ILAMBDA(
    {
        vk::PhysicalDeviceProperties val;
        m_data->PhysicalDevice.getProperties(&val);

        ILRETURN val;
    });

    // Did for testing but leaving to make sure nothing weird is happening
    Logger::Message(std::string("Selected GPU: ") + props.deviceName.data());

    m_data->ComputeQueueIndex = -1;
    m_data->VideoDecodeQueueIndex = -1;
    m_data->GraphicsQueueIndex = -1;
    m_data->PresentQueueIndex = -1;

    if constexpr (AMDDebuggerFix)
    {
        m_data->GraphicsQueueIndex = 0;
        m_data->ComputeQueueIndex = 0;
        m_data->PresentQueueIndex = 0;
    }
    else
    {
        RENDERSCRATCHFRAME;

        uint32_t queueFamilyCount = 0;
        m_data->PhysicalDevice.getQueueFamilyProperties(&queueFamilyCount, nullptr);

        vk::QueueFamilyProperties* queueFamilies = scratchAllocator->TAllocate<vk::QueueFamilyProperties>(queueFamilyCount);
        m_data->PhysicalDevice.getQueueFamilyProperties(&queueFamilyCount, queueFamilies);

        const vk::SurfaceKHR surface = window->GetSurface(m_data->Instance);

        for (uint32_t i = 0; i < queueFamilyCount; ++i)
        {
            // If I am reading correctly Vulkan makes a guarantee that there will be atleast 1 combined graphics and compute queue
            if (queueFamilies[i].queueFlags & vk::QueueFlagBits::eGraphics && queueFamilies[i].queueFlags & vk::QueueFlagBits::eCompute)
            {
                m_data->GraphicsQueueIndex = i;
            }

            if (queueFamilies[i].queueFlags & vk::QueueFlagBits::eVideoDecodeKHR)
            {
                m_data->VideoDecodeQueueIndex = i;
            }

            if (!headless)
            {
                const vk::Bool32 presentSupport = ILAMBDA(
                {
                    vk::Bool32 val;
                    VKRESERR(m_data->PhysicalDevice.getSurfaceSupportKHR(i, surface, &val));

                    ILRETURN val;
                });

                if (presentSupport)
                {
                    // Want graphics queue to be last resort
                    if (i == m_data->GraphicsQueueIndex)
                    {
                        if (m_data->PresentQueueIndex == uint32_t(-1))
                        {
                            m_data->PresentQueueIndex = i;
                        }
                    }
                    else
                    {
                        m_data->PresentQueueIndex = i;
                    }
                }
            }

            if (queueFamilies[i].queueFlags & vk::QueueFlagBits::eCompute)
            {
                // Want present queue to be last resort
                // Have it wanting to use the present queue on NVIDIA cards so this is needed
                if (i == m_data->PresentQueueIndex)
                {
                    if (m_data->ComputeQueueIndex == uint32_t(-1))
                    {
                        m_data->ComputeQueueIndex = i;
                    }
                }
                else
                {
                    m_data->ComputeQueueIndex = i;
                }
            }
        }
    }

    typedef Set<uint32_t> UniqueQueueFamilyT;
    const Set<uint32_t> uniqueQueueFamilies = ILAMBDA(
    {
        UniqueQueueFamilyT vals = UniqueQueueFamilyT(scratchAllocator);

        if (m_data->ComputeQueueIndex != uint32_t(-1))
        {
            vals.Push(m_data->ComputeQueueIndex);
        }
        if (m_data->VideoDecodeQueueIndex != uint32_t(-1))
        {
            vals.Push(m_data->VideoDecodeQueueIndex);
        }
        if (m_data->GraphicsQueueIndex != uint32_t(-1))
        {
            vals.Push(m_data->GraphicsQueueIndex);
        }
        if (m_data->PresentQueueIndex != uint32_t(-1))
        {
            vals.Push(m_data->PresentQueueIndex);
        }

        ILRETURN vals;
    });

    IVERIFY(!uniqueQueueFamilies.Empty());

    typedef Array<vk::DeviceQueueCreateInfo> QueueCreateInfoT;
    const Array<vk::DeviceQueueCreateInfo> queueCreateInfos = ILAMBDA(
    {
        // Compiler has no idea how to resolve the template in a macro so have to use a typedef
        QueueCreateInfoT vals = QueueCreateInfoT(scratchAllocator);

        constexpr float QueuePriority = 1.0f;
        for (const uint32_t queueFamily : uniqueQueueFamilies)
        {
            const vk::DeviceQueueCreateInfo info = vk::DeviceQueueCreateInfo
            (
                { },
                queueFamily,
                1,
                &QueuePriority
            );

            vals.Push(info);
        }

        ILRETURN vals;
    });

    vk::PhysicalDeviceFeatures2 deviceFeatures2;
    deviceFeatures2.features.samplerAnisotropy = vk::True;
    deviceFeatures2.features.multiDrawIndirect = vk::True;

    void** nextChain = &deviceFeatures2.pNext;

    vk::PhysicalDeviceSamplerYcbcrConversionFeatures ycbcrConversionFeatures;
    vk::PhysicalDeviceVideoMaintenance1FeaturesKHR videoMaintance1Features;
    const bool isVideoEnabled = IsVideoEnabled();
    if (isVideoEnabled)
    {
        videoMaintance1Features.videoMaintenance1 = vk::True;

        ycbcrConversionFeatures.samplerYcbcrConversion = vk::True;
        ycbcrConversionFeatures.pNext = &videoMaintance1Features;

        *nextChain = &ycbcrConversionFeatures;
        nextChain = &videoMaintance1Features.pNext;
    }

    vk::PhysicalDeviceMeshShaderFeaturesEXT meshShaderFeature;
    const bool isMeshEnabled = IsMeshEnabled();
    if (isMeshEnabled)
    {
        meshShaderFeature.taskShader = vk::True;
        meshShaderFeature.meshShader = vk::True;
        meshShaderFeature.multiviewMeshShader = vk::False;
        meshShaderFeature.primitiveFragmentShadingRateMeshShader = vk::False;
        meshShaderFeature.meshShaderQueries = vk::False;

        *nextChain = &meshShaderFeature;
        nextChain = &meshShaderFeature.pNext;
    }

    constexpr uint32_t EnabledLayerCount = ILAMBDA(
    {
        if constexpr (VulkanEnableValidationLayers)
        {
            ILRETURN (uint32_t)(sizeof(ValidationLayers) / sizeof(*ValidationLayers));
        }

        ILRETURN uint32_t(0);
    });
    constexpr const char* const* EnabledLayerNames = ILAMBDA(
    {
        if constexpr (VulkanEnableValidationLayers)
        {
            ILRETURN ValidationLayers;
        }

        ILRETURN (const char* const*)nullptr;
    });

    const vk::DeviceCreateInfo deviceCreateInfo = vk::DeviceCreateInfo
    (
        { },
        queueCreateInfos.Size(),
        queueCreateInfos.Data(),
        EnabledLayerCount,
        EnabledLayerNames,
        extensions.Size(),
        extensions.Data(),
        nullptr,
        &deviceFeatures2
    );

    TRACE("Creating Vulkan Device");
    VKRESERRMSG(m_data->PhysicalDevice.createDevice(&deviceCreateInfo, nullptr, &m_data->LogicalDevice), "Failed to create Vulkan Logic Device");
    VULKAN_HPP_DEFAULT_DISPATCHER.init(m_data->LogicalDevice);

    const VmaVulkanFunctions vulkanFunctions = 
    {
        .vkGetInstanceProcAddr = (PFN_vkGetInstanceProcAddr)m_data->VulkanLib->vkGetInstanceProcAddr,
        .vkGetDeviceProcAddr = (PFN_vkGetDeviceProcAddr)m_data->VulkanLib->vkGetDeviceProcAddr
    };

    const VmaAllocatorCreateInfo allocatorCreateInfo =
    {
        .physicalDevice = m_data->PhysicalDevice,
        .device = m_data->LogicalDevice,
        .pVulkanFunctions = &vulkanFunctions,
        .instance = m_data->Instance,
        .vulkanApiVersion = ICARIAN_VULKAN_VERSION
    };

    TRACE("Creating Vulkan Allocator");
    VKRESERRMSG(vmaCreateAllocator(&allocatorCreateInfo, &m_data->VMAAllocator), "Failed to create Vulkan Allocator");

    // By what I can tell most devices use the same queue for graphics and compute this is for correctness shold not affect much
    // Not fussed if it shares with graphics as long as it is not the present queue
    if (m_data->ComputeQueueIndex != uint32_t(-1))
    {
        m_data->LogicalDevice.getQueue(m_data->ComputeQueueIndex, 0, &m_data->ComputeQueue);
    }
    if (m_data->VideoDecodeQueueIndex != uint32_t(-1))
    {
        m_data->LogicalDevice.getQueue(m_data->VideoDecodeQueueIndex, 0, &m_data->VideoDecodeQueue);
    }
    if (m_data->GraphicsQueueIndex != uint32_t(-1))
    {
        m_data->LogicalDevice.getQueue(m_data->GraphicsQueueIndex, 0, &m_data->GraphicsQueue);
    }
    if (m_data->PresentQueueIndex != uint32_t(-1))
    {
        m_data->LogicalDevice.getQueue(m_data->PresentQueueIndex, 0, &m_data->PresentQueue);
    }

    TRACE("Got Vulkan Queues");

    const vk::CommandPoolCreateInfo poolInfo = vk::CommandPoolCreateInfo
    (
        vk::CommandPoolCreateFlagBits::eTransient,
        m_data->GraphicsQueueIndex
    );

    for (unsigned int i = 0; i < CommandIndex_Last; ++i)
    {
        VKRESERRMSG(m_data->LogicalDevice.createCommandPool(&poolInfo, nullptr, &m_data->CommandPools[i]), "Failed to create command pool");
    }

    if (isVideoEnabled)
    {
        m_data->VideoDecodeCapabilities = { };

        m_data->VideoDecodeCapabilities.VideoProfile = vk::VideoProfileInfoKHR
        (
            vk::VideoCodecOperationFlagBitsKHR::eDecodeH264,
            vk::VideoChromaSubsamplingFlagBitsKHR::e420,
            vk::VideoComponentBitDepthFlagBitsKHR::e8,
            vk::VideoComponentBitDepthFlagBitsKHR::e8,
            &DecodeProfile
        );

        m_data->VideoDecodeCapabilities.VideoCapabilities.pNext = &m_data->VideoDecodeCapabilities.DecodeCapabilities;
        m_data->VideoDecodeCapabilities.DecodeCapabilities.pNext = &m_data->VideoDecodeCapabilities.DecodeH264Capabilities;

        VKRESERR(m_data->PhysicalDevice.getVideoCapabilitiesKHR(&m_data->VideoDecodeCapabilities.VideoProfile, &m_data->VideoDecodeCapabilities.VideoCapabilities));
    }

    m_data->PushPool = m_allocator->Create<VulkanPushPool>(this);
    m_data->ComputeEngine = m_allocator->Create<VulkanComputeEngine>(this);
    m_data->GraphicsEngine = m_allocator->Create<VulkanGraphicsEngine>(this);

#ifdef DEBUG
    printf("Used scratch memory in setup: %dKiB \n", (uint32_t)(scratchAllocator->GetUsedSize() >> 10));
#endif
}
VulkanRenderEngineBackend::~VulkanRenderEngineBackend()
{
    const RenderEngine* renderEngine = GetRenderEngine();
    AppWindow* window = renderEngine->m_window;

    TRACE("Begin Vulkan clean up");
    // We are in the destructor and are closing the application so this is fine but still makes my skin crawl having a waitIdle
    m_data->LogicalDevice.waitIdle();

    m_allocator->Destroy(m_data->ComputeEngine);
    m_allocator->Destroy(m_data->PushPool);

    m_data->GraphicsEngine->Cleanup();

    TRACE("Initial destroy Vulkan Deletion Objects");
    for (uint32_t i = 0; i < VulkanDeletionQueueSize; ++i)
    {
        const TLockArray a = m_data->DeletionObjects[i].ToLockArray();

        for (VulkanDeletionObject* obj : a)
        {
            if (obj == nullptr)
            {
                continue;
            }

            obj->Destroy();

            m_deletionAllocator->Destroy(obj);
        }

        m_data->DeletionObjects[i].UClear();
    }

    m_allocator->Destroy(m_data->GraphicsEngine);

    TRACE("Final destroy Vulkan Deletion Objects");
    for (uint32_t i = 0; i < VulkanDeletionQueueSize; ++i)
    {
        const TLockArray a = m_data->DeletionObjects[i].ToLockArray();

        for (VulkanDeletionObject* obj : a)
        {
            if (obj == nullptr)
            {
                continue;
            }

            obj->Destroy();

            m_deletionAllocator->Destroy(obj);
        }

        m_data->DeletionObjects[i].UClear();
    }

    TRACE("Destroy Command Pool");
    for (uint32_t i = 0; i < CommandIndex_Last; ++i)
    {
        m_data->LogicalDevice.destroyCommandPool(m_data->CommandPools[i]);
    }

    if (m_data->Swapchain != nullptr)
    {
        m_allocator->Destroy(m_data->Swapchain);
        m_data->Swapchain = nullptr;
    }

    TRACE("Destroying Vulkan Sync Objects");
    for (uint32_t i = 0; i < VulkanMaxFlightFrames; ++i)
    {
        for (uint32_t j = 0; j < m_data->InterSemaphore[i].Size(); ++j)
        {
            m_data->LogicalDevice.destroySemaphore(m_data->InterSemaphore[i][j]);
        }
    }

    TRACE("Destroying Vulkan Allocator");
    vmaDestroyAllocator(m_data->VMAAllocator);

    vk::SurfaceKHR surface = window->GetSurface(m_data->Instance);
    if (surface != vk::SurfaceKHR(nullptr))
    {
        TRACE("Destroying Surface");
        m_data->Instance.destroySurfaceKHR(surface);
    }

    TRACE("Destroying Devices");
    m_data->LogicalDevice.destroy();

    if constexpr (VulkanEnableValidationLayers)
    {
        TRACE("Cleaning Vulkan Diagnostics");
        m_data->Instance.destroyDebugUtilsMessengerEXT(m_data->Messenger);
    }

    TRACE("Destroying Vulkan Instance");
    m_data->Instance.destroy();

    m_allocator->Destroy(m_data->VulkanLib);

    m_allocator->Free(m_data->OptionalExtensionMask);

    TRACE("Destroying Rendering Allocators");
    for (const RenderScratchAllocator& a : m_data->ScratchAllocators)
    {
        m_allocator->Destroy(a.Allocator);
    }

    m_allocator->Destroy(m_data);

    m_allocator->Destroy(m_deletionAllocator);

    {
#ifdef DEBUG
        Allocator* upstreamAllocator = ((LeakAllocator*)m_allocator)->GetUpstreamAllocator();
        IDEFER(m_smallAllocator->Destroy(upstreamAllocator));
#endif
        m_smallAllocator->Destroy(m_allocator);
    }

    MallocAllocator::Instance->Destroy(m_smallAllocator);
    MallocAllocator::Instance->Destroy(m_largeAllocator);

    LibRenderDoc::Destroy();

    TRACE("Vulkan cleaned up");
}

bool VulkanRenderEngineBackend::IsExtensionEnabled(const std::string_view& a_extension) const
{
    const OptionalHashes::HashType hash = StringHash<OptionalHashes::HashType>(a_extension.data());

    for (uint32_t i = 0; i < OptionalDeviceExtensionCount; ++i)
    {
        if (hash == OptionalDeviceExtensionHashes.Data[i])
        {
            const uint32_t index = i / 8;
            const uint32_t offset = i % 8;

            return IISBITSET(m_data->OptionalExtensionMask[index], offset);
        }
    }

    return false;
}

void VulkanRenderEngineBackend::Update(double a_delta, double a_time)
{
    // TODO: Can probably better manage semaphores.
    const RenderEngine* renderEngine = GetRenderEngine();
    AppWindow* window = renderEngine->m_window;

    // When we are running headless RenderDoc does not know when a frame begins or ends because we do not use a system swapchain
    // To get around that we need to tell RenderDoc when a frame begins and ends
    LibRenderDoc::StartFrame();
    IDEFER(LibRenderDoc::EndFrame());

    const bool init = m_data->Swapchain != nullptr;

    vk::Semaphore lastSemaphore;
    {
        PROFILESTACK("Swap Setup");

        RENDERSCRATCHFRAME;

        StackAllocator* scratchAllocator = RenderScratchAlloc::GetAllocator();

        if (!init)
        {
            m_data->Swapchain = m_allocator->Create<VulkanSwapchain>(this, window, m_allocator, scratchAllocator);
            m_data->GraphicsEngine->SetSwapchain(m_data->Swapchain);
        }

        if (!m_data->Swapchain->StartFrame(&m_data->ImageIndex, &lastSemaphore, a_delta, a_time, scratchAllocator))
        {
            return;
        }
    }

    Profiler::StartFrame("Render Update");

    m_data->PushPool->Reset(m_data->CurrentFrame);

    // TODO: Down the line setup the compute and graphics engine to return VulkanCommandBuffers
    const VulkanCommandBuffer computeCommandBuffer = m_data->ComputeEngine->Update(a_delta, a_time, m_data->CurrentFrame);
    const vk::CommandBuffer vulkanComputeBuffer = computeCommandBuffer.GetCommandBuffer();

    const Array<VulkanCommandBuffer> commandBuffers = m_data->GraphicsEngine->Update(a_delta, a_time, m_data->CurrentFrame);

    Profiler::StartFrame("Render Setup");

    // TODO: This can probably be updated to account for buckets over command buffers
    const uint32_t buffersSize = commandBuffers.Size();
    const uint32_t targetSemaphores = buffersSize + 3;

    const uint32_t semaphoreCount = m_data->InterSemaphore[m_data->CurrentFlightFrame].Size();
    // const uint32_t endBuffer = buffersSize - 1;

    if (targetSemaphores > semaphoreCount)
    {
        TRACE("Allocating inter semaphores");
        const uint32_t diff = targetSemaphores - semaphoreCount;

        constexpr vk::SemaphoreCreateInfo SemaphoreInfo;

        for (uint32_t i = 0; i < diff; ++i)
        {
            vk::Semaphore semaphore;
            VKRESERRMSG(m_data->LogicalDevice.createSemaphore(&SemaphoreInfo, nullptr, &semaphore), "Failed to create inter semaphore");

            m_data->InterSemaphore[m_data->CurrentFlightFrame].Push(semaphore);
        }
    }

    Profiler::StopFrame();

    Profiler::StartFrame("Render Submit");

    {
        // Urgh got hit by an it depends again as to if submiting a single command buffer sequentially or grouping command buffers is faster
        // Seems vary from GPU and driver wildly got a regression on some hardware and an improvement on others.....
        // Why can things not be simple things always seem to turn into its complicated and it depends
        // Need to see if there is a reliable way of seeing and adapting based off it but probably a rabbit hole on its own
        // I have seen a worst case of a drop from ~1.1ms to ~1.4ms frame times
        // TODO: Investigate why it is a regression on some and an improvement on others suspect resource contention

        RENDERSCRATCHFRAME;

        StackAllocator* scratchAllocator = RenderScratchAlloc::GetAllocator();

        Array<Array<VulkanCommandBuffer>> commandBuckets = Array<Array<VulkanCommandBuffer>>(scratchAllocator);
        commandBuckets.Reserve(buffersSize);

        Array<VulkanCommandBuffer> currentBucket = Array<VulkanCommandBuffer>(scratchAllocator);
        uint32_t currentBucketSlot = -1;
        for (const VulkanCommandBuffer& buffer : commandBuffers)
        {
            const vk::CommandBuffer cmdBuffer = buffer.GetCommandBuffer();
            if (cmdBuffer == vk::CommandBuffer(nullptr))
            {
                IERROR("Passed null command buffer");
            }

            const e_VulkanCommandBufferStage stage = buffer.GetBufferStage();

            const uint32_t bucketSlot = ILAMBDA(
            {
                switch (stage)
                {
                case VulkanCommandBufferStage_ShadowPass:
                case VulkanCommandBufferStage_DeferredPass:
                {
                    ILRETURN 0;
                }
                case VulkanCommandBufferStage_LightingPass:
                case VulkanCommandBufferStage_ForwardPass:
                {
                    ILRETURN 1;
                }
                case VulkanCommandBufferStage_PostPass:
                {
                    ILRETURN 2;
                }
                case VulkanCommandBufferStage_UIPass:
                {
                    ILRETURN 3;
                }
                default:
                {
                    break;
                }
                }

                IERROR("Invalid CommandBufferStage");

                ILRETURN 0;
            });

            if (bucketSlot != currentBucketSlot)
            {
                IDEFER(currentBucketSlot = bucketSlot);

                if (currentBucketSlot != uint32_t(-1))
                {
                    commandBuckets.Push(currentBucket);

                    currentBucket.Clear();
                }
            }

            currentBucket.Push(buffer);
        }

        if (!currentBucket.Empty())
        {
            commandBuckets.Push(currentBucket);

            currentBucket.Clear();
        }

        const uint32_t commandBucketCount = commandBuckets.Size();
        if (commandBucketCount > 0)
        {
            const uint32_t semaphoreCount = m_data->InterSemaphore[m_data->CurrentFlightFrame].Size();

            const uint32_t chainSemaphoreCount = ILAMBDA(
            {
                if (vulkanComputeBuffer != vk::CommandBuffer(nullptr))
                {
                    return 2;
                }

                return 1;
            });

            const vk::Semaphore* chainSemaphores = ILAMBDA(
            {
                vk::Semaphore* vals = scratchAllocator->TAllocate<vk::Semaphore>(chainSemaphoreCount);

                for (uint32_t i = 0; i < chainSemaphoreCount; ++i)
                {
                    vals[i] = m_data->InterSemaphore[m_data->CurrentFlightFrame][semaphoreCount - (3 - i)];
                }

                ILRETURN vals;
            });

            const ThreadGuard l = ThreadGuard(m_graphicsQueueLock);

            constexpr vk::PipelineStageFlags ChainFlags = vk::PipelineStageFlagBits::eAllCommands;

            const vk::SubmitInfo initialSubmit = vk::SubmitInfo
            (
                1,
                &lastSemaphore,
                &ChainFlags,
                // Huh apparently this is valid and can chain semaphores without executing a command buffer
                0,
                nullptr,
                chainSemaphoreCount,
                chainSemaphores
            );

            VKRESERRMSG(m_data->GraphicsQueue.submit(1, &initialSubmit, nullptr), "Failed to submit initial chain");

            lastSemaphore = chainSemaphores[0];

            for (uint32_t i = 0; i < commandBucketCount; ++i)
            {
                RENDERSCRATCHFRAME;

                const vk::Semaphore curSemaphore = m_data->InterSemaphore[m_data->CurrentFlightFrame][i];
                IDEFER(lastSemaphore = curSemaphore);

                const Array<VulkanCommandBuffer>& buffers = commandBuckets[i];
                const uint32_t commandBufferCount = buffers.Size();

                const vk::CommandBuffer* commandBuffers = ILAMBDA(
                {
                    vk::CommandBuffer* vals = scratchAllocator->TAllocate<vk::CommandBuffer>(commandBufferCount);

                    for (uint32_t i = 0; i < commandBufferCount; ++i)
                    {
                        const VulkanCommandBuffer& buffer = buffers[i];

                        vals[i] = buffer.GetCommandBuffer();
                    }

                    ILRETURN vals;
                });

                constexpr vk::PipelineStageFlags GraphicsWaitFlags = vk::PipelineStageFlagBits::eAllGraphics;

                const vk::SubmitInfo submitInfo = vk::SubmitInfo
                (
                    1,
                    &lastSemaphore,
                    &GraphicsWaitFlags,
                    commandBufferCount,
                    commandBuffers,
                    1,
                    &curSemaphore
                );

                VKRESERRMSG(m_data->GraphicsQueue.submit(1, &submitInfo, nullptr), "Failed to submit graphics bucket");
            }

            const vk::Semaphore* waitSemaphores = ILAMBDA(
            {
                // This can probably been cleaned up it is a mess
                vk::Semaphore* vals = scratchAllocator->TAllocate<vk::Semaphore>(chainSemaphoreCount);

                if (vulkanComputeBuffer != vk::CommandBuffer(nullptr))
                {
                    vals[1] = m_data->InterSemaphore[m_data->CurrentFlightFrame][semaphoreCount - 1];
                }

                vals[0] = lastSemaphore;

                ILRETURN vals;
            });

            // Huh did not know that submission order does matter even if it is across queues
            // WHY!?! I thought that submission order only mattered in the same queue
            // Well it is fixed now, guess it is another docs vs spec thing
            // You would think I would know by now to look at the spec
            if (vulkanComputeBuffer != vk::CommandBuffer(nullptr))
            {
                const vk::SubmitInfo submitInfo = vk::SubmitInfo
                (
                    1,
                    &(chainSemaphores[1]),
                    &ChainFlags,
                    1,
                    &vulkanComputeBuffer,
                    1,
                    &(waitSemaphores[1])
                );

                VKRESERRMSG(m_data->ComputeQueue.submit(1, &submitInfo, nullptr), "Failed to submit compute command");
            }

            const vk::PipelineStageFlags* waitFlags = ILAMBDA(
            {
                vk::PipelineStageFlags* vals = scratchAllocator->TAllocate<vk::PipelineStageFlags>(chainSemaphoreCount);

                for (uint32_t i = 0; i < chainSemaphoreCount; ++i)
                {
                    vals[i] = vk::PipelineStageFlagBits::eAllCommands;
                }

                ILRETURN vals;
            });

            const vk::Semaphore swapSemaphore = m_data->Swapchain->GetEndSemaphore(m_data->CurrentFlightFrame);
            const vk::Fence swapFence = m_data->Swapchain->GetFence(m_data->CurrentFlightFrame);

            const vk::SubmitInfo submitInfo = vk::SubmitInfo
            (
                chainSemaphoreCount,
                waitSemaphores,
                waitFlags,
                0,
                nullptr,
                1,
                &swapSemaphore
            );

            VKRESERRMSG(m_data->GraphicsQueue.submit(1, &submitInfo, swapFence), "Failed to submit wait chain");
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

        const uint32_t nextIndex = (m_data->DeletionQueueIndex + 1) % VulkanDeletionQueueSize;
        IDEFER(m_data->DeletionQueueIndex = nextIndex);

        const TLockArray a = m_data->DeletionObjects[nextIndex].ToLockArray();

        for (VulkanDeletionObject* obj : a)
        {
            if (obj != nullptr)
            {
                obj->Destroy();

                m_deletionAllocator->Destroy(obj);
            }
        }

        m_data->DeletionObjects[nextIndex].UClear();
    }

    {
        PROFILESTACK("Swap Present");

        m_data->Swapchain->EndFrame(m_data->ImageIndex);

        m_data->CurrentFrame = (m_data->CurrentFrame + 1) % VulkanFlightPoolSize;
        m_data->CurrentFlightFrame = (m_data->CurrentFlightFrame + 1) % VulkanMaxFlightFrames;
    }

    {
        PROFILESTACK("Allocators");

        m_smallAllocator->TrimBlocks();
        m_largeAllocator->TrimBlocks();

        // With the Scratch allocators can sometimes be used by scripting threads so need to wait on them to finish
        // I clear the TStatic because not all thread may need a scratch allocator and prefer hand them out as needed
        const ThreadGuard g = ThreadGuard(m_scratchLock);

        for (uint32_t i = 0; i < m_data->ScratchIndex; ++i)
        {
            const RenderScratchAllocator& a = m_data->ScratchAllocators[i];

            while (a.Count > 0) { }

            a.Allocator->Reset();
        }

        ScratchAllocator.Clear();

        m_data->ScratchIndex = 0;
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
TLockObj<vk::CommandBuffer, SpinLock>* VulkanRenderEngineBackend::CreateCommandBuffer(vk::CommandBufferLevel a_level, e_CommandIndex a_index)
{
    IVERIFY(a_index < CommandIndex_Last);

    const vk::CommandBufferAllocateInfo allocInfo = vk::CommandBufferAllocateInfo
    (
        m_data->CommandPools[a_index],
        a_level,
        1
    );

    TLockObj<vk::CommandBuffer, SpinLock>* lockObj = m_allocator->Create<TLockObj<vk::CommandBuffer, SpinLock>>(&m_graphicsQueueLock);

    vk::CommandBuffer cmdBuffer;
    VKRESERRMSG(m_data->LogicalDevice.allocateCommandBuffers(&allocInfo, &cmdBuffer), "Failed to Allocate Command Buffer");

    lockObj->Set(cmdBuffer);

    return lockObj;
}
void VulkanRenderEngineBackend::DestroyCommandBuffer(TLockObj<vk::CommandBuffer, SpinLock>* a_buffer, e_CommandIndex a_index)
{
    IVERIFY(a_index < CommandIndex_Last);
    IDEFER(m_allocator->Destroy(a_buffer));

    const vk::CommandBuffer buffer = a_buffer->Get();

    PushDeletionObject<VulkanCommandBufferDeletionObject>(m_data->LogicalDevice, m_data->CommandPools[a_index], buffer);
}

TLockObj<vk::CommandBuffer, SpinLock>* VulkanRenderEngineBackend::BeginSingleCommand(e_CommandIndex a_index)
{
    TLockObj<vk::CommandBuffer, SpinLock>* buffer = CreateCommandBuffer(vk::CommandBufferLevel::ePrimary, a_index);
    const vk::CommandBuffer cmdBuffer = buffer->Get();

    constexpr vk::CommandBufferBeginInfo BufferBeginInfo = vk::CommandBufferBeginInfo
    (
        vk::CommandBufferUsageFlagBits::eOneTimeSubmit
    );

    VKRESERR(cmdBuffer.begin(&BufferBeginInfo));

    return buffer;
}
void VulkanRenderEngineBackend::EndSingleCommand(TLockObj<vk::CommandBuffer, SpinLock>* a_buffer, e_CommandIndex a_index)
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

    const vk::Queue queue = ILAMBDA(
    {
        switch (a_index)
        {
        case CommandIndex_Present:
        {
            ILRETURN m_data->PresentQueue;
        }
        case CommandIndex_Graphics:
        {
            ILRETURN m_data->GraphicsQueue;
        }
        case CommandIndex_Compute:
        {
            ILRETURN m_data->ComputeQueue;
        } 
        case CommandIndex_VideoDecode:
        {
            ILRETURN m_data->VideoDecodeQueue;
        }
        default:
        {
            break;
        }
        }

        IERROR("Invalid Command Index");

        ILRETURN vk::Queue();
    });

    VKRESERRMSG(queue.submit(1, &submitInfo, nullptr), "Failed to Submit Command");
}

e_RenderDeviceType VulkanRenderEngineBackend::GetDeviceType() const
{
    const vk::PhysicalDeviceProperties props = ILAMBDA(
    {
        vk::PhysicalDeviceProperties val;
        m_data->PhysicalDevice.getProperties(&val);

        ILRETURN val;
    });

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
    // Virtual GPU can be tricky to tell as it can be an actual GPU or it can be software so we assume worst case
    // The typical scenarios that virtual tend to pop are server in most instances where they use something like CUDA over Vulkan regardless
    // Not gonna look too much into it as we target desktop regardless
    // I am aware of people that flash server firmware onto desktop cards to support GPU slicing that can trigger this hence still kinda need to support it
    // I could just go not my problem but just 1 line of code to have it start on them and any issues are the users problem as they decided to run server firmware on a desktop card
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
    vmaGetHeapBudgets(m_data->VMAAllocator, bugets);
    const VkPhysicalDeviceMemoryProperties* properties;
    vmaGetMemoryProperties(m_data->VMAAllocator, &properties);

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
    vmaGetHeapBudgets(m_data->VMAAllocator, bugets);
    const VkPhysicalDeviceMemoryProperties* properties;
    vmaGetMemoryProperties(m_data->VMAAllocator, &properties);

    uint64_t total = 0;
    for (uint32_t i = 0; i < properties->memoryTypeCount; ++i)
    {
        const vk::MemoryType type = properties->memoryTypes[i];

        if (type.propertyFlags & vk::MemoryPropertyFlagBits::eDeviceLocal)
        {
            const VmaBudget& b = bugets[type.heapIndex];
            total += b.budget;
        }
    }

    return total;
}

uint32_t VulkanRenderEngineBackend::GenerateMesh
(
    const void* a_vertices,
    uint32_t a_vertexCount,
    uint16_t a_vertexStride,
    const uint32_t* a_meshletVertices,
    uint32_t a_meshletVertexCount,
    const uint8_t* a_meshletTriangles,
    uint32_t a_meshletTriangleCount,
    const IcarianCore::ShaderMeshletBuffer* a_meshlets,
    uint32_t a_meshletCount,
    float a_radius
)
{
    return m_data->GraphicsEngine->GenerateMesh
    (
        a_vertices,
        a_vertexCount,
        a_vertexStride,
        a_meshletVertices,
        a_meshletVertexCount,
        a_meshletTriangles,
        a_meshletTriangleCount,
        a_meshlets,
        a_meshletCount,
        a_radius
    );
}
void VulkanRenderEngineBackend::DestroyMesh(uint32_t a_addr)
{
    m_data->GraphicsEngine->DestroyMesh(a_addr);
}

uint32_t VulkanRenderEngineBackend::GenerateModel
(
    const void* a_vertices,
    uint32_t a_vertexCount,
    uint16_t a_vertexStride,
    const uint32_t* a_indices,
    uint32_t a_indexCount,
    float a_radius
)
{
    return m_data->GraphicsEngine->GenerateModel(a_vertices, a_vertexCount, a_vertexStride, a_indices, a_indexCount, a_radius);
}
void VulkanRenderEngineBackend::DestroyModel(uint32_t a_addr)
{
    m_data->GraphicsEngine->DestroyModel(a_addr);
}

uint32_t VulkanRenderEngineBackend::GenerateTexture(uint32_t a_width, uint32_t a_height, e_TextureFormat a_format, const void* a_data)
{
    return m_data->GraphicsEngine->GenerateTexture(a_width, a_height, a_format, a_data);
}
uint32_t VulkanRenderEngineBackend::GenerateTextureMipMapped
(
    uint32_t a_width,
    uint32_t a_height,
    uint32_t a_levels,
    uint64_t* a_offsets,
    e_TextureFormat a_format,
    const void* a_data,
    uint64_t a_dataSize
)
{
    return m_data->GraphicsEngine->GenerateMipMappedTexture(a_width, a_height, a_levels, a_offsets, a_format, a_data, a_dataSize);
}
void VulkanRenderEngineBackend::DestroyTexture(uint32_t a_addr)
{
    m_data->GraphicsEngine->DestroyTexture(a_addr);
}

uint32_t VulkanRenderEngineBackend::GenerateTextureSampler
(
    uint32_t a_textureAddr,
    e_TextureMode a_textureMode,
    e_TextureFilter a_filterMode,
    e_TextureAddress a_addressMode,
    uint32_t a_slot
)
{
    return m_data->GraphicsEngine->GenerateTextureSampler(a_textureAddr, a_textureMode, a_filterMode, a_addressMode, a_slot);
}
void VulkanRenderEngineBackend::DestroyTextureSampler(uint32_t a_addr)
{
    m_data->GraphicsEngine->DestroyTextureSampler(a_addr);
}

void VulkanRenderEngineBackend::IncrementScratchFrame(uint32_t a_index)
{
    IVERIFY(a_index < m_data->ScratchAllocators.Size());

    if (m_data->ScratchAllocators[a_index].Count == 0)
    {
        const SharedThreadGuard g = SharedThreadGuard(m_scratchLock);

        ++(m_data->ScratchAllocators[a_index].Count);

        return;
    }

    ++(m_data->ScratchAllocators[a_index].Count);
}
void VulkanRenderEngineBackend::DecrementScratchFrame(uint32_t a_index)
{
    IVERIFY(a_index < m_data->ScratchAllocators.Size());

    --(m_data->ScratchAllocators[a_index].Count);
}

StackAllocator* VulkanRenderEngineBackend::GetStackAllocator(uint32_t* a_index)
{
    IVERIFY(a_index != nullptr);

    const ThreadGuard g = ThreadGuard(m_scratchLock);

    const uint32_t index = m_data->ScratchIndex++;

    if (m_data->ScratchIndex >= m_data->ScratchAllocators.Size())
    {
        StackAllocator* allocator = m_allocator->Create<StackAllocator>(ScratchAllocatorSize, OSAllocator::Instance);

        const RenderScratchAllocator data =
        {
            .Allocator = allocator
        };

        m_data->ScratchAllocators.Push(data);
    }

    *a_index = index;

    return m_data->ScratchAllocators[index].Allocator;
}

void VulkanRenderEngineBackend::InternalPushDeletionObject(VulkanDeletionObject* a_object)
{
    m_data->DeletionObjects[m_data->DeletionQueueIndex].Push(a_object);
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
