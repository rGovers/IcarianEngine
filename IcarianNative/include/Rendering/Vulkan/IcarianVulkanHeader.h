#pragma once

#ifdef ICARIANNATIVE_ENABLE_GRAPHICS_VULKAN
#include <vulkan/vulkan.hpp>

#define ICARIAN_VMA_VULKAN_VERSION 1001000
#define ICARIAN_VULKAN_VERSION VK_API_VERSION_1_1

#define VMA_VULKAN_VERSION ICARIAN_VMA_VULKAN_VERSION
#include <vk_mem_alloc.h>

#include "IcarianError.h"

static constexpr uint32_t VulkanMaxFlightFrames = 2;
static constexpr uint32_t VulkanFlightPoolSize = VulkanMaxFlightFrames + 1;
static constexpr uint32_t VulkanDeletionQueueSize = VulkanFlightPoolSize + 1;

#ifdef NDEBUG
static constexpr bool VulkanEnableValidationLayers = false;
#else
static constexpr bool VulkanEnableValidationLayers = true;
#endif

// While there are existing functions seems to be inconsitent, therefore my own.
static std::string VkResultToString(vk::Result a_result)
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
        return "Vk::InvalidResult: " + std::to_string((uint32_t)a_result);
    }
    }

    ICARIAN_ASSERT(0);

    return "Vk::ErrorResult: " +  std::to_string((uint32_t)a_result);;
}

static constexpr const char* VulkanErrorPrefix = "VkError: ";

#define IVKSTRR(v) #v
#define IVKSTR(v) IVKSTRR(v)
#define VKRESWARN(res) VulkanResultWarning((vk::Result)res, IVKSTR(__FILE__) "," IVKSTR(__LINE__))
#define VKRESWARNMSG(res, msg) VulkanResultWarning((vk::Result)res, std::string(msg) + ": " IVKSTR(__FILE__) "," IVKSTR(__LINE__))
#define VKRESERR(res) VulkanResultError((vk::Result)res, IVKSTR(__FILE__) "," IVKSTR(__LINE__))
#define VKRESERRMSG(res, msg) VulkanResultError((vk::Result)res, std::string(msg) + ": " IVKSTR(__FILE__) "," IVKSTR(__LINE__))

static void VulkanResultWarning(vk::Result a_result, const std::string_view& a_msg = "")
{
    if (a_result != vk::Result::eSuccess)
    {
        IWARN(VulkanErrorPrefix + std::string(a_msg) + " " + VkResultToString(a_result));
    }
}

static void VulkanResultError(vk::Result a_result, const std::string_view& a_msg = "")
{
    if (a_result != vk::Result::eSuccess)
    {
        IERROR(VulkanErrorPrefix + std::string(a_msg) + " " + VkResultToString(a_result));
    }
}

#endif
