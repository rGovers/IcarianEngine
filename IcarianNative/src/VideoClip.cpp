#include "Rendering/Video/VideoClip.h"

#define GLM_FORCE_SWIZZLE
#include <glm/glm.hpp>

#include <minimp4.h>

#include "Core/IcarianDefer.h"
#include "FileCache.h"
#include "IcarianError.h"
#include "Rendering/Video/H264.h"

static int ReadCallback(int64_t a_offset, void* a_buffer, size_t a_size, void* a_token)
{
    FileHandle* handle = (FileHandle*)a_token;

    const uint64_t fileOffset = handle->GetOffset();
    if (a_offset != fileOffset)
    {
        handle->Ignore(a_offset - fileOffset);
    }

    return handle->Read(a_buffer, a_size) != a_size;
}

VideoClip::VideoClip(const std::filesystem::path& a_path)
{
    m_videoProfile = VideoProfile_Null;

    m_width = 0;
    m_height = 0;

    m_paddedWidth = 0;
    m_paddedHeight = 0;

    m_frameCount = 0;
    m_frames = nullptr;

    m_duration = 0.0;
    m_fps = 0.0;

    FileHandle* handle = FileCache::LoadFile(a_path);
    IVERIFY(handle != nullptr);
    IDEFER(delete handle);

    m_path = a_path;

    MP4D_demux_t demux = { };
    if (MP4D_open(&demux, ReadCallback, handle, (int64_t)handle->GetSize()) != 1)
    {
        IERROR("Failed opening VideoClip");
    }
    IDEFER(MP4D_close(&demux));

    for (unsigned int i = 0; i < demux.track_count; ++i)
    {
        const MP4D_track_t& track = demux.track[i];

        switch (track.handler_type)
        {
        case MP4D_HANDLER_TYPE_VIDE:
        {
            switch (track.object_type_indication) 
            {
            case MP4_OBJECT_TYPE_AVC:
            {
                m_videoProfile = VideoProfile_H264;

                break;
            }
            default:
            {
                IERROR("Unsupported Video Profile");

                break;
            }
            }

            m_width = (uint32_t)track.SampleDescription.video.width;
            m_height = (uint32_t)track.SampleDescription.video.height;

            int index = 0;
            while (1)
            {
                int size = 0;
                const uint8_t* dat = (uint8_t*)MP4D_read_sps(&demux, i, index++, &size);  
                if (dat == NULL)
                {
                    break;
                }

                H264::BitStream bitStream = H264::BitStream(dat, (uint64_t)size);

                const H264::NALHeader nal = H264::ReadNALHeader(&bitStream);
                IVERIFY(nal.Type == H264::NALUnitType_SPS);

                const H264::SPS sps = H264::ReadSPS(&bitStream);

                const uint32_t picWidth = (sps.PICWidthInMBSMinus1 + 1) * 16;
                const uint32_t picHeight = (sps.PICHeightInMapUnitsMinus1 + 1) * 16;

                const bool frame = (sps.Flags & H264::SPSFlags_FrameMBSOnly) != 0;

                const uint32_t width = picWidth - sps.FrameCropLeftOffset * 2 - sps.FrameCropRightOffset * 2;
                const uint32_t height = (2 - frame) * picHeight - sps.FrameCropTopOffset * 2 - sps.FrameCropBottomOffset * 2;
                IVERIFY(m_width == width);
                IVERIFY(m_height == height);

                m_paddedWidth = picWidth;
                m_paddedHeight = picHeight;

                m_sps.Push(sps);
            }

            index = 0;
            while (1)
            {
                int size = 0;
                const uint8_t* dat = (uint8_t*)MP4D_read_pps(&demux, i, index++, &size);
                if (dat == NULL)
                {
                    break;
                }

                H264::BitStream bitStream = H264::BitStream(dat, (uint64_t)size);

                const H264::NALHeader nal = H264::ReadNALHeader(&bitStream);
                IVERIFY(nal.Type == H264::NALUnitType_PPS);

                const H264::PPS pps = H264::ReadPPS(&bitStream);
                m_pps.Push(pps);
            }


            Array<VideoFrameInfo> frames;

            uint32_t pocCycle = 0;
            uint32_t prevPICOrderCNTLSB = 0;
            uint32_t prevPICOrderCNTMSB = 0;

            uint32_t trackDuration = 0;

            const double invTimescale = 1.0 / track.timescale;

            for (uint32_t j = 0; j < track.sample_count; ++j)
            {
                unsigned int frameBytes;
                unsigned int timestamp;
                unsigned int duration;
                const MP4D_file_offset_t offset = MP4D_frame_offset(&demux, i, j, &frameBytes, &timestamp, &duration);
                trackDuration += duration;

                handle->Seek(offset);
                uint8_t* dat = new uint8_t[frameBytes];
                IDEFER(delete[] dat);
                handle->Read(dat, (uint64_t)frameBytes);

                const uint8_t* p = dat;
                while (frameBytes > 0) 
                {
                    const uint32_t size = (((uint32_t)p[0] << 24) | ((uint32_t)p[1] << 16) | ((uint32_t)p[2] << 8) | p[3]) + 4;
                    IVERIFY(frameBytes >= size);

                    H264::BitStream bitStream = H264::BitStream(p + 4, frameBytes);
                    
                    const H264::NALHeader nal = H264::ReadNALHeader(&bitStream);

                    e_VideoFrameType type;
                    switch (nal.Type) 
                    {
                    case H264::NALUnitType_CodedSliceIDR:
                    {
                        type = VideoFrameType_Intra;

                        break;
                    }
                    case H264::NALUnitType_CodedSliceNonIDR:
                    {
                        type = VideoFrameType_Predictive;

                        break;
                    }
                    default:
                    {
                        frameBytes -= size;
                        p += size;

                        continue;
                    }
                    }

                    const H264::SliceHeader header = H264::ReadSliceHeader(&bitStream, nal, m_pps, m_sps);

                    if (header.PICOrderCNTLSB == 0)
                    {
                        ++pocCycle;
                    }

                    const H264::PPS& pps = m_pps[header.PICParameterSetID];
                    const H264::SPS& sps = m_sps[pps.SEQParameterSetID];

                    const uint32_t maxPICOrderCNTLSB = 1 << (sps.Log2MaxPicOrderCNTLSBMinus4 + 4);
                    const uint32_t halfMaxPICOrderCNTLSB = maxPICOrderCNTLSB / 2;

                    uint32_t picOrderCNTMSB = prevPICOrderCNTMSB;
                    if (header.PICOrderCNTLSB < prevPICOrderCNTLSB && (prevPICOrderCNTLSB - header.PICOrderCNTLSB) >= halfMaxPICOrderCNTLSB)
                    {
                        picOrderCNTMSB = prevPICOrderCNTMSB + maxPICOrderCNTLSB;
                    }
                    else if (header.PICOrderCNTLSB > prevPICOrderCNTLSB && (header.PICOrderCNTLSB - prevPICOrderCNTLSB) >= halfMaxPICOrderCNTLSB)
                    {
                        picOrderCNTMSB = prevPICOrderCNTMSB - maxPICOrderCNTLSB;
                    }

                    prevPICOrderCNTLSB = header.PICOrderCNTLSB;
                    prevPICOrderCNTMSB = picOrderCNTMSB;

                    const VideoFrameInfo info = 
                    {
                        .Type = type,
                        .PrioData = 
                        {
                            .POC = picOrderCNTMSB + header.PICOrderCNTLSB,
                            .GOP = pocCycle - 1,
                        },
                        .Size = size,
                        .TimeStamp = timestamp * invTimescale,
                        .Offset = (uint64_t)offset + (p - dat),
                    };

                    frames.Push(info);

                    break;
                }
            }

            IVERIFY(m_frames == nullptr);
            m_frameCount = frames.Size();
            m_frames = new VideoFrameInfo[m_frameCount];

            // This is annoying as if the file is presorted then this way is faster but if it has not then forward iteration is faster in theory ahhh......
            // File spec does not guarantee order and requireds a specific order
            // Running with this as test files are presorted so cannot validate
            for (uint32_t i = 0; i < m_frameCount; ++i)
            {
                const VideoFrameInfo& frameA = frames[i];

                for (int64_t j = (int64_t)i - 1; j >= 0; --j)
                {
                    const VideoFrameInfo& frameB = m_frames[j];

                    if (frameB.PrioData.Priority < frameA.PrioData.Priority)
                    {
                        m_frames[j + 1] = frameA;

                        goto NextFrame;
                    }

                    m_frames[j + 1] = m_frames[j];
                }

                m_frames[i] = frameA;

                NextFrame:;
            }

            m_fps = (float)((double)track.timescale / trackDuration * track.sample_count);
            m_duration = trackDuration * invTimescale;

            break;
        }
        case MP4D_HANDLER_TYPE_SOUN:
        {
            // TODO: Implement me!

            break;
        }
        }
    }
}
VideoClip::~VideoClip()
{
    if (m_frames != nullptr)
    {
        delete[] m_frames;
    }
}

