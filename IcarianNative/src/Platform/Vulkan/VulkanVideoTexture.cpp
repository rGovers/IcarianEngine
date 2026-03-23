// Icarian Engine - C# Game Engine
// 
// License at end of file.

#ifdef ICARIANNATIVE_ENABLE_GRAPHICS_VULKAN

#include "Rendering/Vulkan/VulkanVideoTexture.h"

#include "Core/Bitfield.h"
#include "Rendering/Video/VideoClip.h"
#include "Rendering/Video/VideoInfo/H264VideoInfo.h"
#include "Rendering/Video/VideoManager.h"
#include "Rendering/Vulkan/VulkanRenderEngineBackend.h"
#include "Rendering/Vulkan/VulkanTexture.h"

class VulkanVideoSessionDeletionObject : public VulkanDeletionObject
{
private:
    VulkanRenderEngineBackend*    m_engine;
    vk::VideoSessionKHR           m_videoSession;
    vk::VideoSessionParametersKHR m_sessionParamaters;

protected:

public:
    VulkanVideoSessionDeletionObject(VulkanRenderEngineBackend* a_engine, vk::VideoSessionKHR a_videoSession, vk::VideoSessionParametersKHR a_sessionParamaters)
    {
        m_engine = a_engine;
        m_videoSession = a_videoSession;
        m_sessionParamaters = a_sessionParamaters;
    }
    virtual ~VulkanVideoSessionDeletionObject()
    {

    }

    virtual void Destroy()
    {
        const vk::Device device = m_engine->GetLogicalDevice();

        device.destroyVideoSessionKHR(m_videoSession);
        device.destroyVideoSessionParametersKHR(m_sessionParamaters);
    }
};

class VulkanVideoBufferDeletionObject : public VulkanDeletionObject
{
private:
    VulkanRenderEngineBackend* m_engine;

    vk::Buffer                 m_buffer;
    VmaAllocation              m_allocation;

protected:

public:
    VulkanVideoBufferDeletionObject(VulkanRenderEngineBackend* a_engine, VmaAllocation a_allocation, vk::Buffer a_buffer)
    {
        m_engine = a_engine;

        m_allocation = a_allocation;
        m_buffer = a_buffer;
    }
    virtual ~VulkanVideoBufferDeletionObject()
    {

    }

    virtual void Destroy()
    {
        const vk::Device device = m_engine->GetLogicalDevice();
        const VmaAllocator allocator = m_engine->GetVMAAllocator();

        device.destroyBuffer(m_buffer);

        vmaFreeMemory(allocator, m_allocation);
    }
};

class VulkanVideoAllocationDeletionObject : public VulkanDeletionObject
{
private:
    VulkanRenderEngineBackend* m_engine;
    VmaAllocation              m_allocation;

protected:

public:
    VulkanVideoAllocationDeletionObject(VulkanRenderEngineBackend* a_engine, VmaAllocation a_allocation)
    {
        m_engine = a_engine;
        m_allocation = a_allocation;
    }
    virtual ~VulkanVideoAllocationDeletionObject()
    {

    }

    virtual void Destroy()
    {
        const VmaAllocator allocator = m_engine->GetVMAAllocator();

        vmaFreeMemory(allocator, m_allocation);
    }
};

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
        break;
    }
    }

    IERROR("Invalid level IDC");

    return STD_VIDEO_H264_LEVEL_IDC_INVALID;
}

