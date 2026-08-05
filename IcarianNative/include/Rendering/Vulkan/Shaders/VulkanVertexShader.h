// Icarian Engine - C# Game Engine
//
// License at end of file.

#pragma once

#ifdef ICARIANNATIVE_ENABLE_GRAPHICS_VULKAN

#include "Rendering/Vulkan/Shaders/VulkanShader.h"

#include "Core/DataTypes/Array.h"
#include "Core/DataTypes/COWString.h"
#include "Core/DataTypes/Dictionary.h"

struct VulkanVertexFShaderBuilder
{
    VulkanRenderEngineBackend* Engine;
    IcarianCore::COWU8String String;
    IcarianCore::Dictionary<IcarianCore::COWU8String, IcarianCore::COWU8String> Imports;
    IcarianCore::COWU8String EntryPoint;
    // uint16_t StartSlot;
    IcarianCore::Array<ShaderBufferInput> OtherInputs;
};

struct VulkanVertexGLSLShaderBuilder
{
    VulkanRenderEngineBackend* Engine;
    IcarianCore::COWU8String String;
    ShaderBufferInput* Inputs;
    uint32_t InputCount;
    IcarianCore::COWU8String EntryPoint;
};

class VulkanVertexShader : public VulkanShader
{
private:

protected:

public:
    VulkanVertexShader() = delete;
    VulkanVertexShader
    (
        VulkanRenderEngineBackend* a_engine,
        const ShaderBufferInput* a_inputs,
        uint32_t a_inputCount,
        const uint32_t* a_data,
        uint32_t a_dataCount,
        IcarianCore::Allocator* a_allocator
    );
    virtual ~VulkanVertexShader();

    virtual e_VulkanShaderType GetShaderType() const
    {
        return VulkanShaderType_Vertex;
    }

    static void CreateFromFShader
    (
        VulkanVertexShader* a_out,
        const VulkanVertexFShaderBuilder& a_builder,
        IcarianCore::Allocator* a_allocator,
        IcarianCore::Allocator* a_tempAllocator
    );
    static void CreateFromGLSL
    (
        VulkanVertexShader* a_out,
        const VulkanVertexGLSLShaderBuilder& a_builder,
        IcarianCore::Allocator* a_allocator,
        IcarianCore::Allocator* a_tempAllocator
    );
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