bool VideoClip::GetVideoClipData(double a_inTimeStamp, uint32_t* a_startIndex, uint32_t* a_endIndex, uint8_t** a_data, uint32_t* a_size) const
{
    IVERIFY(a_size != nullptr);
    IVERIFY(a_data != nullptr);

    if (a_startIndex != nullptr)
    {
        *a_startIndex = -1;
    }
    if (a_endIndex != nullptr)
    {
        *a_endIndex = -1;
    }

    *a_size = 0;
    *a_data = nullptr;

    uint32_t startIndex = 0;
    uint32_t endIndex = m_frameCount;
    for (uint32_t i = 0; i < m_frameCount; ++i)
    {
        if (m_frames[i].TimeStamp > a_inTimeStamp)
        {
            for (uint32_t j = i; j < m_frameCount; ++j)
            {
                if (m_frames[j].Type == VideoFrameType_Intra)
                {
                    endIndex = j;

                    break;
                }
            }

            goto FoundIntra;
        }

        if (m_frames[i].Type == VideoFrameType_Intra)
        {
            startIndex = i;
        }
    }

    return false;

    FoundIntra:;

    IVERIFY(startIndex < m_frameCount);
    IVERIFY(endIndex < m_frameCount);
    IVERIFY(startIndex != endIndex);

    if (a_startIndex != nullptr)
    {
        *a_startIndex = startIndex;
    }
    if (a_endIndex != nullptr)
    {
        *a_endIndex = endIndex;
    }

    FileHandle* handle = FileCache::LoadFile(m_path);
    IVERIFY(handle != nullptr);
    IDEFER(delete handle);

    uint32_t size = 0;
    for (uint32_t i = startIndex; i < endIndex; ++i)
    {
        size += m_frames[i].Size;
    }

    *a_size = size;
    // Should use calloc but cannot communicate to end user to use free instead of delete so new and memset it is
    *a_data = new uint8_t[size];

    uint8_t* p = *a_data;
    // Can skip bits so just zero them
    memset(p, 0, size);

    // Oversized but better over then under and/or multiple allocations
    uint8_t* readBuff = new uint8_t[size + 4];
    IDEFER(delete[] readBuff);
    for (uint32_t i = startIndex; i < endIndex; ++i)
    {
        const VideoFrameInfo& frame = m_frames[i];

        handle->Seek(frame.Offset);
        handle->Read(readBuff, frame.Size);

        const uint8_t* offBuf = readBuff + 4;

        H264::BitStream bitStream = H264::BitStream(offBuf, frame.Size);
                    
        const H264::NALHeader nal = H264::ReadNALHeader(&bitStream);
        IVERIFY(nal.Type == H264::NALUnitType_CodedSliceIDR || nal.Type == H264::NALUnitType_CodedSliceNonIDR);

        memcpy(p, H264::NALStartCode, H264::NALStartCodeSize);
        memcpy(p + H264::NALStartCodeSize, offBuf, frame.Size - 4);

        p += frame.Size;
    }

    return true;
}