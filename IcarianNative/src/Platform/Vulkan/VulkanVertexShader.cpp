// Icarian Engine - C# Game Engine
// 
// License at end of file.

#ifdef ICARIANNATIVE_ENABLE_GRAPHICS_VULKAN

#include "Rendering/Vulkan/Shaders/VulkanVertexShader.h"

#include "Rendering/FlareShader.h"
#include "Rendering/SPIRVTools.h"
#include "Rendering/Vulkan/VulkanRenderEngineBackend.h"
#include "Trace.h"

VulkanVertexShader::VulkanVertexShader
(
    VulkanRenderEngineBackend* a_engine,
    const ShaderBufferInput* a_inputs,
    uint32_t a_inputCount,
    const uint32_t* a_data,
    uint32_t a_dataCount,
    Allocator* a_allocator
) : VulkanShader(a_engine, a_inputs, a_inputCount, a_allocator)
{
    TRACE("Creating Vertex Shader");
    const vk::Device device = m_engine->GetLogicalDevice();

    const vk::ShaderModuleCreateInfo createInfo = vk::ShaderModuleCreateInfo
    (
        { },
        a_dataCount * sizeof(uint32_t),
        a_data
    );

    VKRESERRMSG(device.createShaderModule(&createInfo, nullptr, &m_module), "Failed to create VertexShader");
}
VulkanVertexShader::~VulkanVertexShader()
{
    const vk::Device device = m_engine->GetLogicalDevice();

    device.destroyShaderModule(m_module);
}

void VulkanVertexShader::CreateFromFShader(VulkanVertexShader* a_out, const VulkanVertexFShaderBuilder& a_builder, Allocator* a_allocator, Allocator* a_tempAllocator)
{
    IVERIFY(a_out != nullptr);
    IVERIFY(a_builder.Engine != nullptr);
    IVERIFY(!a_builder.String.Empty());
    IVERIFY(!a_builder.EntryPoint.Empty());

    Array<ShaderBufferInput> inputs = Array<ShaderBufferInput>(a_tempAllocator);

    const FlareShader::ShaderBuilder builder =
    {
        .String = a_builder.String,
        .Platform = FlareShader::ShaderPlatform_Vulkan,
        .Imports = a_builder.Imports,
        .Inputs = &inputs,
        .OtherInputs = a_builder.OtherInputs,
    };

    COWU8String shader = COWU8String(a_tempAllocator);
    // TODO: Probably want to change the allocators so they can take other allocators rather then passing through the main allocator for both
    // Probably better to use the temp allocator as a memory block for another BlockAllocator as string ops are a mess for allocators
    // Basically if we need a specific allocator we should still use the provided one but as a memory block for the allocator we require
    // Regardless this needs to change as the user will be unaware that the main allocator is being used for temp allocations
    const COWU8String error = FlareShader::GLSLFromFlareShader(&shader, builder, a_allocator, a_allocator);
    if (!error.Empty())
    {
        IERROR(std::string("Flare Vertex Shader generation error: ") + error.CStr());
    }

    const VulkanVertexGLSLShaderBuilder glslBuilder =
    {
        .Engine = a_builder.Engine,
        .String = shader,
        .Inputs = inputs.Data(),
        .InputCount = inputs.Size(),
        .EntryPoint = a_builder.EntryPoint,
    };

    CreateFromGLSL(a_out, glslBuilder, a_allocator, a_tempAllocator);
}
void VulkanVertexShader::CreateFromGLSL(VulkanVertexShader* a_out, const VulkanVertexGLSLShaderBuilder& a_builder, Allocator* a_allocator, Allocator* a_tempAllocator)
{
    IVERIFY(a_out != nullptr);
    IVERIFY(a_builder.Engine != nullptr);
    IVERIFY(!a_builder.EntryPoint.Empty());
    IVERIFY(!a_builder.String.Empty());

    const Array<uint32_t> spirv = spirv_fromGLSL
    (
        EShLangVertex,
        a_builder.String,
        true,
        a_builder.EntryPoint,
        a_tempAllocator
    );
    if (spirv.Empty())
    {
        IERROR("Failed to compile Vertex Shader");
    }

    new (a_out) VulkanVertexShader
    (
        a_builder.Engine,
        a_builder.Inputs,
        a_builder.InputCount,
        spirv.Data(),
        (uint32_t)spirv.Size(),
        a_allocator
    );
}

#endif

// MIT License
// 
// Copyright (c) 2026 River Govers
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