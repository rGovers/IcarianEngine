#include "Core/Bitfield.h"
#ifdef ICARIANNATIVE_ENABLE_GRAPHICS_VULKAN

#include "Rendering/Vulkan/VulkanVideoTexture.h"

#include "Rendering/Video/VideoClip.h"
#include "Rendering/Video/VideoManager.h"
#include "Rendering/Vulkan/VulkanRenderEngineBackend.h"

class VideoBufferVulkanDeletionObject : public VulkanDeletionObject
{
private:
    VulkanRenderEngineBackend* m_engine;

    VulkanVideoBuffer          m_buffer;

protected:

public:
    VideoBufferVulkanDeletionObject(VulkanRenderEngineBackend* a_engine, const VulkanVideoBuffer& a_buffer)
    {
        m_engine = a_engine;

        m_buffer = a_buffer;
    }
    ~VideoBufferVulkanDeletionObject()
    {

    }

    virtual void Destroy()
    {
        const VmaAllocator allocator = m_engine->GetAllocator();

        vmaDestroyBuffer(allocator, m_buffer.Buffer, m_buffer.Allocation);
    }
};

static void CreateBuffer(const VmaAllocator& a_allocator, const VulkanVideoDecodeCapabilities* a_videoCapabilities, const uint8_t* a_data, uint32_t a_size, VmaAllocation* a_allocation, vk::Buffer* a_buffer)
{
    constexpr VmaAllocationCreateInfo BufferAllocInfo = 
    { 
        .flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT,
        .usage = VMA_MEMORY_USAGE_AUTO,
        .requiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
    };

    const vk::VideoProfileListInfoKHR profileListInfo = vk::VideoProfileListInfoKHR
    (
        1,
        &a_videoCapabilities->VideoProfile
    );

    const VkBufferCreateInfo bufferInfo = 
    {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .pNext = &profileListInfo,
        .size = a_size,
        .usage = VK_BUFFER_USAGE_VIDEO_DECODE_SRC_BIT_KHR,
    };

    VkBuffer buffer;
    VmaAllocationInfo info;
    VKRESERRMSG(vmaCreateBuffer(a_allocator, &bufferInfo, &BufferAllocInfo, &buffer, a_allocation, &info), "Failed to create Decode Video Buffer");
    *a_buffer = buffer;

#ifdef DEBUG
    vmaSetAllocationName(a_allocator, *a_allocation, "Video Decode Buffer");
#endif

    memcpy(info.pMappedData, a_data, (size_t)a_size);
    VKRESERR(vmaFlushAllocation(a_allocator, *a_allocation, 0, (VkDeviceSize)a_size));
}

double VulkanVideoTexture::GenerateClipBuffer(const VideoClip* a_clip, const VulkanVideoDecodeCapabilities* a_videoCapabilities, double a_timeStamp, uint32_t a_index)
{
    const VmaAllocator allocator = m_engine->GetAllocator();

    m_buffers[a_index] = { 0 };

    uint8_t* dat;
    uint32_t size;
    uint32_t endIndex;
    if (!a_clip->GetVideoClipData(a_timeStamp, nullptr, &endIndex, &dat, &size))
    {
        return -1.0;
    }
    IDEFER(delete[] dat);

    const VideoFrameInfo* frameInfo = a_clip->GetFrameInfo();

    const double endTimeStamp = frameInfo[endIndex].TimeStamp;

    VmaAllocation allocation;
    vk::Buffer buffer;
    CreateBuffer(allocator, a_videoCapabilities, dat, size, &allocation, &buffer);

    const VulkanVideoBuffer vidBuf = 
    {
        .EndTimeStamp = endTimeStamp,
        .Allocation = allocation,
        .Buffer = buffer
    };

    m_buffers[a_index] = vidBuf;

    return endTimeStamp;
}

