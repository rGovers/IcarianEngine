// Icarian Engine - C# Game Engine
// 
// License at end of file.

#ifdef ICARIANNATIVE_ENABLE_GRAPHICS_VULKAN

#include "Rendering/Vulkan/LibVulkan.h"

#ifdef WIN32
#include "Core/WindowsHeaders.h"

constexpr const char VulkanLib[] = "vulkan-1.dll";
#else
#include <dlfcn.h>

constexpr const char VulkanLib[] = "libvulkan.so";
#endif

LibVulkan::LibVulkan()
{
#ifdef WIN32
    HMODULE lib = LoadLibraryA(VulkanLib);
    m_lib = lib;
    if (m_lib == NULL)
    {
        goto Error;
    }

    vkGetInstanceProcAddr = (void*)GetProcAddress(lib, "vkGetInstanceProcAddr");
    if (vkGetInstanceProcAddr == NULL)
    {
        goto Error;
    }

    vkGetDeviceProcAddr = (void*)GetProcAddress(lib, "vkGetDeviceProcAddr");
    if (vkGetDeviceProcAddr == NULL)
    {
        goto Error;
    }

    return;
#else
    m_lib = dlopen(VulkanLib, RTLD_LAZY | RTLD_LOCAL);
    if (m_lib == NULL)
    {
        goto Error;
    }

    vkGetInstanceProcAddr = dlsym(m_lib, "vkGetInstanceProcAddr");
    if (vkGetInstanceProcAddr == NULL)
    {
        goto Error;
    }
    vkGetDeviceProcAddr = dlsym(m_lib, "vkGetDeviceProcAddr");
    if (vkGetDeviceProcAddr == NULL)
    {
        goto Error;
    }

    return;
#endif

Error:;
    IcarianError
        (
"Icarian Engine failed to load Vulkan. \
\
Please ensure you have a Vulkan 1.1 capable GPU and drivers are upto date."
        );
}
LibVulkan::~LibVulkan()
{
#ifdef WIN32
    FreeLibrary((HMODULE)m_lib);
#else
    dlclose(m_lib);
#endif
}

#endif