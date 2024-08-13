// Icarian Engine - C# Game Engine
// 
// License at end of file.

#pragma once

#ifdef ICARIANNATIVE_ENABLE_GRAPHICS_VULKAN

#include "Rendering/Vulkan/IcarianVulkanHeader.h"

#include "EngineTextureSamplerInteropStructures.h"

class VulkanGraphicsEngine;
class VulkanRenderEngineBackend;

class VulkanTextureSampler
{
private:
    VulkanRenderEngineBackend* m_engine;

    vk::Sampler                m_sampler;

    VulkanTextureSampler(VulkanRenderEngineBackend* a_engine);

protected:

public:
    ~VulkanTextureSampler();
    
    static VulkanTextureSampler* GenerateFromBuffer(VulkanRenderEngineBackend* a_engine, VulkanGraphicsEngine* a_gEngine, const TextureSamplerBuffer& a_sampler);

    inline vk::Sampler GetSampler() const
    {
        return m_sampler;
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