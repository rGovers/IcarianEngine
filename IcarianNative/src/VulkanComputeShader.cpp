#ifdef ICARIANNATIVE_ENABLE_GRAPHICS_VULKAN

#include "Rendering/Vulkan/VulkanComputeShader.h"

#include "Flare/IcarianAssert.h"
#include "Logger.h"
#include "Rendering/SPIRVTools.h"
#include "Rendering/Vulkan/VulkanRenderEngineBackend.h"
#include "Trace.h"

VulkanComputeShader::VulkanComputeShader(VulkanRenderEngineBackend* a_engine, const std::vector<uint32_t>& a_data) : VulkanShader(a_engine)
{
    TRACE("Creating ComputeShader");
    
    const vk::Device device = m_engine->GetLogicalDevice();

    const vk::ShaderModuleCreateInfo createInfo = vk::ShaderModuleCreateInfo
    (
        { },
        a_data.size() * sizeof(uint32_t),
        (uint32_t*)a_data.data()
    );

    ICARIAN_ASSERT_MSG_R(device.createShaderModule(&createInfo, nullptr, &m_module) == vk::Result::eSuccess, "Failed to create ComputeShader");
}
VulkanComputeShader::~VulkanComputeShader()
{
    const vk::Device device = m_engine->GetLogicalDevice();

    device.destroyShaderModule(m_module);
}

VulkanComputeShader* VulkanComputeShader::CreateFromFShader(VulkanRenderEngineBackend* a_engine, const std::string_view& a_str)
{
    return CreateFromGLSL(a_engine, GLSL_fromFShader(a_str));
}
VulkanComputeShader* VulkanComputeShader::CreateFromGLSL(VulkanRenderEngineBackend* a_engine, const std::string_view& a_str)
{
    ICARIAN_ASSERT_MSG(!a_str.empty(), "Empty compute shader string");

    const std::vector<uint32_t> spirv = spirv_fromGLSL(EShLangCompute, a_str, true);
    
    ICARIAN_ASSERT_MSG_R(!spirv.empty(), "Failed to generate Compute Spirv");

    return new VulkanComputeShader(a_engine, spirv);
}

#endif