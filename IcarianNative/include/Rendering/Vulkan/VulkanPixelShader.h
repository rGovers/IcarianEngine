#pragma once

#include <string_view>

#include "Rendering/Vulkan/VulkanShader.h"

class VulkanPixelShader : public VulkanShader
{
private:
    
protected:

public:
    VulkanPixelShader() = delete;
    VulkanPixelShader(VulkanRenderEngineBackend* a_engine, const std::vector<unsigned int>& a_data);
    virtual ~VulkanPixelShader();

    static VulkanPixelShader* CreateFromGLSL(VulkanRenderEngineBackend* a_engine, const std::string_view& a_str);
    static VulkanPixelShader* CreateFromFShader(VulkanRenderEngineBackend* a_engine, const std::string_view& a_str);
};