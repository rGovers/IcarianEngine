// Icarian Engine - C# Game Engine
// 
// License at end of file.

#pragma once

#ifdef ICARIANNATIVE_ENABLE_GRAPHICS_VULKAN

#include "Rendering/Vulkan/IcarianVulkanHeader.h"

#include <cstdint>

class VideoClip;
class VulkanRenderEngineBackend;
struct VulkanVideoDecodeCapabilities;

struct VulkanVideoBuffer
{
    double EndTimeStamp;
    VmaAllocation Allocation;
    vk::Buffer Buffer;
};

class VulkanVideoTexture
{
private:
    static constexpr uint32_t VideoBufferCount = 3;

    VulkanRenderEngineBackend*    m_engine;

    uint32_t                      m_currentVideoBuffer;
    VulkanVideoBuffer             m_buffers[VideoBufferCount];

    vk::VideoSessionKHR           m_videoSession;
    vk::VideoSessionParametersKHR m_sessionParameters;

    uint32_t                      m_videoAddr;

    double                        m_duration;
    double                        m_time;
    float                         m_fps;

    double GenerateClipBuffer(const VideoClip* a_clip, const VulkanVideoDecodeCapabilities* a_videoCapabilities, double a_timeStamp, uint32_t a_index);

protected:

public:
    VulkanVideoTexture(VulkanRenderEngineBackend* a_engine, uint32_t a_videoAddr);
    ~VulkanVideoTexture();

    void UpdateVulkan(vk::CommandBuffer a_commandBuffer, double a_delta);
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