void VulkanVideoTexture::LoadHardwarePlayback(const VideoInfo* a_info)
{
    m_vulkanVideoData = new VulkanHarwareVideoData();

    m_vulkanVideoData->StreamAllocation = nullptr;
    m_vulkanVideoData->StreamBuffer = nullptr;

    const VmaAllocator allocator = m_engine->GetVMAAllocator();
    const vk::Device device = m_engine->GetLogicalDevice();

    const VulkanVideoDecodeCapabilities* videoCapabilities = m_engine->GetVideoDecodeCapabilities();

    const H264VideoInfo* h264Info = (H264VideoInfo*)a_info;

    const uint32_t spsCount = h264Info->GetSPSCount();
    const H264::SPS* spsData = h264Info->GetSPSData();

    uint32_t refFrames = 0;

    StdVideoH264SequenceParameterSet* vulkanSPS = new StdVideoH264SequenceParameterSet[spsCount];
    IDEFER(delete[] vulkanSPS);
    StdVideoH264SequenceParameterSetVui* vulkanVUISet = (StdVideoH264SequenceParameterSetVui*)calloc(spsCount, sizeof(StdVideoH264SequenceParameterSetVui));
    IDEFER(free(vulkanVUISet));

    for (uint32_t i = 0; i < spsCount; ++i)
    {
        const H264::SPS& sps = spsData[i];

        refFrames = glm::max(refFrames, (uint32_t)sps.NumRefFrames);

        if (sps.Flags & H264::SPSFlags_VUIParametersPresent)
        {
            const StdVideoH264SpsVuiFlags vVUIFlags = 
            {
                .aspect_ratio_info_present_flag = (sps.VUI.Flags & H264::SPSVUIFlags_AspectRatioInfoPresent) != 0,
                .overscan_info_present_flag = (sps.VUI.Flags & H264::SPSVUIFlags_OverscanInfoPresent) != 0,
                .overscan_appropriate_flag = (sps.VUI.Flags & H264::SPSVUIFlags_OverscanAppropriate) != 0,
                .video_signal_type_present_flag = (sps.VUI.Flags & H264::SPSVUIFlags_VideoSignalTypePresent) != 0,
                .video_full_range_flag = (sps.VUI.Flags & H264::SPSVUIFlags_VideoFullRange) != 0,
                .color_description_present_flag = (sps.VUI.Flags & H264::SPSVUIFlags_ColourDescriptionPresent) != 0,
                .chroma_loc_info_present_flag = (sps.VUI.Flags & H264::SPSVUIFlags_ChromaLOCInfoPresent) != 0,
                .timing_info_present_flag = (sps.VUI.Flags & H264::SPSVUIFlags_TimingInfoPresent) != 0,
                .fixed_frame_rate_flag = (sps.VUI.Flags & H264::SPSVUIFlags_FixedFrameRate) != 0,
                .bitstream_restriction_flag = (sps.VUI.Flags & H264::SPSVUIFlags_BitstreamRestriction) != 0,
                .nal_hrd_parameters_present_flag = (sps.VUI.Flags & H264::SPSVUIFlags_NALHRDParametersPresent) != 0,
                .vcl_hrd_parameters_present_flag = (sps.VUI.Flags & H264::SPSVUIFlags_VCLHRDParametersPresent) != 0 
            };

            vulkanVUISet[i] = StdVideoH264SequenceParameterSetVui
            {
                .flags = vVUIFlags,
                .aspect_ratio_idc = (StdVideoH264AspectRatioIdc)sps.VUI.AspectRatioIDC,
                .sar_width = sps.VUI.SARWidth,
                .sar_height = sps.VUI.SARHeight,
                .video_format = sps.VUI.VideoFormat,
                .colour_primaries = sps.VUI.ColourPrimaries,
                .transfer_characteristics = sps.VUI.TransferCharacteristics,
                .matrix_coefficients = sps.VUI.MatrixCoefficients,
                .num_units_in_tick = sps.VUI.NumUnitsInTick,
                .time_scale = sps.VUI.TimeScale,
                .max_num_reorder_frames = sps.VUI.NumReorderFrames,
                .max_dec_frame_buffering = sps.VUI.MaxDecFrameBuffering,
            };
        }

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

        vulkanSPS[i] = StdVideoH264SequenceParameterSet
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
            .offset_for_non_ref_pic = sps.OffsetForNonRefPic,
            .offset_for_top_to_bottom_field = sps.OffsetForTopToBottomField,
            .log2_max_pic_order_cnt_lsb_minus4 = sps.Log2MaxPicOrderCNTLSBMinus4,
            .num_ref_frames_in_pic_order_cnt_cycle = sps.NumRefFramesInPICOrderCNTCycle,
            .max_num_ref_frames = sps.NumRefFrames,
            .pic_width_in_mbs_minus1 = sps.PICWidthInMBSMinus1,
            .pic_height_in_map_units_minus1 = sps.PICHeightInMapUnitsMinus1,
            .frame_crop_left_offset = sps.FrameCropLeftOffset,
            .frame_crop_right_offset = sps.FrameCropRightOffset,
            .frame_crop_top_offset = sps.FrameCropTopOffset,
            .frame_crop_bottom_offset = sps.FrameCropBottomOffset,
            .pOffsetForRefFrame = sps.OffsetForRefFrame,
            .pSequenceParameterSetVui = &(vulkanVUISet[i])
        };
    }

    const uint32_t ppsCount = h264Info->GetPPSCount();
    const H264::PPS* ppsData = h264Info->GetPPSData();

    StdVideoH264PictureParameterSet* vulkanPPS = new StdVideoH264PictureParameterSet[ppsCount];
    IDEFER(delete[] vulkanPPS);
    StdVideoH264ScalingLists* scalingLists = (StdVideoH264ScalingLists*)calloc(ppsCount, sizeof(StdVideoH264ScalingLists));
    IDEFER(free(scalingLists));

    for (uint32_t i = 0; i < ppsCount; ++i)
    {
        const H264::PPS& pps = ppsData[i];

        StdVideoH264ScalingLists lists;
        lists.scaling_list_present_mask = pps.PICScalingListPresentFlag;
        lists.use_default_scaling_matrix_mask = pps.UseDefaultScalingMatrixFlag;

        for (uint32_t j = 0; j < 6; ++j)
        {
            for (uint32_t k = 0; k < 16; ++k)
            {
                lists.ScalingList4x4[j][k] = pps.ScalingList4x4[j][k];
            }
        }

        for (uint32_t j = 0; j < 2; ++j)
        {
            for (uint32_t k = 0; k < 64; ++k)
            {
                lists.ScalingList8x8[j][k] = pps.ScalingList8x8[j][k];
            }
        }

        scalingLists[i] = lists;

        const StdVideoH264PpsFlags vPPSFlags = 
        {
            .transform_8x8_mode_flag = (pps.Flags & H264::PPSFlags_Transform8x8ModeFlag) != 0,
            .redundant_pic_cnt_present_flag = (pps.Flags & H264::PPSFlags_RedundantPICCNTPresentFlag) != 0,
            .constrained_intra_pred_flag = (pps.Flags & H264::PPSFlags_ConstrainedIntraPred) != 0,
            .deblocking_filter_control_present_flag = (pps.Flags & H264::PPSFlags_DeblockingFilterControlPresent) != 0,
            .weighted_pred_flag = (pps.Flags & H264::PPSFlags_WeightedPred) != 0,
            .bottom_field_pic_order_in_frame_present_flag = (pps.Flags & H264::PPSFlags_PICOrderPresent) != 0,
            .entropy_coding_mode_flag = (pps.Flags & H264::PPSFlags_EntropyCodingMode) != 0,
            .pic_scaling_matrix_present_flag = (pps.Flags & H264::PPSFlags_PICScalingMatrixPresent) != 0
        };

        vulkanPPS[i] = StdVideoH264PictureParameterSet
        {
            .flags = vPPSFlags,
            .seq_parameter_set_id = pps.SEQParameterSetID,
            .pic_parameter_set_id = pps.PICParameterSetID,
            .num_ref_idx_l0_default_active_minus1 = pps.NumRefIdxl0ActiveMinus1,
            .num_ref_idx_l1_default_active_minus1 = pps.NumRefIdxl1ActiveMinus1,
            .weighted_bipred_idc = (StdVideoH264WeightedBipredIdc)pps.WeightedBipredIDC,
            .pic_init_qp_minus26 = pps.PICInitQPMinus26,
            .pic_init_qs_minus26 = pps.PICInitQSMinus26,
            .chroma_qp_index_offset = pps.ChromaQPIndexOffset,
            .second_chroma_qp_index_offset = pps.SecondChromaQPIndexOffset,
            .pScalingLists = &(scalingLists[i]),
        };
    }

    const uint32_t videoQueueIndex = m_engine->GetVideoDecodeIndex();
    const uint32_t width = h264Info->GetPaddedWidth();
    const uint32_t height = h264Info->GetPaddedHeight();

    if (width >= videoCapabilities->VideoCapabilities.maxCodedExtent.width)
    {
        IERROR("Video width higher then Video Capabilities");
    }
    if (height >= videoCapabilities->VideoCapabilities.maxCodedExtent.height)
    {
        IERROR("Video height higher then Video Capabilities");
    }

    m_vulkanVideoData->DPBSlots = glm::min(refFrames + 1, videoCapabilities->VideoCapabilities.maxDpbSlots);
    const vk::Extent2D extents = vk::Extent2D(width, height);

    m_vulkanVideoData->VideoTexture = new VulkanTexture(m_engine, width, height, TextureFormat_NV12, m_vulkanVideoData->DPBSlots);

    const vk::VideoSessionCreateInfoKHR videoSessionInfo = vk::VideoSessionCreateInfoKHR
    (
        videoQueueIndex,
        { },
        &videoCapabilities->VideoProfile,
        vk::Format::eG8B8R82Plane420Unorm,
        extents,
        vk::Format::eG8B8R82Plane420Unorm,
        m_vulkanVideoData->DPBSlots,
        refFrames * 2,
        &videoCapabilities->VideoCapabilities.stdHeaderVersion
    );

    VKRESERR(device.createVideoSessionKHR(&videoSessionInfo, nullptr, &m_vulkanVideoData->VideoSession));

    const vk::VideoDecodeH264SessionParametersAddInfoKHR h264ParamAddInfo = vk::VideoDecodeH264SessionParametersAddInfoKHR
    (
        spsCount,
        vulkanSPS,
        ppsCount,
        vulkanPPS
    );

    const vk::VideoDecodeH264SessionParametersCreateInfoKHR h264DecodeParamInfo = vk::VideoDecodeH264SessionParametersCreateInfoKHR
    (
        spsCount,
        ppsCount,
        &h264ParamAddInfo
    );

    const vk::VideoSessionParametersCreateInfoKHR h264ParamInfo = vk::VideoSessionParametersCreateInfoKHR
    (
        { },
        nullptr,
        m_vulkanVideoData->VideoSession,
        &h264DecodeParamInfo
    );

    VKRESERR(device.createVideoSessionParametersKHR(&h264ParamInfo, nullptr, &m_vulkanVideoData->SessionParameters));

    VKRESERR(device.getVideoSessionMemoryRequirementsKHR(m_vulkanVideoData->VideoSession, &m_vulkanVideoData->MaxBuffers, nullptr));
    IVERIFY(m_vulkanVideoData->MaxBuffers <= VulkanHarwareVideoData::VideoBufferCount);

    vk::VideoSessionMemoryRequirementsKHR* requirements = new vk::VideoSessionMemoryRequirementsKHR[m_vulkanVideoData->MaxBuffers];
    IDEFER(delete[] requirements);
    vk::BindVideoSessionMemoryInfoKHR* bindInfo = new vk::BindVideoSessionMemoryInfoKHR[m_vulkanVideoData->MaxBuffers];
    IDEFER(delete[] bindInfo);

    VKRESERR(device.getVideoSessionMemoryRequirementsKHR(m_vulkanVideoData->VideoSession, &m_vulkanVideoData->MaxBuffers, requirements));

    for (uint32_t i = 0; i < m_vulkanVideoData->MaxBuffers; ++i)
    {
        const VkVideoSessionMemoryRequirementsKHR r = requirements[i];

        const VmaAllocationCreateInfo allocCreateInfo = 
        {
            .memoryTypeBits = r.memoryRequirements.memoryTypeBits
        };
        
        VmaAllocationInfo allocInfo;
        VKRESERR(vmaAllocateMemory(allocator, &r.memoryRequirements, &allocCreateInfo, &m_vulkanVideoData->Allocations[i], &allocInfo));

        const vk::BindVideoSessionMemoryInfoKHR bInfo = vk::BindVideoSessionMemoryInfoKHR
        (
            r.memoryBindIndex,
            allocInfo.deviceMemory,
            allocInfo.offset,
            allocInfo.size
        );

        bindInfo[i] = bInfo;
    }

    VKRESERR(device.bindVideoSessionMemoryKHR(m_vulkanVideoData->VideoSession, m_vulkanVideoData->MaxBuffers, bindInfo));
}

