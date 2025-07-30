// Icarian Engine - C# Game Engine
//
// License at end of file.

#pragma once

#ifdef ICARIANNATIVE_ENABLE_GRAPHICS_VULKAN

#include "Rendering/Vulkan/IcarianVulkanHeader.h"

#include "Core/ShaderBuffers.h"

class VulkanRenderEngineBackend;

class VulkanMesh
{
private:
    VulkanRenderEngineBackend* m_engine;

    VmaAllocation              m_allocation;
    vk::Buffer                 m_buffer;

    vk::DeviceSize             m_meshletVertexOffset;
    vk::DeviceSize             m_meshletTriangleOffset;
    vk::DeviceSize             m_meshletOffset;
    uint32_t                   m_meshletCount;

    float                      m_radius;

protected:

public:
    // TODO: Need to figure out if I want to have the meshlets index into the index buffer or have the meshlets contain the index buffer
    VulkanMesh
    (
        VulkanRenderEngineBackend* a_engine,
        const void* a_vertices,
        uint32_t a_vertexCount,
        uint16_t a_vertexSize,
        const uint32_t* a_meshVertices,
        uint32_t a_meshVertexCount,
        const uint8_t* a_meshTriangles,
        uint32_t a_meshTriangleCount,
        const IcarianCore::ShaderMeshletBuffer* a_meshlets,
        uint32_t a_meshletCount,
        float a_radius
    );
    ~VulkanMesh();

    inline uint32_t GetMeshletCount() const
    {
        return m_meshletCount;
    }
    inline float GetRadius() const
    {
        return m_radius;
    }

    inline vk::Buffer GetBuffer() const
    {
        return m_buffer;
    }

    inline vk::DeviceSize GetVertexOffset() const
    {
        return 0;
    }
    inline vk::DeviceSize GetMeshletVertexOffset() const
    {
        return m_meshletVertexOffset;
    }
    inline vk::DeviceSize GetMeshletTriangleOffset() const
    {
        return m_meshletTriangleOffset;
    }
    inline vk::DeviceSize GetMeshletOffset() const
    {
        return m_meshletOffset;
    }
};

#endif

// MIT License
//
// Copyright (c) 2025 River Govers
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
