// Icarian Engine - C# Game Engine
// 
// License at end of file.

#pragma once

#ifdef ICARIANNATIVE_ENABLE_GRAPHICS_VULKAN
#include "Rendering/Vulkan/IcarianVulkanHeader.h"

#include <vector>

class VulkanRenderEngineBackend;

#include "EngineMaterialInteropStructures.h"

class VulkanComputeLayout
{
private:
    VulkanRenderEngineBackend* m_engine;

    vk::PipelineLayout         m_layout;

    uint32_t                   m_inputCount;
    vk::DescriptorSetLayout*   m_descLayouts;
    ShaderBufferInput*         m_slotInputs;

protected:

public:
    VulkanComputeLayout(VulkanRenderEngineBackend* a_engine, const ShaderBufferInput* a_inputs, uint32_t a_inputCount);
    ~VulkanComputeLayout();

    inline uint32_t GetInputCount() const
    {
        return m_inputCount;
    }

    inline const vk::DescriptorSetLayout* GetDescriptorLayouts() const
    {
        return m_descLayouts;
    }   
    inline const ShaderBufferInput* GetShaderInputs() const
    {
        return m_slotInputs;
    }

    inline vk::PipelineLayout GetLayout() const
    {
        return m_layout;
    }
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