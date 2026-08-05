// Icarian Engine - C# Game Engine
//
// License at end of file.

#pragma once

#ifdef ICARIANNATIVE_ENABLE_GRAPHICS_VULKAN

#if defined(WIN32) && defined(ICARIANNATIVE_ENABLE_DMA)
#include "Core/WindowsHeaders.h"
#define VK_USE_PLATFORM_WIN32_KHR
#endif

#include "Core/IcarianPragma.h"

// TODO: Need to further investigate but GCC is complaining about Vulkan using a header that is deprecated in C++17
// Seems to contradict documentation by what I can tell but as iso646 was not removed until C++20 by what I can tell
// Supressing for now to get it to compile again as I am not sure what tree to bark up about it
// Probably have to poke the Khronos Group about it
// This get trickier as too my knowledge the header in question that includes iso646 is generated for the Vulkan spec and not hand rolled
// That may mean the generator need to change or the spec and cannot just patch it
// Seems to compile if you use <version> over <ciso646> so not sure if spec issue or GCC issue
ICARIAN_WARNINGPUSH
// Yes this is a weird warning that you hardly see because it is relating to the CPP spec
ICARIAN_WARNINGSUPPRESS("-Wcpp")
#include <vulkan/vulkan.hpp>
ICARIAN_WARNINGPOP

// Nvidia driver was being weird about SPIRV 1.4 on Vulkan 1.1 bumping to Vulkan 1.2 seems to have fixed it
// Weird that the extension was being odd but not gonna question it
// Validation layer was quiet just the driver complaining
#define ICARIAN_VMA_VULKAN_VERSION 1002000
#define ICARIAN_VULKAN_VERSION VK_API_VERSION_1_2

#define VMA_VULKAN_VERSION ICARIAN_VMA_VULKAN_VERSION
#include <vk_mem_alloc.h>

#include "Core/StringUtils.h"
#include "IcarianError.h"

#define ICARIAN_VULKANVERSION_STRX(x) #x
#define ICARIAN_VULKANVERSION_STRI(x) ICARIAN_VULKANVERSION_STRX(x)

static constexpr uint32_t VulkanMaxFlightFrames = 2;
static constexpr uint32_t VulkanFlightPoolSize = VulkanMaxFlightFrames + 1;
static constexpr uint32_t VulkanDeletionQueueSize = VulkanFlightPoolSize + 1;
static constexpr uint32_t VulkanEngineVersionHash = StringHash<uint32_t>
(
    ICARIAN_VULKANVERSION_STRI(ICARIANNATIVE_VERSION_MAJOR) "."
    ICARIAN_VULKANVERSION_STRI(ICARIANNATIVE_VERSION_MINOR) "."
    ICARIAN_VULKANVERSION_STRI(ICARIANNATIVE_VERSION_PATCH) ":"
    ICARIAN_VULKANVERSION_STRI(ICARIANNATIVE_COMMIT_HASH) " "
    ICARIAN_VULKANVERSION_STRI(ICARIANNATIVE_VERSION_TAG)
);

// AMD debuggers do not support multi queue so switch this to true when you need to do graphics debugging with AMD tools
// AMD GPUs run just fine it is just their debuggers
// There is an active issue that has not been fixed yet
static constexpr bool AMDDebuggerFix = false;

// Debug flag to force enable Mesh Shader emulation
static constexpr bool VulkanForceMeshEmulation = false;

#ifdef NDEBUG
static constexpr bool VulkanEnableValidationLayers = false;
#else
static constexpr bool VulkanEnableValidationLayers = true;
#endif

#ifdef ICARIANNATIVE_ENABLE_MARKERS
#define VULKAN_MARKER_COL(engine, cmdBuffer, name, r, g, b) \
    const bool _markersEnabled = engine->IsExtensionEnabled(VK_EXT_DEBUG_MARKER_EXTENSION_NAME); \
    if (_markersEnabled) \
    { \
        const vk::DebugMarkerMarkerInfoEXT markerInfo = vk::DebugMarkerMarkerInfoEXT \
        ( \
            name, \
            { r / 255.0f, g / 255.0f, b / 255.0f, 1.0f } \
        ); \
        cmdBuffer.debugMarkerBeginEXT(markerInfo); \
    } \
    IDEFER( \
    if (_markersEnabled) \
    { \
        cmdBuffer.debugMarkerEndEXT(); \
    }); \
    void(0)
