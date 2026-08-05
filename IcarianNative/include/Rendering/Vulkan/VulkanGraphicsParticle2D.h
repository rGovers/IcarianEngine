// Icarian Engine - C# Game Engine
//
// License at end of file.

#pragma once

#ifdef ICARIANNATIVE_ENABLE_GRAPHICS_VULKAN

#include "Rendering/Vulkan/IcarianVulkanHeader.h"

#include <cstdint>

class VulkanComputeEngine;
class VulkanGraphicsEngine;
class VulkanRenderEngineBackend;

#include "Core/DataTypes/Array.h"
#include "Core/DataTypes/SpinLock.h"

#include "EngineMaterialInteropStructures.h"
#include "EngineParticleSystemInteropStructures.h"

class VulkanGraphicsParticle2D
{
private:
    IcarianCore::Allocator*               m_allocator;

    VulkanRenderEngineBackend*            m_backend;
    VulkanComputeEngine*                  m_cEngine;
    VulkanGraphicsEngine*                 m_gEngine;

    IcarianCore::Array<ShaderBufferInput> m_inputs;

    uint32_t                              m_computeBufferAddr;
    uint32_t                              m_renderProgramAddr;

    IcarianCore::SpinLock                 m_lock;

    void Build(const ComputeParticleBuffer& a_buffer, IcarianCore::Allocator* a_tempAllocator);
    void Destroy();

protected:

public:
    VulkanGraphicsParticle2D
    (
        VulkanRenderEngineBackend* a_backend,
        VulkanComputeEngine* a_cEngine,
        VulkanGraphicsEngine* a_gEngine,
        uint32_t a_computeBufferAddr,
        IcarianCore::Allocator* a_allocator
    );
    ~VulkanGraphicsParticle2D();

    void Update
    (
        uint32_t a_index,
        uint32_t a_bufferIndex,
        uint32_t a_renderLayer,
        vk::CommandBuffer a_commandBuffer,
        uint32_t a_renderTextureAddr,
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
