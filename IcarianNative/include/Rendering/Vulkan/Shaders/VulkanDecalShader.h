// Icarian Engine - C# Game Engine
// 
// License at end of file.

#pragma once

#ifdef ICARIANNATIVE_ENABLE_GRAPHICS_VULKAN

#include <string>
#include <unordered_map>

class Allocator;

class VulkanPixelShader;
class VulkanRenderEngineBackend;
class VulkanShaderData;
class VulkanVertexShader;

struct VulkanDecalShaderBuilder
{
    VulkanRenderEngineBackend* Engine;
    std::string String;
    std::unordered_map<std::string, std::string> Imports;
};

// NOTE: This is not a normal shader type this is a custom shader
class VulkanDecalShader
{
private:
    Allocator*                 m_allocator;

    VulkanVertexShader*        m_vertexShader;
    VulkanPixelShader*         m_pixelShader;

    VulkanDecalShader(VulkanVertexShader* a_vertexShader, VulkanPixelShader* a_pixelShader, Allocator* a_allocator);

protected:

public:
    VulkanDecalShader() = delete;
    ~VulkanDecalShader();

    inline VulkanVertexShader* GetVertexShader() const
    {
        return m_vertexShader;
    }
    inline VulkanPixelShader* GetPixelShader() const
    {
        return m_pixelShader;
    }

    static void CreateFromFShader(VulkanDecalShader* a_out, const VulkanDecalShaderBuilder& a_builder, Allocator* a_allocator); 
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