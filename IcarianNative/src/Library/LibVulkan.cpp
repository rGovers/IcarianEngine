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
Please ensure you have a Vulkan 1.2 capable GPU and drivers are upto date."
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