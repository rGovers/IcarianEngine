#ifdef ICARIANNATIVE_ENABLE_GRAPHICS_VULKAN

#include "Rendering/Vulkan/VulkanVertexShader.h"

#include "Core/IcarianAssert.h"
#include "Core/FlareShader.h"
#include "Rendering/SPIRVTools.h"
#include "Rendering/Vulkan/VulkanRenderEngineBackend.h"
#include "Trace.h"

VulkanVertexShader::VulkanVertexShader(VulkanRenderEngineBackend* a_engine, const ShaderBufferInput* a_inputs, uint32_t a_inputCount, const std::vector<uint32_t>& a_data) : VulkanShader(a_engine, a_inputs, a_inputCount)
{
    const vk::Device device = m_engine->GetLogicalDevice();

    const vk::ShaderModuleCreateInfo createInfo = vk::ShaderModuleCreateInfo
    (
        { },
        a_data.size() * sizeof(uint32_t),
        (uint32_t*)a_data.data()
    );

    ICARIAN_ASSERT_MSG_R(device.createShaderModule(&createInfo, nullptr, &m_module) == vk::Result::eSuccess, "Failed to create VertexShader");

    TRACE("Created VertexShader");
}
VulkanVertexShader::~VulkanVertexShader()
{
    const vk::Device device = m_engine->GetLogicalDevice();

    device.destroyShaderModule(m_module);
}

VulkanVertexShader* VulkanVertexShader::CreateFromFShader(VulkanRenderEngineBackend* a_engine, const std::string_view& a_str)
{
    std::string error;
    std::vector<ShaderBufferInput> inputs;
    const std::string glsl = IcarianCore::GLSLFromFlareShader(a_str, IcarianCore::ShaderPlatform_Vulkan, &inputs, &error);

    if (glsl.empty())
    {
        IERROR("Flare vertex shader error: " + error);

        return nullptr;
    }

    return CreateFromGLSL(a_engine, inputs.data(), (uint32_t)inputs.size(), glsl);
}
VulkanVertexShader* VulkanVertexShader::CreateFromGLSL(VulkanRenderEngineBackend* a_engine, const ShaderBufferInput* a_inputs, uint32_t a_inputCount, const std::string_view& a_str)
{
    const std::vector<uint32_t> spirv = spirv_fromGLSL(EShLangVertex, a_str, true);
    if (spirv.empty())
    {
        IERROR("Failed to compile vertex shader");

        return nullptr;
    }    

    return new VulkanVertexShader(a_engine, a_inputs, a_inputCount, spirv);
}
#endif