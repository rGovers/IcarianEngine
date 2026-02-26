// Icarian Engine - C# Game Engine
// 
// License at end of file.

#pragma once

#ifdef ICARIANNATIVE_ENABLE_GRAPHICS_VULKAN

#include "Rendering/Vulkan/Shaders/VulkanShader.h"

#include "DataTypes/Array.h"
#include "DataTypes/COWString.h"

#include "EngineMaterialInteropStructures.h"

struct VulkanMeshFShaderBuilder
{
    VulkanRenderEngineBackend* Engine;
    COWU8String String;
    std::unordered_map<std::string, std::string> Imports;
    COWU8String EntryPoint;
    Array<ShaderBufferInput> OtherInputs;
};

class VulkanMeshShader : public VulkanShader
{
private:

protected:

public:
    VulkanMeshShader() = delete;
    VulkanMeshShader
    (
        VulkanRenderEngineBackend* a_engine,
        const ShaderBufferInput* a_inputs,
        uint32_t a_inputCount,
        const uint32_t* a_data,
        uint32_t a_dataCount,
        Allocator* a_allocator
    );
    virtual ~VulkanMeshShader();

    virtual e_VulkanShaderType GetShaderType() const
    {
        return VulkanShaderType_Mesh;
    }

    uint32_t GetVertexInputAttributeCount() const;
    VertexInputAttribute GetVertexInputAttribute(uint32_t a_index) const;

    static void CreateFromFShader(VulkanMeshShader* a_out, const VulkanMeshFShaderBuilder& a_builder, Allocator* a_allocator, Allocator* a_tempAllocator);
};

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
