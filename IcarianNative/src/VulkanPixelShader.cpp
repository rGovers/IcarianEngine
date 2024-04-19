#ifdef ICARIANNATIVE_ENABLE_GRAPHICS_VULKAN

#include "Rendering/Vulkan/VulkanPixelShader.h"

#include "Core/IcarianAssert.h"
#include "Rendering/SPIRVTools.h"
#include "Rendering/Vulkan/VulkanRenderEngineBackend.h"
#include "Trace.h"

VulkanPixelShader::VulkanPixelShader(VulkanRenderEngineBackend* a_engine, const std::vector<uint32_t>& a_data) : VulkanShader(a_engine)
{
    const vk::Device device = m_engine->GetLogicalDevice();

    const vk::ShaderModuleCreateInfo createInfo = vk::ShaderModuleCreateInfo
    (
        vk::ShaderModuleCreateFlags(),
        a_data.size() * sizeof(uint32_t),
        (uint32_t*)a_data.data()
    );

    ICARIAN_ASSERT_MSG_R(device.createShaderModule(&createInfo, nullptr, &m_module) == vk::Result::eSuccess, "Failed to create PixelShader");

    TRACE("Created PixelShader");
}
VulkanPixelShader::~VulkanPixelShader()
{
    const vk::Device device = m_engine->GetLogicalDevice();

    device.destroyShaderModule(m_module);
}

VulkanPixelShader* VulkanPixelShader::CreateFromFShader(VulkanRenderEngineBackend* a_engine, const std::string_view& a_str)
{
    return CreateFromGLSL(a_engine, GLSL_fromFShader(a_str));
}
VulkanPixelShader* VulkanPixelShader::CreateFromGLSL(VulkanRenderEngineBackend* a_engine, const std::string_view& a_str)
{
    const std::vector<uint32_t> spirv = spirv_fromGLSL(EShLangFragment, a_str, true);
    
    ICARIAN_ASSERT_MSG_R(!spirv.empty(), "Failed to generate Pixel Spirv");

    return new VulkanPixelShader(a_engine, spirv);
}
#endif