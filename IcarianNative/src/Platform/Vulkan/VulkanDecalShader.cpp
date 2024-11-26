// Icarian Engine - C# Game Engine
// 
// License at end of file.

#ifdef ICARIANNATIVE_ENABLE_GRAPHICS_VULKAN

#include "Rendering/Vulkan/Shaders/VulkanDecalShader.h"

#include <vector>

#include "Core/FlareShader.h"
#include "DataTypes/Allocator.h"
#include "IcarianError.h"
#include "Rendering/Vulkan/Shaders/VulkanPixelShader.h"
#include "Rendering/Vulkan/Shaders/VulkanVertexShader.h"
#include "Rendering/Vulkan/VulkanShaderData.h"

#include "EngineMaterialInteropStructures.h"

VulkanDecalShader::VulkanDecalShader(VulkanVertexShader* a_vertexShader, VulkanPixelShader* a_pixelShader, Allocator* a_allocator)
{
    m_allocator = a_allocator;

    m_vertexShader = a_vertexShader;
    m_pixelShader = a_pixelShader;
}
VulkanDecalShader::~VulkanDecalShader()
{
    m_allocator->Destroy(m_vertexShader);
    m_allocator->Destroy(m_pixelShader);
}

void VulkanDecalShader::CreateFromFShader(VulkanDecalShader* a_out, const VulkanDecalShaderBuilder& a_builder, Allocator* a_allocator)
{
    std::vector<ShaderBufferInput> inputs;
    std::string error;

    const std::string pixelStr = IcarianCore::DecalShaderFromFlareShader(a_builder.String, IcarianCore::ShaderPlatform_Vulkan, a_builder.Imports, &inputs, &error);

    const VulkanPixelFShaderBuilder pixelBuilder =
    {
        .Engine = a_builder.Engine,
        .String = pixelStr,
        .EntryPoint = "__fMain",
    };

    VulkanPixelShader* pixelShader = a_allocator->TAllocate<VulkanPixelShader>();
    VulkanPixelShader::CreateFromFShader(pixelShader, pixelBuilder, a_allocator);

    uint32_t camSlot = -1;
    uint32_t modelSlot = -1;
    for (const ShaderBufferInput& i : inputs)
    {
        switch (i.BufferType) 
        {
        case ShaderBufferType_CameraBuffer:
        {
            camSlot = i.Slot;

            break;
        }
        case ShaderBufferType_SSModelBuffer:
        {
            modelSlot = i.Slot;

            break;
        }
        default:
        {
            break;
        }
        }
    }

    IVERIFY(camSlot != -1);
    IVERIFY(modelSlot != -1);

    inputs.clear();
    const std::string vertexStr = IcarianCore::CreateDecalVertexShader(&inputs, IcarianCore::ShaderPlatform_Vulkan, camSlot, modelSlot, &error);

    const VulkanVertexFShaderBuilder vertexBuilder =
    {
        .Engine = a_builder.Engine,
        .String = vertexStr,
        .EntryPoint = "main"
    };

    VulkanVertexShader* vertexShader = a_allocator->TAllocate<VulkanVertexShader>();
    VulkanVertexShader::CreateFromFShader(vertexShader, vertexBuilder, a_allocator);

    new (a_out) VulkanDecalShader(vertexShader, pixelShader, a_allocator);
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