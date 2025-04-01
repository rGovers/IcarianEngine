// Icarian Engine - C# Game Engine
// 
// License at end of file.

#pragma once

#ifdef ICARIANNATIVE_ENABLE_GRAPHICS_VULKAN

#include "Rendering/Vulkan/IcarianVulkanHeader.h"

#include <cstdint>

class VideoClip;
class VideoInfo;
class VulkanRenderEngineBackend;
class VulkanTexture;
struct VulkanVideoDecodeCapabilities;

struct VulkanHarwareVideoData
{
    static constexpr uint32_t VideoBufferCount = 8;
    static constexpr uint32_t DBPFrames = 16;
    static constexpr uint32_t TotalDBPFrames = DBPFrames + 1;
    
    uint32_t                        DPBSlots;
    uint32_t                        MaxBuffers;
    uint32_t                        BufferSize;
    
    VmaAllocation                   Allocations[VideoBufferCount];

    vk::Buffer                      StreamBuffer;
    VmaAllocation                   StreamAllocation;

    vk::VideoSessionKHR             VideoSession;
    vk::VideoSessionParametersKHR   SessionParameters;
  
    VulkanTexture*                  VideoTexture;
};

// Between Vulkan and H264 there are far too many varibles to keep track of FUCK THIS
// I will be back when this breaks not before I want off this ride
class VulkanVideoTexture
{
private:
    VulkanRenderEngineBackend*      m_engine;
  
    // Yes there is a pointer to class data but only use it when using hardware decoding and is over a kb of data
    // This way when not using hardware decoding the class fits in a cache line
    VulkanHarwareVideoData*         m_vulkanVideoData;

    uint32_t                        m_outTextureAddr;
    uint32_t                        m_lastIntra;
    uint32_t                        m_lastFrame;
    
    uint32_t                        m_videoAddr;

    void LoadHardwarePlayback(const VideoInfo* a_info);

protected:

public:
    VulkanVideoTexture(VulkanRenderEngineBackend* a_engine, uint32_t a_videoAddr);
    ~VulkanVideoTexture();

    inline bool IsHardware() const
    {
        return m_vulkanVideoData != nullptr;
    }

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