VulkanVideoTexture::VulkanVideoTexture(VulkanRenderEngineBackend* a_engine, uint32_t a_videoAddr)
{
    m_engine = a_engine;

    m_videoAddr = a_videoAddr;

    m_vulkanVideoData = nullptr;
    m_lastFrame = -1;
    m_lastIntra = 0;

    const VideoClip* clip = VideoManager::GetVideoClip(m_videoAddr);
    IVERIFY(clip != nullptr);
    IVERIFY(clip->IsValid());

    const VideoInfo* info = clip->GetVideoInfo();
    IVERIFY(info != nullptr);
    IVERIFY(info->IsValid());

    switch (info->GetVideoProfile())
    {
    case VideoProfile_H264:
    {
// Exists for testing can switch to 0 without going to hunt for the extension
#if 1
        if (m_engine->IsVideoEnabled())
        {
            LoadHardwarePlayback(info);

            break;
        }
#endif
        IERROR("Software decode not implemented yet");

        break;
    }
    default:
    {
        IERROR("Invalid video profile");

        break;
    }
    }
}
VulkanVideoTexture::~VulkanVideoTexture()
{
    if (m_vulkanVideoData != nullptr)
    {
        if (m_vulkanVideoData->StreamAllocation != nullptr)
        {
            m_engine->PushDeletionObject<VulkanVideoBufferDeletionObject>(m_engine, m_vulkanVideoData->StreamAllocation, m_vulkanVideoData->StreamBuffer);
        }

        m_engine->PushDeletionObject<VulkanVideoSessionDeletionObject>(m_engine, m_vulkanVideoData->VideoSession, m_vulkanVideoData->SessionParameters);

        for (uint32_t i = 0; i < m_vulkanVideoData->MaxBuffers; ++i)
        {
            m_engine->PushDeletionObject<VulkanVideoAllocationDeletionObject>(m_engine, m_vulkanVideoData->Allocations[i]);
        }

        delete m_vulkanVideoData->VideoTexture;

        delete m_vulkanVideoData;
    }
}