#define VULKAN_MARKER(engine, cmdBuffer, name) VULKAN_MARKER_COL(engine, cmdBuffer, name, 0, 0, 0)
#else
#define VULKAN_MARKER_COL(engine, cmdBuffer, name, r, g, b) void(0)
#define VULKAN_MARKER(engine, cmdBuffer, name) void(0)
#endif

// While there are existing functions seems to be inconsitent, therefore my own.
constexpr static const char* VkResultToString(vk::Result a_result)
{
    switch (a_result)
    {
    case vk::Result::eSuccess:
    {
        return "Vk::Success";
    }
    case vk::Result::eNotReady:
    {
        return "Vk::NotReady";
    }
    case vk::Result::eTimeout:
    {
        return "Vk::Timeout";
    }
    case vk::Result::eEventSet:
    {
        return "Vk::EventSet";
    }
    case vk::Result::eEventReset:
    {
        return "Vk::EventReset";
    }
    case vk::Result::eIncomplete:
    {
        return "Vk::Incomplete";
    }
    case vk::Result::eErrorOutOfHostMemory:
    {
        return "Vk::ErrorOutOfHostMemory";
    }
    case vk::Result::eErrorOutOfDeviceMemory:
    {
        return "Vk::ErrorOutOfDeviceMemory";
    }
    case vk::Result::eErrorInitializationFailed:
    {
        return "Vk::ErrorInitializationFailed";
    }
    case vk::Result::eErrorDeviceLost:
    {
        return "Vk::ErrorDeviceLost";
    }
    case vk::Result::eErrorMemoryMapFailed:
    {
        return "Vk::ErrorMemoryMapFailed";
    }
    case vk::Result::eErrorLayerNotPresent:
    {
        return "Vk::ErrorLayerNotPresent";
    }
    case vk::Result::eErrorExtensionNotPresent:
    {
        return "Vk::ErrorExtensionNotPresent";
    }
    case vk::Result::eErrorFeatureNotPresent:
    {
        return "Vk::ErrorFeatureNotPresent";
    }
    case vk::Result::eErrorIncompatibleDriver:
    {
        return "Vk::ErrorIncompatibleDriver";
    }
    case vk::Result::eErrorTooManyObjects:
    {
        return "Vk::ErrorTooManyObjects";
    }
    case vk::Result::eErrorFormatNotSupported:
    {
        return "Vk::ErrorFormatNotSupported";
    }
    case vk::Result::eErrorFragmentedPool:
    {
        return "Vk::ErrorFragmentedPool";
    }
    case vk::Result::eErrorUnknown:
    {
        return "Vk::ErrorUnknown";
    }
    case vk::Result::eErrorOutOfPoolMemory:
    {
        return "Vk::ErrorOutOfPoolMemory";
    }
    case vk::Result::eErrorInvalidExternalHandle:
    {
        return "Vk::eErrorInvalidExternalHandle";
    }
    case vk::Result::eErrorFragmentation:
    {
        return "Vk::ErrorFragmentation";
    }
    case vk::Result::eErrorInvalidOpaqueCaptureAddress:
    {
        return "Vk::ErrorInvalidOpaqueCaptureAddress";
    }
    case vk::Result::ePipelineCompileRequired:
    {
        return "Vk::PipelineCompileRequired";
    }
    case vk::Result::eErrorSurfaceLostKHR:
    {
        return "Vk::ErrorSurfaceLostKHR";
    }
    case vk::Result::eErrorNativeWindowInUseKHR:
    {
        return "Vk::ErrorNativeWindowInUseKHR";
    }
    case vk::Result::eSuboptimalKHR:
    {
        return "Vk::SuboptimalKHR";
    }
    case vk::Result::eErrorOutOfDateKHR:
    {
        return "Vk::ErrorOutOfDateKHR";
    }
    case vk::Result::eErrorIncompatibleDisplayKHR:
    {
        return "Vk::ErrorIncompatibleDisplayKHR";
    }
    case vk::Result::eErrorValidationFailedEXT:
    {
        return "Vk::ErrorValidationFailedEXT";
    }
    case vk::Result::eErrorInvalidShaderNV:
    {
        return "Vk::ErrorInvalidShaderNV";
    }
    case vk::Result::eErrorImageUsageNotSupportedKHR:
    {
        return "Vk::ErrorImageUsageNotSupportedKHR";
    }
    case vk::Result::eErrorVideoPictureLayoutNotSupportedKHR:
    {
        return "Vk::ErrorVideoPictureLayoutNotSupportedKHR";
    }
    case vk::Result::eErrorVideoProfileOperationNotSupportedKHR:
    {
        return "Vk::ErrorVideoProfileOperationNotSupportedKHR";
    }
    case vk::Result::eErrorVideoProfileFormatNotSupportedKHR:
    {
        return "Vk::ErrorVideoProfileFormatNotSupportedKHR";
    }
    case vk::Result::eErrorVideoProfileCodecNotSupportedKHR:
    {
        return "Vk::ErrorVideoProfileCodecNotSupportedKHR";
    }
    case vk::Result::eErrorVideoStdVersionNotSupportedKHR:
    {
        return "Vk::ErrorVideoStdVersionNotSupportedKHR";
    }
    case vk::Result::eErrorInvalidDrmFormatModifierPlaneLayoutEXT:
    {
        return "Vk::ErrorInvalidDrmFormatModifierPlaneLayoutEXT";
    }
    case vk::Result::eErrorNotPermittedKHR:
    {
        return "Vk::ErrorNotPermittedKHR";
    }
#ifdef VK_USE_PLATFORM_WIN32_KHR
    case vk::Result::eErrorFullScreenExclusiveModeLostEXT:
    {
        return "Vk::ErrorFullScreenExclusiveModeLostEXT";
    }
#endif
    case vk::Result::eThreadIdleKHR:
    {
        return "Vk::ThreadIdleKHR";
    }
    case vk::Result::eThreadDoneKHR:
    {
        return "Vk::ThreadDoneKHR";
    }
    case vk::Result::eOperationDeferredKHR:
    {
        return "Vk::OperationDeferredKHR";
    }
    case vk::Result::eOperationNotDeferredKHR:
    {
        return "Vk::OperationNotDeferredKHR";
    }
    case vk::Result::eErrorInvalidVideoStdParametersKHR:
    {
        return "Vk::ErrorInvalidVideoStdParametersKHR";
    }
    case vk::Result::eErrorCompressionExhaustedEXT:
    {
        return "Vk::ErrorCompressionExhaustedEXT";
    }
    case vk::Result::eIncompatibleShaderBinaryEXT:
    {
        return "Vk::IncompatibleShaderBinaryEXT";
    }
    default:
    {
        return "Vk::InvalidResult";
    }
    }

    ICARIAN_ASSERT(0);

    return "Vk::ErrorResult";
}

