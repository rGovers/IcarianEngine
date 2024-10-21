// Icarian Engine - C# Game Engine
// 
// License at end of file.

#include "Rendering/Video/VideoInfo/H264VideoInfo.h"

#include "Core/IcarianError.h"
#include "DataTypes/Array.h"
#include "FileCache.h"
#include "IcarianError.h"

H264VideoInfo::H264VideoInfo(const MP4D_demux_t& a_demux, const MP4D_track_t& a_trackInfo, uint32_t a_trackIndex, FileHandle* a_handle)
{
    IERRBLOCK;

    m_width = (uint32_t)a_trackInfo.SampleDescription.video.width;
    m_height = (uint32_t)a_trackInfo.SampleDescription.video.height;

    int index = 0;

    {
        Array<H264::SPS> spsArr;

        while (1) 
        {
            int size = 0;
            const uint8_t* dat = (uint8_t*)MP4D_read_sps(&a_demux, a_trackIndex, index++, &size);
            if (dat == NULL) 
            {
                break;
            }

            H264::BitStream bitStream = H264::BitStream(dat, (uint64_t)size);

            const H264::NALHeader nal = H264::ReadNALHeader(&bitStream);
            IERRCHECK(nal.Type == H264::NALUnitType_SPS);

            const H264::SPS sps = H264::ReadSPS(&bitStream);

            const uint32_t picWidth = (sps.PICWidthInMBSMinus1 + 1) * 16;
            const uint32_t picHeight = (sps.PICHeightInMapUnitsMinus1 + 1) * 16;

            const bool frame = (sps.Flags & H264::SPSFlags_FrameMBSOnly) != 0;

            const uint32_t width = picWidth - sps.FrameCropLeftOffset * 2 - sps.FrameCropRightOffset * 2;
            const uint32_t height = (2 - frame) * picHeight - sps.FrameCropTopOffset * 2 - sps.FrameCropBottomOffset * 2;
            IERRCHECK(m_width == width);
            IERRCHECK(m_height == height);

            m_paddedWidth = picWidth;
            m_paddedHeight = picHeight;

            spsArr.Push(sps);
        }

        m_spsCount = spsArr.Size();

        m_sps = new H264::SPS[m_spsCount];
        IERRDEFER(
        {
            m_spsCount = 0;

            delete[] m_sps;
            m_sps = nullptr;
        });

        for (uint32_t i = 0; i < m_spsCount; ++i)
        {
            m_sps[i] = spsArr[i];
        }
    }

    {
        Array<H264::PPS> ppsArr;

        index = 0;
        while (1) 
        {
            int size = 0;
            const uint8_t* dat = (uint8_t*)MP4D_read_pps(&a_demux, a_trackIndex, index++, &size);
            if (dat == NULL) 
            {
                break;
            }

            H264::BitStream bitStream = H264::BitStream(dat, (uint64_t)size);

            const H264::NALHeader nal = H264::ReadNALHeader(&bitStream);
            IERRCHECK(nal.Type == H264::NALUnitType_PPS);

            const H264::PPS pps = H264::ReadPPS(&bitStream);
            ppsArr.Push(pps);
        }

        m_ppsCount = ppsArr.Size();

        m_pps = new H264::PPS[m_ppsCount];
        IERRDEFER(
        {
            m_ppsCount = 0;

            delete[] m_pps;
            m_pps = nullptr;
        });

        for (uint32_t i = 0; i < m_ppsCount; ++i)
        {
            m_pps[i] = ppsArr[i];
        }
    }

    Array<H264VideoFrameInfo> frames;

    uint32_t pocCycle = 0;
    uint32_t prevPICOrderCNTLSB = 0;
    uint32_t prevPICOrderCNTMSB = 0;

    uint32_t trackDuration = 0;

    const double invTimescale = 1.0 / a_trackInfo.timescale;

    for (uint32_t i = 0; i < a_trackInfo.sample_count; ++i) 
    {
        unsigned int frameBytes;
        unsigned int timestamp;
        unsigned int duration;
        const MP4D_file_offset_t offset = MP4D_frame_offset(&a_demux, a_trackIndex, i, &frameBytes, &timestamp, &duration);
        trackDuration += duration;

        a_handle->Seek(offset);
        uint8_t* dat = new uint8_t[frameBytes];
        IDEFER(delete[] dat);
        a_handle->Read(dat, (uint64_t)frameBytes);

        const uint8_t* p = dat;
        while (frameBytes > 0) 
        {
            const uint32_t size = (((uint32_t)p[0] << 24) | ((uint32_t)p[1] << 16) | ((uint32_t)p[2] << 8) | p[3]) + 4;
            IERRCHECK(frameBytes >= size);

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

            const H264VideoFrameInfo info = 
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

    m_frameCount = frames.Size();
    m_frames = new H264VideoFrameInfo[m_frameCount];
    IERRDEFER(
    {
        delete[] m_frames;
        m_frames = nullptr;
    });

    // This is annoying as if the file is presorted then this way is faster but
    // if it has not then forward iteration is faster in theory ahhh...... File
    // spec does not guarantee order and requireds a specific order Running with
    // this as test files are presorted so cannot validate
    for (uint32_t i = 0; i < m_frameCount; ++i) 
    {
        const H264VideoFrameInfo& frameA = frames[i];

        for (int64_t j = (int64_t)i - 1; j >= 0; --j) 
        {
            const H264VideoFrameInfo& frameB = m_frames[j];

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

    m_fps = (float)((double)a_trackInfo.timescale / trackDuration * a_trackInfo.sample_count);
    m_duration = trackDuration * invTimescale;
}
H264VideoInfo::~H264VideoInfo()
{
    if (m_pps != nullptr)
    {
        delete[] m_pps;
    }
    if (m_sps != nullptr)
    {
        delete[] m_sps;
    }

    if (m_frames != nullptr)
    {
        delete[] m_frames;
    }
}
bool H264VideoInfo::GetVideoClipData(FileHandle* a_handle, double a_inTimeStamp, uint32_t* a_startIndex, uint32_t* a_endIndex, uint8_t** a_data, uint32_t* a_size)
{
    IERRBLOCK;

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

    IERRCHECKRET(startIndex < m_frameCount, false);
    IERRCHECKRET(endIndex < m_frameCount, false);
    IERRCHECKRET(startIndex != endIndex, false);

    if (a_startIndex != nullptr)
    {
        *a_startIndex = startIndex;
    }
    IERRDEFER(
        if (a_startIndex != nullptr)
        {
            *a_startIndex = -1;
        });
    if (a_endIndex != nullptr)
    {
        *a_endIndex = endIndex;
    }
    IERRDEFER(
        if (a_endIndex != nullptr)
        {
            *a_endIndex = -1;
        });

    uint32_t size = 0;
    for (uint32_t i = startIndex; i < endIndex; ++i)
    {
        size += m_frames[i].Size;
    }

    *a_size = size;
    // Should use calloc but cannot communicate to end user to use free instead of delete so new and memset it is
    *a_data = new uint8_t[size];
    IERRDEFER(
    {
        *a_size = 0;

        delete[] *a_data;
        *a_data = nullptr;
    });

    uint8_t* p = *a_data;
    // Can skip bits so just zero them
    memset(p, 0, size);

    // Oversized but better over then under and/or multiple allocations
    uint8_t* readBuff = new uint8_t[size + 4];
    IDEFER(delete[] readBuff);
    for (uint32_t i = startIndex; i < endIndex; ++i)
    {
        const H264VideoFrameInfo& frame = m_frames[i];

        IERRCHECKRET(a_handle->Seek(frame.Offset), false);
        IERRCHECKRET(a_handle->Read(readBuff, frame.Size) == frame.Size, false);

        const uint8_t* offBuf = readBuff + 4;

        H264::BitStream bitStream = H264::BitStream(offBuf, frame.Size);
                    
        const H264::NALHeader nal = H264::ReadNALHeader(&bitStream);
        IERRCHECKRET(nal.Type == H264::NALUnitType_CodedSliceIDR || nal.Type == H264::NALUnitType_CodedSliceNonIDR, false);

        memcpy(p, H264::NALStartCode, H264::NALStartCodeSize);
        memcpy(p + H264::NALStartCodeSize, offBuf, frame.Size - 4);

        p += frame.Size;
    }

    return true;
}

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