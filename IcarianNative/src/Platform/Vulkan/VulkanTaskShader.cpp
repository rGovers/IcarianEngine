// Icarian Engine - C# Game Engine
// 
// License at end of file.

#ifdef ICARIANNATIVE_ENABLE_GRAPHICS_VULKAN

#include "Rendering/Vulkan/Shaders/VulkanTaskShader.h"

#include "DataTypes/Allocators/LeakAllocator.h"
#include "DataTypes/Allocators/MultiSourceAllocator.h"
#include "Rendering/FlareShader.h"
#include "Rendering/SPIRVTools.h"
#include "Rendering/Vulkan/VulkanRenderEngineBackend.h"
#include "Trace.h"

VulkanTaskShader::VulkanTaskShader
(
    VulkanRenderEngineBackend* a_engine,
    const ShaderBufferInput* a_inputs,
    uint32_t a_inputCount,
    const uint32_t* a_data,
    uint32_t a_dataCount,
    Allocator* a_allocator
) : VulkanShader(a_engine, a_inputs, a_inputCount, a_allocator)
{
    const vk::Device device = m_engine->GetLogicalDevice();

    const vk::ShaderModuleCreateInfo createInfo = vk::ShaderModuleCreateInfo
    (
        { },
        a_dataCount * sizeof(uint32_t),
        a_data
    );

    VKRESERRMSG(device.createShaderModule(&createInfo, nullptr, &m_module), "Failed to create TaskShader");

    TRACE("Created TaskShader");
}
VulkanTaskShader::~VulkanTaskShader()
{
    const vk::Device device = m_engine->GetLogicalDevice();

    device.destroyShaderModule(m_module);
}

void VulkanTaskShader::CreateFromFShader(VulkanTaskShader* a_out, const VulkanTaskFShaderBuilder& a_builder, Allocator* a_allocator, Allocator* a_tempAllocator)
{
    IVERIFY(a_out != nullptr);
    IVERIFY(a_builder.Engine != nullptr);
    IVERIFY(!a_builder.EntryPoint.Empty());
    IVERIFY(!a_builder.String.Empty());

    // We will be doing a bunch of string ops so do not want to use the temp allocator directly
    // Temp allocator is not setup for alot of allocation/deallocation so wrap it in a BlockAllocator to better manage the memory
    constexpr uint32_t BlockSize = 32 << 10;
    BlockAllocator tempBlock = BlockAllocator(BlockSize, a_tempAllocator);

    const AllocationSource allocatorSources[] =
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

    Allocator* tempAllocator = a_tempAllocator->Create<MultiSourceAllocator>
    (
        &tempBlock,
        allocatorSources,
        AllocatorSourceCount
    );
#ifdef DEBUG
    tempAllocator = a_tempAllocator->Create<LeakAllocator>(tempAllocator);
    IDEFER(
    {
        Allocator* upstreamAllocator = ((LeakAllocator*)tempAllocator)->GetUpstreamAllocator();
        a_tempAllocator->Destroy(tempAllocator);
        a_tempAllocator->Destroy(upstreamAllocator);
    });
#else
    IDEFER(a_tempAllocator->Destroy(tempAllocator));
#endif

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
    const COWU8String error = FlareShader::GLSLFromFlareShader(&shader, builder, a_allocator, tempAllocator);
    if (!error.Empty())
    {
        IERROR(std::string("Flare Task Shader generation error: ") + error.CStr());
    }

    const VulkanTaskGLSLShaderBuilder glslBuilder =
    {
        .Engine = a_builder.Engine,
        .String = shader,
        .Inputs = inputs.Data(),
        .InputCount = inputs.Size(),
        .EntryPoint = a_builder.EntryPoint
    };

    CreateFromGLSL(a_out, glslBuilder, a_allocator, a_tempAllocator);
}
void VulkanTaskShader::CreateFromGLSL(VulkanTaskShader* a_out, const VulkanTaskGLSLShaderBuilder& a_builder, Allocator* a_allocator, Allocator* a_tempAllocator)
{
    IVERIFY(a_out != nullptr);
    IVERIFY(a_builder.Engine != nullptr);
    IVERIFY(!a_builder.EntryPoint.Empty());
    IVERIFY(!a_builder.String.Empty());

    const Array<uint32_t> spirv = spirv_fromGLSL
    (
        EShLangTask,
        a_builder.String,
        true,
        a_builder.EntryPoint,
        a_tempAllocator
    );
    if (spirv.Empty())
    {
        IERROR("Failed to compile Task Shader");
    }

    new (a_out) VulkanTaskShader
    (
        a_builder.Engine,
        a_builder.Inputs,
        a_builder.InputCount,
        spirv.Data(),
        spirv.Size(),
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