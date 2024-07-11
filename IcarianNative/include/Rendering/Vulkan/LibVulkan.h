#pragma once

#ifdef ICARIANNATIVE_ENABLE_GRAPHICS_VULKAN

#include "Rendering/Vulkan/IcarianVulkanHeader.h"

class LibVulkan
{
private:
    void* m_lib;

protected:

public:
    LibVulkan();
    ~LibVulkan();

    void* vkGetInstanceProcAddr;
    void* vkGetDeviceProcAddr;
};

#endif