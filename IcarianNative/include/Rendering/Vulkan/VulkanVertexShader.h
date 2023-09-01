#pragma once

#ifdef ICARIANNATIVE_ENABLE_GRAPHICS_VULKAN
#include <string_view>

#include "VulkanShader.h"

class VulkanVertexShader : public VulkanShader
{
private:

protected:

public:
    VulkanVertexShader() = delete;
    VulkanVertexShader(VulkanRenderEngineBackend* a_engine, const std::vector<unsigned int>& a_data);
    virtual ~VulkanVertexShader();

    static VulkanVertexShader* CreateFromGLSL(VulkanRenderEngineBackend* a_engine, const std::string_view& a_str);
    static VulkanVertexShader* CreateFromFShader(VulkanRenderEngineBackend* a_engine, const std::string_view& a_str);
};
#endif