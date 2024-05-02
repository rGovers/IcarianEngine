#pragma once

#ifdef ICARIANNATIVE_ENABLE_GRAPHICS_VULKAN
#include <string_view>

#include "Rendering/Vulkan/VulkanShader.h"

class VulkanPixelShader : public VulkanShader
{
private:
    
protected:

public:
    VulkanPixelShader() = delete;
    VulkanPixelShader(VulkanRenderEngineBackend* a_engine, const ShaderBufferInput* a_inputs, uint32_t a_inputCount, const std::vector<uint32_t>& a_data);
    virtual ~VulkanPixelShader();

    static VulkanPixelShader* CreateFromFShader(VulkanRenderEngineBackend* a_engine, const std::string_view& a_str);
    static VulkanPixelShader* CreateFromGLSL(VulkanRenderEngineBackend* a_engine, const ShaderBufferInput* a_inputs, uint32_t a_inputCount, const std::string_view& a_str);
};
#endif