constexpr StdVideoH264LevelIdc ConvertLevelIDCVulkan(uint32_t a_levelIDC)
{
    switch (a_levelIDC) 
    {
    case 0:
    {
        return STD_VIDEO_H264_LEVEL_IDC_1_0;
    }
    case 11:
    {
        return STD_VIDEO_H264_LEVEL_IDC_1_1;
    }
    case 12:
    {
        return STD_VIDEO_H264_LEVEL_IDC_1_2;
    }
    case 13:
    {
        return STD_VIDEO_H264_LEVEL_IDC_1_3;
    }
    case 20:
    {
        return STD_VIDEO_H264_LEVEL_IDC_2_0;
    }
    case 21:
    {
        return STD_VIDEO_H264_LEVEL_IDC_2_1;
    }
    case 22:
    {
        return STD_VIDEO_H264_LEVEL_IDC_2_2;
    }
    case 30:
    {
        return STD_VIDEO_H264_LEVEL_IDC_3_0;
    }
    case 31:
    {
        return STD_VIDEO_H264_LEVEL_IDC_3_1;
    }
    case 32:
    {
        return STD_VIDEO_H264_LEVEL_IDC_3_2;
    }
    case 40:
    {
        return STD_VIDEO_H264_LEVEL_IDC_4_0;
    }
    case 41:
    {
        return STD_VIDEO_H264_LEVEL_IDC_4_1;
    }
    case 42:
    {
        return STD_VIDEO_H264_LEVEL_IDC_4_2;
    }
    case 50:
    {
        return STD_VIDEO_H264_LEVEL_IDC_5_0;
    }
    case 51:
    {   
        return STD_VIDEO_H264_LEVEL_IDC_5_1;
    }
    case 52:
    {
        return STD_VIDEO_H264_LEVEL_IDC_5_2;
    }
    case 60:
    {
        return STD_VIDEO_H264_LEVEL_IDC_6_0;
    }
    case 61:
    {
        return STD_VIDEO_H264_LEVEL_IDC_6_1;
    }
    case 62:
    {
        return STD_VIDEO_H264_LEVEL_IDC_6_2;
    }
    default:
    {
        IERROR("Invalid level IDC");
    }
    }

    return STD_VIDEO_H264_LEVEL_IDC_INVALID;
}

VulkanVideoTexture::VulkanVideoTexture(VulkanRenderEngineBackend* a_engine, uint32_t a_videoAddr)
{
    m_engine = a_engine;

    m_videoAddr = a_videoAddr;

    m_currentVideoBuffer = 0;
    m_time = 0.0;

    if (!m_engine->IsExtensionEnabled(VK_KHR_VIDEO_DECODE_H264_EXTENSION_NAME))
    {
        IERROR("Software decode not implemented yet");
    }

    const VideoClip* clip = VideoManager::GetVideoClip(m_videoAddr);
    IVERIFY(clip != nullptr);

    m_fps = clip->GetFPS();
    m_duration = clip->GetDuration();
    
    const float frameStep = 1.0f / m_fps;

    const VulkanVideoDecodeCapabilities* videoCapabilities = m_engine->GetVideoDecodeCapabilities();

    double lastTimeStamp = 0;
    for (uint32_t i = 0; i < VideoBufferCount; ++i)
    {
        lastTimeStamp = GenerateClipBuffer(clip, videoCapabilities, lastTimeStamp, i);

        if (lastTimeStamp == -1)
        {
            break;
        }

        lastTimeStamp += frameStep;
    }

    const vk::Device device = m_engine->GetLogicalDevice();

    const uint32_t spsCount = clip->GetSPSCount();
    const H264::SPS* spsData = clip->GetSPSData();
    uint32_t refFrames = 0;

    StdVideoH264SequenceParameterSet* vulkanSPS = new StdVideoH264SequenceParameterSet[spsCount];
    IDEFER(delete[] vulkanSPS);

    for (uint32_t i = 0; i < spsCount; ++i)
    {
        const H264::SPS& sps = spsData[i];

        refFrames = glm::max(refFrames, (uint32_t)sps.NumRefFrames);

        const StdVideoH264SpsFlags vSPSFlags = 
        {
            .constraint_set0_flag = IISBITSET(sps.ConstraintSetFlag, 0),
            .constraint_set1_flag = IISBITSET(sps.ConstraintSetFlag, 1),
            .constraint_set2_flag = IISBITSET(sps.ConstraintSetFlag, 2),
            .constraint_set3_flag = IISBITSET(sps.ConstraintSetFlag, 3),
            .constraint_set4_flag = IISBITSET(sps.ConstraintSetFlag, 4),
            .constraint_set5_flag = IISBITSET(sps.ConstraintSetFlag, 5),
            .direct_8x8_inference_flag = (sps.Flags & H264::SPSFlags_Direct8x8Inference) != 0,
            .mb_adaptive_frame_field_flag = (sps.Flags & H264::SPSFlags_MBAdaptiveFrameField) != 0,
            .frame_mbs_only_flag = (sps.Flags & H264::SPSFlags_FrameMBSOnly) != 0,
            .delta_pic_order_always_zero_flag = (sps.Flags & H264::SPSFlags_DeltaPICOrderAlwaysZero) != 0,
            .separate_colour_plane_flag = (sps.Flags & H264::SPSFlags_SeparateColourPlane) != 0,
            .gaps_in_frame_num_value_allowed_flag = (sps.Flags & H264::SPSFlags_GapsInFrameNumValueAllowed) != 0,
            .qpprime_y_zero_transform_bypass_flag = (sps.Flags & H264::SPSFlags_QPPrimeYZeroTransformBypass) != 0,
            .frame_cropping_flag = (sps.Flags & H264::SPSFlags_FrameCropping) != 0,
            .seq_scaling_matrix_present_flag = (sps.Flags & H264::SPSFlags_SeqScalingMatrixPresent) != 0,
            .vui_parameters_present_flag = (sps.Flags & H264::SPSFlags_VUIParametersPresent) != 0
        };

        StdVideoH264SequenceParameterSet vSPS =
        {
            .flags = vSPSFlags,
            .profile_idc = (StdVideoH264ProfileIdc)sps.ProfileIDC,
            .level_idc = ConvertLevelIDCVulkan(sps.LevelIDC),
            .chroma_format_idc = (StdVideoH264ChromaFormatIdc)sps.ChromaFormatIDC,
            .seq_parameter_set_id = sps.SeqParameterSetID,
            .bit_depth_luma_minus8 = sps.BitDepthLumaMinus8,
            .bit_depth_chroma_minus8 = sps.BitDepthChromaMinus8,
            .log2_max_frame_num_minus4 = sps.Log2MaxFrameNumMinus4,
            .pic_order_cnt_type = (StdVideoH264PocType)sps.PICOrderCNTType,
        };

        vulkanSPS[i] = vSPS;
    }

    const uint32_t ppsCount = clip->GetPPSCount();
    const H264::PPS* ppsData = clip->GetPPSData();

    const uint32_t videoQueueIndex = m_engine->GetVideoDecodeIndex();

    const uint32_t width = glm::min(clip->GetPaddedWidth(), videoCapabilities->VideoCapabilities.maxCodedExtent.width);
    const uint32_t height = glm::min(clip->GetPaddedHeight(), videoCapabilities->VideoCapabilities.maxCodedExtent.height);

    const uint32_t dpbSlots = glm::min(refFrames + 1, videoCapabilities->VideoCapabilities.maxDpbSlots);

    const vk::VideoSessionCreateInfoKHR videoSessionInfo = vk::VideoSessionCreateInfoKHR
    (
        videoQueueIndex,
        { },
        &videoCapabilities->VideoProfile,
        vk::Format::eG8B8R82Plane420Unorm,
        vk::Extent2D(width, height),
        vk::Format::eG8B8R82Plane420Unorm,
        dpbSlots,
        refFrames * 2,
        &videoCapabilities->VideoCapabilities.stdHeaderVersion
    );
    VKRESERR(device.createVideoSessionKHR(&videoSessionInfo, nullptr, &m_videoSession));

    // const vk::VideoDecodeH264SessionParametersAddInfoKHR h264ParamAddInfo = vk::VideoDecodeH264SessionParametersAddInfoKHR
    // (
    //     spsCount,
    //     (StdVideoH264SequenceParameterSet*)vulkanSPS,
    // );

    vk::VideoDecodeH264SessionParametersCreateInfoKHR h264ParamInfo = vk::VideoDecodeH264SessionParametersCreateInfoKHR
    (
        spsCount,
        ppsCount
    );
}
VulkanVideoTexture::~VulkanVideoTexture()
{
    for (uint32_t i = 0; i < VideoBufferCount; ++i)
    {
        const bool valid = m_buffers[i].Buffer != vk::Buffer(nullptr);
        if (valid)
        {
            m_engine->PushDeletionObject(new VideoBufferVulkanDeletionObject(m_engine, m_buffers[i]));
        }
    }
}

