// Icarian Engine - C# Game Engine
// 
// License at end of file.

#ifdef ICARIANNATIVE_ENABLE_GRAPHICS_VULKAN

#include "Rendering/Vulkan/Shaders/VulkanMeshShader.h"

#include "Rendering/FlareShader.h"
#include "Rendering/SPIRVTools.h"
#include "Rendering/Vulkan/Shaders/VulkanComputeShader.h"
#include "Rendering/Vulkan/Shaders/VulkanVertexShader.h"
#include "Rendering/Vulkan/VulkanGraphicsEngine.h"
#include "Rendering/Vulkan/VulkanRenderEngineBackend.h"

VulkanMeshShader::VulkanMeshShader
(
    VulkanRenderEngineBackend* a_engine,
    const ShaderBufferInput* a_inputs,
    uint32_t a_inputCount,
    const uint32_t* a_data,
    uint32_t a_dataCount,
    Allocator* a_allocator
) : VulkanShader(a_engine, a_inputs, a_inputCount, a_allocator)
{
    TRACE("Creating Mesh Shader");
    const vk::Device device = m_engine->GetLogicalDevice();

    const vk::ShaderModuleCreateInfo createInfo = vk::ShaderModuleCreateInfo
    (
        { },
        a_dataCount * sizeof(uint32_t),
        a_data
    );

    VKRESERRMSG(device.createShaderModule(&createInfo, nullptr, &m_module), "Failed to create MeshShader");
}
VulkanMeshShader::~VulkanMeshShader()
{

}

void VulkanMeshShader::CreateFromFShader(VulkanMeshShader* a_out, const VulkanMeshFShaderBuilder& a_builder, Allocator* a_allocator, Allocator* a_tempAllocator)
{
    IVERIFY(a_out != nullptr);
    IVERIFY(a_builder.Engine != nullptr);
    IVERIFY(!a_builder.String.Empty());

    IVERIFY(a_builder.Engine->IsMeshEnabled());

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
    const COWU8String error = FlareShader::GLSLFromFlareShader(&shader, builder, a_allocator, a_tempAllocator);
    if (!error.Empty())
    {
        IERROR(std::string("Flare Mesh Shader generation error: ") + error.CStr());
    }

    const Array<uint32_t> spirv = spirv_fromGLSL(EShLangMesh, shader, true, a_builder.EntryPoint, a_tempAllocator);
    if (spirv.Empty())
    {
        IERROR("Failed to compile Mesh shader");
    }

    new (a_out) VulkanMeshShader
    (
        a_builder.Engine,
        inputs.Data(),
        inputs.Size(),
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
