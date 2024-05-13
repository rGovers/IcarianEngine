#ifdef ICARIANNATIVE_ENABLE_GRAPHICS_VULKAN

#include "Rendering/Vulkan/VulkanPixelShader.h"

#include "Core/IcarianAssert.h"
#include "Core/FlareShader.h"
#include "Rendering/SPIRVTools.h"
#include "Rendering/Vulkan/VulkanRenderEngineBackend.h"
#include "Trace.h"

VulkanPixelShader::VulkanPixelShader(VulkanRenderEngineBackend* a_engine, const ShaderBufferInput* a_inputs, uint32_t a_inputCount, const std::vector<uint32_t>& a_data) : VulkanShader(a_engine, a_inputs, a_inputCount)
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
    std::string error;
    std::vector<ShaderBufferInput> inputs;
    const std::string glsl = IcarianCore::GLSLFromFlareShader(a_str, IcarianCore::ShaderPlatform_Vulkan, &inputs, &error);

    if (glsl.empty())
    {
        IERROR("Flare pixel shader error: " + error);

        return nullptr;
    }

    return CreateFromGLSL(a_engine, inputs.data(), (uint32_t)inputs.size(), glsl);
}
VulkanPixelShader* VulkanPixelShader::CreateFromGLSL(VulkanRenderEngineBackend* a_engine, const ShaderBufferInput* a_inputs, uint32_t a_inputCount, const std::string_view& a_str)
{
    const std::vector<uint32_t> spirv = spirv_fromGLSL(EShLangFragment, a_str, true);
    if (spirv.empty())
    {
        IERROR("Failed to compile pixel shader");

        return nullptr;
    }

    return new VulkanPixelShader(a_engine, a_inputs, a_inputCount, spirv);
}
#endif