void VulkanVideoTexture::UpdateVulkan(vk::CommandBuffer a_commandBuffer, double a_delta)
{
    IDEFER(m_time = glm::min(m_time + a_delta, m_duration));

    const uint32_t decodeFrame = (uint32_t)(m_time * m_fps);

    while (m_time > m_buffers[m_currentVideoBuffer].EndTimeStamp)
    {
        const float frameStep = 1.0f / m_fps;

        const uint32_t nextIndex = (m_currentVideoBuffer + 1) % VideoBufferCount;
        IDEFER(m_currentVideoBuffer = nextIndex);

        const bool valid = m_buffers[m_currentVideoBuffer].Buffer != vk::Buffer(nullptr);
        if (valid)
        {
            m_engine->PushDeletionObject(new VideoBufferVulkanDeletionObject(m_engine, m_buffers[m_currentVideoBuffer]));
        }

        const VideoClip* clip = VideoManager::GetVideoClip(m_videoAddr);
        IVERIFY(clip != nullptr);

        const VulkanVideoDecodeCapabilities* videoCapabilities = m_engine->GetVideoDecodeCapabilities();

        const double v = GenerateClipBuffer(clip, videoCapabilities, m_buffers[nextIndex].EndTimeStamp + frameStep, m_currentVideoBuffer);
        if (v < 0)
        {
            break;
        }
    }

    vk::VideoBeginCodingInfoKHR beginInfo = vk::VideoBeginCodingInfoKHR
    (
        { },
        m_videoSession
    );
    a_commandBuffer.beginVideoCodingKHR(&beginInfo);

    constexpr vk::VideoEndCodingInfoKHR EndInfo;
    a_commandBuffer.endVideoCodingKHR(&EndInfo);
}

#endif