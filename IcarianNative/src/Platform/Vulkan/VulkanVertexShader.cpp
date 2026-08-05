// Icarian Engine - C# Game Engine
//
// License at end of file.

#ifdef ICARIANNATIVE_ENABLE_GRAPHICS_VULKAN

#include "Rendering/Vulkan/Shaders/VulkanVertexShader.h"

#include "Core/DataTypes/Allocators/LeakAllocator.h"
#include "Core/DataTypes/Allocators/MultiSourceAllocator.h"
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
    IcarianCore::Allocator* a_allocator
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

void VulkanVertexShader::CreateFromFShader
(
    VulkanVertexShader* a_out,
    const VulkanVertexFShaderBuilder& a_builder,
    IcarianCore::Allocator* a_allocator,
    IcarianCore::Allocator* a_tempAllocator
)
{
    IVERIFY(a_out != nullptr);
    IVERIFY(a_builder.Engine != nullptr);
    IVERIFY(!a_builder.String.Empty());
    IVERIFY(!a_builder.EntryPoint.Empty());

    // We will be doing a bunch of string ops so do not want to use the temp allocator directly
    // Temp allocator is not setup for alot of allocation/deallocation so wrap it in a BlockAllocator to better manage the memory
    constexpr uint32_t BlockSize = 32 << 10;
    IcarianCore::BlockAllocator tempBlock = IcarianCore::BlockAllocator(BlockSize, a_tempAllocator);

    const IcarianCore::AllocationSource allocatorSources[] =
    {
        {
            .Alloc = &tempBlock,
            .MaxSize = BlockSize >> 1,
        },
        {
            // Fallback to the render system allocator if the allocation is too large
            .Alloc = a_allocator,
            .MaxSize = uint64_t(-1)
        }
    };
    constexpr uint32_t AllocatorSourceCount = sizeof(allocatorSources) / sizeof(*allocatorSources);

    IcarianCore::Allocator* tempAllocator = a_tempAllocator->Create<IcarianCore::MultiSourceAllocator>
    (
        &tempBlock,
        allocatorSources,
        AllocatorSourceCount
    );
#ifdef DEBUG
    tempAllocator = a_tempAllocator->Create<IcarianCore::LeakAllocator>(tempAllocator);
    IDEFER(
    {
        IcarianCore::Allocator* upstreamAllocator = ((IcarianCore::LeakAllocator*)tempAllocator)->GetUpstreamAllocator();
        a_tempAllocator->Destroy(tempAllocator);
        a_tempAllocator->Destroy(upstreamAllocator);
    });
#else
    IDEFER(a_tempAllocator->Destroy(tempAllocator));
#endif

    IcarianCore::Array<ShaderBufferInput> inputs = IcarianCore::Array<ShaderBufferInput>(a_tempAllocator);

    const FlareShader::ShaderBuilder builder =
    {
        .String = a_builder.String,
        .Platform = FlareShader::ShaderPlatform_Vulkan,
        .Imports = a_builder.Imports,
        .Inputs = &inputs,
        .OtherInputs = a_builder.OtherInputs,
    };

    IcarianCore::COWU8String shader = IcarianCore::COWU8String(a_tempAllocator);
    const IcarianCore::COWU8String error = FlareShader::GLSLFromFlareShader(&shader, builder, a_allocator, tempAllocator);
    if (!error.Empty())
    {
        IERROR("Flare Vertex Shader generation error: " + error);
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
void VulkanVertexShader::CreateFromGLSL
(
    VulkanVertexShader* a_out,
    const VulkanVertexGLSLShaderBuilder& a_builder,
    IcarianCore::Allocator* a_allocator,
    IcarianCore::Allocator* a_tempAllocator
)
{
    IVERIFY(a_out != nullptr);
    IVERIFY(a_builder.Engine != nullptr);
    IVERIFY(!a_builder.EntryPoint.Empty());
    IVERIFY(!a_builder.String.Empty());

    const IcarianCore::Array<uint32_t> spirv = spirv_fromGLSL
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
