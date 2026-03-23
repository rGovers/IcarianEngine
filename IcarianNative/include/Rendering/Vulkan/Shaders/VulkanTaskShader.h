// Icarian Engine - C# Game Engine
// 
// License at end of file.

#pragma once

#ifdef ICARIANNATIVE_ENABLE_GRAPHICS_VULKAN

#include "Rendering/Vulkan/Shaders/VulkanShader.h"

#include "DataTypes/Array.h"
#include "DataTypes/COWString.h"
#include "DataTypes/Dictionary.h"

struct VulkanTaskFShaderBuilder
{
    VulkanRenderEngineBackend* Engine;
    COWU8String String;
    Dictionary<COWU8String, COWU8String> Imports;
    COWU8String EntryPoint;
    Array<ShaderBufferInput> OtherInputs;
};

struct VulkanTaskGLSLShaderBuilder
{
    VulkanRenderEngineBackend* Engine;
    COWU8String String;
    ShaderBufferInput* Inputs;
    uint32_t InputCount;
    COWU8String EntryPoint;
};

class VulkanTaskShader : public VulkanShader
{
private:

protected:

public:
    VulkanTaskShader() = delete;
    VulkanTaskShader
    (
        VulkanRenderEngineBackend* a_engine,
        const ShaderBufferInput* a_inputs,
        uint32_t a_inputCount,
        const uint32_t* a_data,
        uint32_t a_dataCount,
        Allocator* a_allocator
    );
    virtual ~VulkanTaskShader();

        virtual e_VulkanShaderType GetShaderType() const
    {
        return VulkanShaderType_Task;
    }

    static void CreateFromFShader(VulkanTaskShader* a_out, const VulkanTaskFShaderBuilder& a_builder, Allocator* a_allocator, Allocator* a_tempAllocator);
    static void CreateFromGLSL(VulkanTaskShader* a_out, const VulkanTaskGLSLShaderBuilder& a_builder, Allocator* a_allocator, Allocator* a_tempAllocator);
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