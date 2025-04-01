// Icarian Engine - C# Game Engine
// 
// License at end of file.

#pragma once

#ifdef ICARIANNATIVE_ENABLE_GRAPHICS_VULKAN

#include "Rendering/Vulkan/Shaders/VulkanShader.h"

struct VulkanComputeFShaderBuilder
{
    VulkanRenderEngineBackend* Engine;
    std::string String;
    std::unordered_map<std::string, std::string> Imports;
    std::string EntryPoint;
};

struct VulkanComputeGLSLShaderBuilder
{
    VulkanRenderEngineBackend* Engine;
    std::string String;
    uint32_t WorkgroupX;
    uint32_t WorkgroupY;
    uint32_t WorkgroupZ;
    ShaderBufferInput* Inputs;
    uint32_t InputCount;
    std::string EntryPoint;
};

class VulkanComputeShader : public VulkanShader
{
private:
    uint32_t m_workgroupX;
    uint32_t m_workgroupY;
    uint32_t m_workgroupZ;

protected:

public:
    VulkanComputeShader() = delete;
    VulkanComputeShader(VulkanRenderEngineBackend* a_engine, uint32_t a_workgroupX, uint32_t a_workgroupY, uint32_t a_workgroupZ, const ShaderBufferInput* a_inputs, uint32_t a_inputCount, const std::vector<uint32_t>& a_data, Allocator* a_allocator);
    ~VulkanComputeShader();

    inline uint32_t GetWorkgroupX() const
    {
        return m_workgroupX;
    }
    inline uint32_t GetWorkgroupY() const
    {
        return m_workgroupY;
    }
    inline uint32_t GetWorkgroupZ() const
    {
        return m_workgroupZ;
    }

    static void CreateFromFShader(VulkanComputeShader* a_out, const VulkanComputeFShaderBuilder& a_builder, Allocator* a_allocator);
    static void CreateFromGLSL(VulkanComputeShader* a_out, const VulkanComputeGLSLShaderBuilder& a_builder, Allocator* a_allocator);
};

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