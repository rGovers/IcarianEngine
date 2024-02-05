#pragma once

#ifdef ICARIANNATIVE_ENABLE_GRAPHICS_VULKAN
#include <string_view>

#include "VulkanShader.h"

class VulkanComputeShader : public VulkanShader
{
private:

protected:

public:
    VulkanComputeShader() = delete;
    VulkanComputeShader(VulkanRenderEngineBackend* a_engine, const std::vector<uint32_t>& a_data);
    ~VulkanComputeShader();

    static VulkanComputeShader* CreateFromFShader(VulkanRenderEngineBackend* a_engine, const std::string_view& a_str);
    static VulkanComputeShader* CreateFromGLSL(VulkanRenderEngineBackend* a_engine, const std::string_view& a_str);
};

#endif