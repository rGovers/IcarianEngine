// Icarian Engine - C# Game Engine
// 
// License at end of file.

#ifdef ICARIANNATIVE_ENABLE_GRAPHICS_VULKAN

#include "Rendering/Vulkan/LibVulkan.h"

#include "Core/IcarianError.h"

#ifdef WIN32
#include "Core/WindowsHeaders.h"

constexpr const char VulkanLib[] = "vulkan-1.dll";
#else
#include <dlfcn.h>

constexpr const char VulkanLib[] = "libvulkan.so";
#endif

LibVulkan::LibVulkan()
{
    IERRBLOCK;

    IERRDEFER(IcarianError
        (
"Icarian Engine failed to load Vulkan. \n\
\n\
Please ensure you have a Vulkan 1.2 capable GPU and drivers are upto date."
        ));

    // Yes I am aware there are more efficent ways to deal with Vulkan and can directly load driver bindings over going through vulkan-1.dll however until it is an issue KISS
    // If it is a good implementation vulkan-1.dll vkGetInstanceProcAddr should give the driver bindings anyway atleast in theory drivers/Windows are a bitch so cannot be certain
#ifdef WIN32
    HMODULE lib = LoadLibraryA(VulkanLib);
    m_lib = lib;
    IERRCHECK(m_lib != NULL);

    vkGetInstanceProcAddr = (void*)GetProcAddress(lib, "vkGetInstanceProcAddr");
    IERRCHECK(vkGetInstanceProcAddr != NULL);
    vkGetDeviceProcAddr = (void*)GetProcAddress(lib, "vkGetDeviceProcAddr");
    IERRCHECK(vkGetDeviceProcAddr != NULL);
#else
    m_lib = dlopen(VulkanLib, RTLD_LAZY | RTLD_LOCAL);
    IERRCHECK(m_lib != NULL);

    vkGetInstanceProcAddr = dlsym(m_lib, "vkGetInstanceProcAddr");
    IERRCHECK(vkGetInstanceProcAddr != NULL);
    vkGetDeviceProcAddr = dlsym(m_lib, "vkGetDeviceProcAddr");
    IERRCHECK(vkGetDeviceProcAddr != NULL);
#endif
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