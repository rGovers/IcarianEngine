#ifdef ICARIANNATIVE_ENABLE_GRAPHICS_VULKAN

#include "Rendering/Vulkan/VulkanComputeShader.h"

#include "Core/IcarianAssert.h"
#include "Core/FlareShader.h"
#include "Rendering/SPIRVTools.h"
#include "Rendering/Vulkan/VulkanRenderEngineBackend.h"
#include "Trace.h"

VulkanComputeShader::VulkanComputeShader(VulkanRenderEngineBackend* a_engine, const ShaderBufferInput* a_inputs, uint32_t a_inputCount, const std::vector<uint32_t>& a_data) : VulkanShader(a_engine, a_inputs, a_inputCount)
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
    std::string error;
    std::vector<ShaderBufferInput> inputs;
    const std::string glsl = IcarianCore::GLSLFromFlareShader(a_str, IcarianCore::ShaderPlatform_Vulkan, &inputs, &error);
    if (glsl.empty())
    {
        IERROR("Flare compute shader error: " + error);

        return nullptr;
    }

    return CreateFromGLSL(a_engine, inputs.data(), (uint32_t)inputs.size(), glsl);
}
VulkanComputeShader* VulkanComputeShader::CreateFromGLSL(VulkanRenderEngineBackend* a_engine, const ShaderBufferInput* a_inputs, uint32_t a_inputCount, const std::string_view& a_str)
{
    ICARIAN_ASSERT_MSG(!a_str.empty(), "Empty compute shader string");

    const std::vector<uint32_t> spirv = spirv_fromGLSL(EShLangCompute, a_str, true);
    if (spirv.empty())
    {
        IERROR("Failed to compile compute shader");

        return nullptr;
    }

    return new VulkanComputeShader(a_engine, a_inputs, a_inputCount, spirv);
}

#endif