// Icarian Engine - C# Game Engine
// 
// License at end of file.

#ifdef ICARIANNATIVE_ENABLE_GRAPHICS_VULKAN

#include "Rendering/Vulkan/Shaders/VulkanComputeShader.h"

#include "Core/FlareShader.h"
#include "Rendering/SPIRVTools.h"
#include "Rendering/Vulkan/VulkanRenderEngineBackend.h"
#include "Trace.h"

VulkanComputeShader::VulkanComputeShader(VulkanRenderEngineBackend* a_engine, uint32_t a_workgroupX, uint32_t a_workgroupY, uint32_t a_workgroupZ, const ShaderBufferInput* a_inputs, uint32_t a_inputCount, const std::vector<uint32_t>& a_data, Allocator* a_allocator) : VulkanShader(a_engine, a_inputs, a_inputCount, a_allocator)
{
    TRACE("Creating ComputeShader");
    const vk::Device device = m_engine->GetLogicalDevice();

    m_workgroupX = a_workgroupX;
    m_workgroupY = a_workgroupY;
    m_workgroupZ = a_workgroupZ;

    const vk::ShaderModuleCreateInfo createInfo = vk::ShaderModuleCreateInfo
    (
        { },
        a_data.size() * sizeof(uint32_t),
        (uint32_t*)a_data.data()
    );

    VKRESERRMSG(device.createShaderModule(&createInfo, nullptr, &m_module), "Failed to create ComputeShader");
}
VulkanComputeShader::~VulkanComputeShader()
{
    const vk::Device device = m_engine->GetLogicalDevice();

    device.destroyShaderModule(m_module);
}

void VulkanComputeShader::CreateFromFShader(VulkanComputeShader* a_out, const VulkanComputeFShaderBuilder& a_builder, Allocator* a_allocator)
{
    IVERIFY(a_out != nullptr);
    IVERIFY(a_builder.Engine != nullptr);
    IVERIFY(!a_builder.EntryPoint.empty());
    IVERIFY(!a_builder.String.empty());

    std::string error;
    std::vector<ShaderBufferInput> inputs;
    IcarianCore::ShaderWorkgroups workgroups;
    const std::string str = IcarianCore::GLSLFromFlareShader(a_builder.String, IcarianCore::ShaderPlatform_VulkanCompute, a_builder.Imports, &inputs, &error, &workgroups);
    if (str.empty())
    {
        IERROR("Flare Compute shader error: " + error);
    }

    const VulkanComputeGLSLShaderBuilder glslBuilder =
    {
        .Engine = a_builder.Engine,
        .String = str,
        .WorkgroupX = workgroups.GroupX,
        .WorkgroupY = workgroups.GroupY,
        .WorkgroupZ = workgroups.GroupZ,
        .Inputs = inputs.data(),
        .InputCount = (uint32_t)inputs.size(),
        .EntryPoint = a_builder.EntryPoint,
    };

    return CreateFromGLSL(a_out, glslBuilder, a_allocator);
}
void VulkanComputeShader::CreateFromGLSL(VulkanComputeShader* a_out, const VulkanComputeGLSLShaderBuilder& a_builder, Allocator* a_allocator)
{
    IVERIFY(a_out != nullptr);
    IVERIFY(a_builder.Engine != nullptr);
    IVERIFY(!a_builder.EntryPoint.empty());
    IVERIFY(!a_builder.String.empty());

    const std::vector<uint32_t> spirv = spirv_fromGLSL(EShLangCompute, a_builder.String, true, a_builder.EntryPoint);
    if (spirv.empty())
    {
        IERROR("Failed to compile Compute shader");
    }

    new (a_out) VulkanComputeShader(a_builder.Engine, a_builder.WorkgroupX, a_builder.WorkgroupY, a_builder.WorkgroupZ, a_builder.Inputs, a_builder.InputCount, spirv, a_allocator);
}

#endif

// MIT License
// 
// Copyright (c) 2025 River Govers
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