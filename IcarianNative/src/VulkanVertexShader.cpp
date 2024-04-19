#ifdef ICARIANNATIVE_ENABLE_GRAPHICS_VULKAN

#include "Rendering/Vulkan/VulkanVertexShader.h"

#include "Core/IcarianAssert.h"
#include "Rendering/SPIRVTools.h"
#include "Rendering/Vulkan/VulkanRenderEngineBackend.h"
#include "Trace.h"

VulkanVertexShader::VulkanVertexShader(VulkanRenderEngineBackend* a_engine, const std::vector<uint32_t>& a_data) : VulkanShader(a_engine)
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
    return CreateFromGLSL(a_engine, GLSL_fromFShader(a_str));
}
VulkanVertexShader* VulkanVertexShader::CreateFromGLSL(VulkanRenderEngineBackend* a_engine, const std::string_view& a_str)
{
    const std::vector<uint32_t> spirv = spirv_fromGLSL(EShLangVertex, a_str, true);
    
    ICARIAN_ASSERT_MSG_R(!spirv.empty(), "Failed to generate Vertex Spirv");

    return new VulkanVertexShader(a_engine, spirv);
}
#endif