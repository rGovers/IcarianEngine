// Icarian Engine - C# Game Engine
// 
// License at end of file.

#ifdef ICARIANNATIVE_ENABLE_GRAPHICS_VULKAN

#include "Rendering/Vulkan/Shaders/VulkanMeshShader.h"

#include "Core/FlareShader.h"
#include "Rendering/SPIRVTools.h"
#include "Rendering/Vulkan/Shaders/VulkanComputeShader.h"
#include "Rendering/Vulkan/Shaders/VulkanVertexShader.h"
#include "Rendering/Vulkan/VulkanGraphicsEngine.h"
#include "Rendering/Vulkan/VulkanRenderEngineBackend.h"

VulkanMeshShader::VulkanMeshShader(VulkanRenderEngineBackend* a_engine, VulkanGraphicsEngine* a_gEngine, VulkanVertexShader* a_vertexShader, VulkanComputeShader* a_computeShader, Allocator* a_allocator)
{
    // Using IERROR for now as this is an exotic new system can pull back to using IVERIFY for DEBUG only checks later
    const bool isMeshEnabled = a_engine->IsMeshEnabled();
    if (isMeshEnabled)
    {
        IERROR("Creating Emulated MeshShader in Native mode");
    }

    m_engine = a_engine;
    m_gEngine = a_gEngine;
    m_allocator = a_allocator;
}
VulkanMeshShader::VulkanMeshShader
(
    VulkanRenderEngineBackend* a_engine,
    VulkanGraphicsEngine* a_gEngine,
    const ShaderBufferInput* a_inputs,
    uint32_t a_inputCount,
    vk::ShaderModule a_module,
    Allocator* a_allocator
)
{
    const bool isMeshEnabled = a_engine->IsMeshEnabled();
    if (!isMeshEnabled)
    {
        IERROR("Creating Native MeshShader in Emulated mode");
    }

    m_engine = a_engine;
    m_gEngine = a_gEngine;
    m_allocator = a_allocator;

    m_data.Native.InputCount = a_inputCount;
    m_data.Native.Inputs = m_allocator->TAllocate<ShaderBufferInput>(a_inputCount);
    for (uint32_t i = 0; i < a_inputCount; ++i)
    {
        m_data.Native.Inputs[i] = a_inputs[i];
    }

    m_data.Native.Module = a_module;
}
VulkanMeshShader::~VulkanMeshShader()
{
    const bool isMeshEnabled = m_engine->IsMeshEnabled();
    if (isMeshEnabled)
    {
        const vk::Device device = m_engine->GetLogicalDevice();

        device.destroyShaderModule(m_data.Native.Module);

        m_allocator->Free(m_data.Native.Inputs);
    }
    else
    {
        m_allocator->Destroy(m_data.Emulated.VertexShader);
        m_allocator->Destroy(m_data.Emulated.ComputeShader);
    }
}

uint32_t VulkanMeshShader::GetShaderInputCount() const
{
    const bool isMeshEnabled = m_engine->IsMeshEnabled();
    if (isMeshEnabled)
    {
        return m_data.Native.InputCount;
    }

    IVERIFY(m_data.Emulated.ComputeShader != nullptr);
    const VulkanComputeShader* computeShader = m_data.Emulated.ComputeShader;

    return computeShader->GetShaderInputCount();
}
ShaderBufferInput VulkanMeshShader::GetShaderInput(uint32_t a_index) const
{
    const bool isMeshEnabled = m_engine->IsMeshEnabled();
    if (isMeshEnabled)
    {
        return m_data.Native.Inputs[a_index];
    }

    IVERIFY(m_data.Emulated.ComputeShader != nullptr);
    const VulkanComputeShader* computeShader = m_data.Emulated.ComputeShader;

    return computeShader->GetShaderInput(a_index);
}

vk::ShaderModule VulkanMeshShader::GetShaderModule() const
{
    const bool isMeshEnabled = m_engine->IsMeshEnabled();
    if (!isMeshEnabled)
    {
        IERROR("Calling GetShaderModule in Emulated mode");
    }

    return m_data.Native.Module;
}

