// Icarian Engine - C# Game Engine
// 
// License at end of file.

#ifdef ICARIANNATIVE_ENABLE_GRAPHICS_VULKAN

#include "Rendering/Vulkan/Shaders/VulkanShader.h"

#include "DataTypes/Allocator.h"

VulkanShader::VulkanShader(VulkanRenderEngineBackend* a_engine, const ShaderBufferInput* a_inputs, uint32_t a_inputCount, Allocator* a_allocator)
{
    m_module = nullptr;
    m_engine = a_engine;
    m_inputCount = a_inputCount;
    m_allocator = a_allocator;

    // We can now pass this is null sometimes as Shaders can now contain Shaders
    m_inputs = nullptr;
    if (a_inputCount > 0 && a_inputs != nullptr)
    {
        m_inputs = m_allocator->TAllocate<ShaderBufferInput>(m_inputCount);
        for (uint32_t i = 0; i < m_inputCount; ++i)
        {
            m_inputs[i] = a_inputs[i];
        }
    }
}
VulkanShader::~VulkanShader()
{
    if (m_inputs != nullptr)
    {
        m_allocator->Free(m_inputs);
        // Virtual so null for safety
        m_inputs = nullptr;
    }
}

ShaderBufferInput VulkanShader::GetShaderInput(uint32_t a_index) const
{
    IVERIFY(a_index < m_inputCount);

    return m_inputs[a_index];
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