void VulkanVideoTexture::UpdateVulkan(vk::CommandBuffer a_commandBuffer, double a_delta)
{
    const VmaAllocator allocator = m_engine->GetVMAAllocator();

    VideoClip* clip = VideoManager::GetVideoClip(m_videoAddr);
    IVERIFY(clip != nullptr);
    IVERIFY(clip->IsValid());

    const VideoInfo* info = clip->GetVideoInfo();
    IVERIFY(info != nullptr);
    IVERIFY(info->IsValid());

    const e_VideoProfile videoProfile = info->GetVideoProfile();

    vk::VideoReferenceSlotInfoKHR referenceSlots[VulkanHarwareVideoData::TotalDBPFrames];
    vk::VideoPictureResourceInfoKHR pictureResource[VulkanHarwareVideoData::TotalDBPFrames];
    vk::VideoDecodeH264DpbSlotInfoKHR dpbSlots[VulkanHarwareVideoData::TotalDBPFrames];
    StdVideoDecodeH264ReferenceInfo referenceInfos[VulkanHarwareVideoData::TotalDBPFrames];

    const double time = clip->GetTime();
    IDEFER(
        // Maybe move this out of the VideoTexture?
        if (clip->GetUpdateMode() == VideoUpdateMode_Video)
        {
            clip->SetTime(time + a_delta);
        });

    uint32_t currentFrame = -1;
    uint32_t nextIntra = -1;
    bool write = false;

    switch (videoProfile)
    {
    case VideoProfile_H264:
    {
        const H264VideoInfo* h264Info = (H264VideoInfo*)info;

        const H264VideoFrameInfo* frames = h264Info->GetFrames();
        const uint32_t frameCount = h264Info->GetFrameCount();

        // Do not want to seek through all the frames if we do not have to so keep track of the last intra frame
        for (uint32_t i = m_lastIntra; i < frameCount; ++i)
        {
            const H264VideoFrameInfo& f = frames[i];
            if (f.Type == VideoFrameType_Intra)
            {
                write = i != m_lastIntra;
                m_lastIntra = i;
            }

            if (f.TimeStamp >= time)
            {
                currentFrame = i;

                break;
            }
        }

        currentFrame = glm::min(currentFrame, frameCount);

        for (uint32_t i = currentFrame + 1; i < frameCount; ++i)
        {
            const H264VideoFrameInfo& f = frames[i];
            if (f.Type == VideoFrameType_Intra)
            {
                nextIntra = i;

                break;
            }
        }

        nextIntra = glm::min(nextIntra, frameCount);

        const VulkanTexture* tex = m_vulkanVideoData->VideoTexture;

        const uint32_t width = tex->GetWidth();
        const uint32_t height = tex->GetHeight();
        const vk::Extent2D extent = vk::Extent2D(width, height);
        const vk::ImageView view = tex->GetImageView();

        const H264::SliceHeader* headers = h264Info->GetSliceHeaderData();

        for (uint32_t i = m_lastIntra; i <= currentFrame; ++i)
        {
            const H264::SliceHeader& h = headers[i];

            const uint32_t index = i - m_lastIntra;
            const uint32_t indexWrap = index % VulkanHarwareVideoData::TotalDBPFrames; 
            const uint32_t dpbIndex = index % m_vulkanVideoData->DPBSlots;

            // Not sure if that is correct but seems to work?
            const StdVideoDecodeH264ReferenceInfoFlags flags = 
            { 
                .top_field_flag = (h.Flags & H264::SliceHeaderFlags_BottomField) == 0,
                .bottom_field_flag = (h.Flags & H264::SliceHeaderFlags_BottomField) != 0,
            };

            referenceInfos[indexWrap] = StdVideoDecodeH264ReferenceInfo
            {
                .flags = flags,
                .FrameNum = h.FrameNum,
                .PicOrderCnt = { frames[i].PrioData.POC, frames[i].PrioData.POC }
            };

            dpbSlots[indexWrap] = vk::VideoDecodeH264DpbSlotInfoKHR
            (
                &(referenceInfos[indexWrap])
            );

            constexpr vk::Offset2D Offset = vk::Offset2D(0, 0);

            pictureResource[indexWrap] = vk::VideoPictureResourceInfoKHR
            (
                Offset,
                extent,
                dpbIndex,
                view
            );

            referenceSlots[indexWrap] = vk::VideoReferenceSlotInfoKHR
            (
                dpbIndex,
                &(pictureResource[indexWrap]),
                &(dpbSlots[indexWrap])
            );
        }

        break;
    }
    default:
    {
        IERROR("Invalid video profile");

        break;
    }
    }

    if (m_lastFrame == currentFrame || m_lastIntra == nextIntra)
    {
        return;
    }
    IDEFER(m_lastFrame = currentFrame);

    const uint32_t referenceSlotCount = glm::min(currentFrame - m_lastIntra, VulkanHarwareVideoData::TotalDBPFrames);

    const vk::VideoBeginCodingInfoKHR beginInfo = vk::VideoBeginCodingInfoKHR
    (
        { },
        m_vulkanVideoData->VideoSession,
        m_vulkanVideoData->SessionParameters,
        referenceSlotCount + 1,
        referenceSlots
    );

    a_commandBuffer.beginVideoCodingKHR(&beginInfo);
    constexpr vk::VideoEndCodingInfoKHR EndInfo;
    IDEFER(a_commandBuffer.endVideoCodingKHR(&EndInfo));

    if (write || m_vulkanVideoData->StreamAllocation == nullptr)
    {
        const VulkanVideoDecodeCapabilities* capabilities = m_engine->GetVideoDecodeCapabilities();

        if (m_vulkanVideoData->StreamAllocation != nullptr)
        {
            m_engine->PushDeletionObject<VulkanVideoBufferDeletionObject>(m_engine, m_vulkanVideoData->StreamAllocation, m_vulkanVideoData->StreamBuffer);
            m_vulkanVideoData->StreamAllocation = nullptr;
            m_vulkanVideoData->StreamBuffer = nullptr;
        }

        const uint32_t alignment = (uint32_t)glm::max(capabilities->VideoCapabilities.minBitstreamBufferOffsetAlignment, capabilities->VideoCapabilities.minBitstreamBufferSizeAlignment);

        uint8_t* dat;
        if (!clip->GetVideoClipData(m_lastIntra, nextIntra, alignment, &dat, &m_vulkanVideoData->BufferSize))
        {
            return;
        }
        IDEFER(delete[] dat);

        // This makes me uncomfortable allocating on the fly but using more memory then needed also does so....
        const VkBufferCreateInfo streamBufferCreateInfo =
        {
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .flags = VK_BUFFER_CREATE_VIDEO_PROFILE_INDEPENDENT_BIT_KHR,
            .size = m_vulkanVideoData->BufferSize,
            .usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_VIDEO_DECODE_SRC_BIT_KHR,
        };

        const VmaAllocationCreateInfo streamBufferAllocInfo =
        {
            .flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT,
            .usage = VMA_MEMORY_USAGE_AUTO
        };

        VmaAllocationInfo streamAllocInfo;
        VkBuffer streamBuffer;
        VKRESERR(vmaCreateBuffer(allocator, &streamBufferCreateInfo, &streamBufferAllocInfo, &streamBuffer, &m_vulkanVideoData->StreamAllocation, &streamAllocInfo));
        m_vulkanVideoData->StreamBuffer = streamBuffer;

        if (streamAllocInfo.pMappedData != NULL)
        {
            IDEFER(vmaFlushAllocation(allocator, m_vulkanVideoData->StreamAllocation, 0, (VkDeviceSize)m_vulkanVideoData->BufferSize));

            memcpy(streamAllocInfo.pMappedData, dat, (size_t)m_vulkanVideoData->BufferSize);
        }
    }

    switch (videoProfile) 
    {
    case VideoProfile_H264:
    {
        const uint32_t curI = currentFrame - m_lastIntra;
        const uint32_t curDPBIndex = curI % m_vulkanVideoData->DPBSlots;

        const H264VideoInfo* h264Info = (H264VideoInfo*)info;

        const H264::SliceHeader* headers = h264Info->GetSliceHeaderData();
        const H264::SliceHeader& h = headers[currentFrame];

        const H264::PPS* ppsData = h264Info->GetPPSData();
        const H264::PPS& pps = ppsData[h.PICParameterSetID];

        const H264VideoFrameInfo* frames = h264Info->GetFrames();
        const H264VideoFrameInfo& f = frames[currentFrame];

        const bool isIntra = f.Type == VideoFrameType_Intra;
        const bool isReference = f.NALIDC > 0;

        const StdVideoDecodeH264PictureInfoFlags stdFlags = 
        {
            .field_pic_flag = (h.Flags & H264::SliceHeaderFlags_FieldPIC) != 0,
            .is_intra = isIntra,
            .IdrPicFlag = isIntra && isReference,
            .bottom_field_flag = (h.Flags & H264::SliceHeaderFlags_BottomField) != 0,
            .is_reference = isReference,
        };

        const StdVideoDecodeH264PictureInfo stdPictureInfo =
        {
            .flags = stdFlags,
            .seq_parameter_set_id = pps.SEQParameterSetID,
            .pic_parameter_set_id = h.PICParameterSetID,
            .frame_num = h.FrameNum,
            .idr_pic_id = h.IDRPICID,
            .PicOrderCnt = { f.PrioData.POC, f.PrioData.POC },
        };

        uint32_t sliceOffset = 0;

        const vk::VideoDecodeH264PictureInfoKHR pictureInfo = vk::VideoDecodeH264PictureInfoKHR
        (
            &stdPictureInfo,
            1,
            &sliceOffset
        );

        vk::VideoDecodeInfoKHR decodeInfo = vk::VideoDecodeInfoKHR
        (
            { },
            m_vulkanVideoData->StreamBuffer,
            0,
            (vk::DeviceSize)m_vulkanVideoData->BufferSize,
            pictureResource[curDPBIndex],
            &(referenceSlots[curDPBIndex]),
            referenceSlotCount,
            nullptr,
            &pictureInfo
        );
        if (referenceSlotCount > 0)
        {
            decodeInfo.pReferenceSlots = referenceSlots;
        }

        a_commandBuffer.decodeVideoKHR(decodeInfo);

        break;
    }
    default:
    {
        IERROR("Invalid video profile");

        break;
    }
    }
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