VulkanVertexShader* VulkanMeshShader::GetVertexShader() const
{
    const bool isMeshEnabled = m_engine->IsMeshEnabled();
    if (isMeshEnabled)
    {
        IERROR("Calling GetVertexShader in Native mode");
    }

    return m_data.Emulated.VertexShader;
}
VulkanComputeShader* VulkanMeshShader::GetComputeShader() const
{
    const bool isMeshEnabled = m_engine->IsMeshEnabled();
    if (isMeshEnabled)
    {
        IERROR("Calling GetComputeShader in Native mode");
    }

    return m_data.Emulated.ComputeShader;
}

void VulkanMeshShader::CreateFromFShader(VulkanMeshShader* a_out, const VulkanMeshFShaderBuilder& a_builder, Allocator* a_allocator)
{
    IVERIFY(a_out != nullptr);
    IVERIFY(a_builder.Engine != nullptr);
    IVERIFY(!a_builder.EntryPoint.empty());
    IVERIFY(!a_builder.String.empty());

    const bool isMeshEnabled = a_builder.Engine->IsMeshEnabled();
    if (isMeshEnabled)
    {
        std::string error;
        std::vector<ShaderBufferInput> inputs;
        const std::string str = IcarianCore::GLSLFromFlareShader(a_builder.String, IcarianCore::ShaderPlatform_Vulkan, a_builder.Imports, &inputs, &error);

        if (str.empty())
        {
            IERROR("Flare Mesh shader error: " + error);
        }

        const std::vector<uint32_t> spirv = spirv_fromGLSL(EShLangMesh, str, true, a_builder.EntryPoint);
        if (spirv.empty())
        {
            IERROR("Failed to compile Mesh shader");
        }    

        const vk::Device device = a_builder.Engine->GetLogicalDevice();

        const vk::ShaderModuleCreateInfo createInfo = vk::ShaderModuleCreateInfo
        (
            { },
            spirv.size() * sizeof(uint32_t),
            (uint32_t*)spirv.data()
        );

        vk::ShaderModule module;
        VKRESERRMSG(device.createShaderModule(&createInfo, nullptr, &module), "Failed to create MeshShader");
    
        new (a_out) VulkanMeshShader(a_builder.Engine, a_builder.GraphicsEngine, inputs.data(), (uint32_t)inputs.size(), module, a_allocator);
    }
    else 
    {
        std::string error;
        std::vector<ShaderBufferInput> inputs;
        IcarianCore::ShaderOutput output;
        const std::string computeStr = IcarianCore::GLSLFromFlareShader(a_builder.String, IcarianCore::ShaderPlatform_VulkanCompute, a_builder.Imports, &inputs, &error, &output);

        const VulkanComputeGLSLShaderBuilder computeBuilder =
        {
            .Engine = a_builder.Engine,
            .String = computeStr,
            .WorkgroupX = output.Workgroups.GroupX,
            .WorkgroupY = output.Workgroups.GroupY,
            .WorkgroupZ = output.Workgroups.GroupZ,
            .Inputs = inputs.data(),
            .InputCount = (uint32_t)inputs.size(),
            // TODO: This does not seem right and could be bug prone need to revist entry point selection
            .EntryPoint = a_builder.EntryPoint,
        };

        VulkanComputeShader* computeShader = a_allocator->TAllocate<VulkanComputeShader>();
        VulkanComputeShader::CreateFromGLSL(computeShader, computeBuilder, a_allocator);

        const std::string vertexStr = IcarianCore::GenerateMeshVertexStub(output.MeshOutputs);

        const VulkanVertexGLSLShaderBuilder vertexBuilder =
        {
            .Engine = a_builder.Engine,
            .String = vertexStr,
            .EntryPoint = "main",
        };

        VulkanVertexShader* vertexShader = a_allocator->TAllocate<VulkanVertexShader>();
        VulkanVertexShader::CreateFromGLSL(vertexShader, vertexBuilder, a_allocator);

        new (a_out) VulkanMeshShader(a_builder.Engine, a_builder.GraphicsEngine, vertexShader, computeShader, a_allocator);
    }
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