static constexpr const char* VulkanErrorPrefix = "VkError: ";

#define IVKSTRR(v) #v
#define IVKSTR(v) IVKSTRR(v)
#define VKRESWARN(res) VulkanResultWarning((vk::Result)res, IVKSTR(__FILE__) "," IVKSTR(__LINE__))
#define VKRESWARNMSG(res, msg) VulkanResultWarning((vk::Result)res, IcarianCore::COWU8String(msg, IcarianCore::MallocAllocator::Instance) + ": " IVKSTR(__FILE__) "," IVKSTR(__LINE__))
#define VKRESERR(res) VulkanResultError((vk::Result)res, IVKSTR(__FILE__) "," IVKSTR(__LINE__))
#define VKRESERRMSG(res, msg) VulkanResultError((vk::Result)res, IcarianCore::COWU8String(msg, IcarianCore::MallocAllocator::Instance) + ": " IVKSTR(__FILE__) "," IVKSTR(__LINE__))

[[maybe_unused]] static void VulkanResultWarning(vk::Result a_result, const IcarianCore::COWU8String& a_msg)
{
    if (a_result != vk::Result::eSuccess)
    {
        IWARN(VulkanErrorPrefix + a_msg + " " + VkResultToString(a_result));
    }
}
[[maybe_unused]] static void VulkanResultWarning(vk::Result a_result, const char* a_msg)
{
    if (a_result != vk::Result::eSuccess)
    {
        const IcarianCore::COWU8String msg = IcarianCore::COWU8String(a_msg, IcarianCore::MallocAllocator::Instance);

        VulkanResultWarning(a_result, msg);
    }
}

[[maybe_unused]] static void VulkanResultError(vk::Result a_result, const IcarianCore::COWU8String& a_msg)
{
    if (a_result != vk::Result::eSuccess)
    {
        IERROR(VulkanErrorPrefix + a_msg + " " + VkResultToString(a_result));
    }
}
[[maybe_unused]] static void VulkanResultError(vk::Result a_result, const char* a_msg)
{
    if (a_result != vk::Result::eSuccess)
    {
        const IcarianCore::COWU8String msg = IcarianCore::COWU8String(a_msg, IcarianCore::MallocAllocator::Instance);

        VulkanResultError(a_result, msg);
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
