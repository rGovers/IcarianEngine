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