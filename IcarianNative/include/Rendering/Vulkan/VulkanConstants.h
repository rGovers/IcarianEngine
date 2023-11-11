#pragma once

#ifdef ICARIANNATIVE_ENABLE_GRAPHICS_VULKAN
#include <vulkan/vulkan.hpp>

#include <spirv-tools/libspirv.hpp>

#define ICARIAN_VMA_VULKAN_VERSION 1000000
#define ICARIAN_VULKAN_VERSION VK_API_VERSION_1_0

#define VMA_VULKAN_VERSION ICARIAN_VMA_VULKAN_VERSION
#include <vk_mem_alloc.h>

static constexpr uint32_t VulkanMaxFlightFrames = 2;
static constexpr uint32_t VulkanFlightPoolSize = VulkanMaxFlightFrames + 1;
static constexpr uint32_t VulkanDeletionQueueSize = VulkanFlightPoolSize + 1;

static constexpr spv_target_env VulkanShaderTarget = SPV_ENV_VULKAN_1_0;

#ifdef NDEBUG
static constexpr bool VulkanEnableValidationLayers = false;
#else
static constexpr bool VulkanEnableValidationLayers = true;
#endif
#endif