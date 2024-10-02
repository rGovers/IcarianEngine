// Icarian Engine - C# Game Engine
// 
// License at end of file.

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

VulkanComputeShader* VulkanComputeShader::CreateFromFShader(VulkanRenderEngineBackend* a_engine, const std::unordered_map<std::string, std::string>& a_imports, const std::string_view& a_str)
{
    std::string error;
    std::vector<ShaderBufferInput> inputs;
    const std::string glsl = IcarianCore::GLSLFromFlareShader(a_str, IcarianCore::ShaderPlatform_Vulkan, a_imports, &inputs, &error);
    if (glsl.empty())
    {
        IERROR("Flare compute shader error: " + error);

        return nullptr;
    }

    return CreateFromGLSL(a_engine, inputs.data(), (uint32_t)inputs.size(), glsl);
}
VulkanComputeShader* VulkanComputeShader::CreateFromGLSL(VulkanRenderEngineBackend* a_engine, const ShaderBufferInput* a_inputs, uint32_t a_inputCount, const std::string_view& a_str)
{
    IVERIFY(!a_str.empty());

    const bool spirv14 = a_engine->IsExtensionEnabled(VK_KHR_SPIRV_1_4_EXTENSION_NAME);

    const std::vector<uint32_t> spirv = spirv_fromGLSL(EShLangCompute, a_str, spirv14, true);
    if (spirv.empty())
    {
        IERROR("Failed to compile compute shader");

        return nullptr;
    }

    return new VulkanComputeShader(a_engine, a_inputs, a_inputCount, spirv);
}

#endif

// MIT License
// 
// Copyright (c) 2024 River Govers
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.