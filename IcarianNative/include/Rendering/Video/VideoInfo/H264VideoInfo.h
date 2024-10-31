// Icarian Engine - C# Game Engine
// 
// License at end of file.

#pragma once

#include <minimp4.h>

#include "Rendering/Video/VideoInfo/VideoInfo.h"

#include "Rendering/Video/H264.h"

struct H264VideoFrameInfo
{
    e_VideoFrameType Type;
    union
    {
        uint64_t Priority;

        struct 
        {
            int32_t POC;
            uint32_t GOP;
        };
    } PrioData;
    uint32_t NALIDC;
    uint32_t Size;
    double TimeStamp;
    uint64_t Offset;
};

class H264VideoInfo : public VideoInfo
{
private:
    uint32_t              m_width;
    uint32_t              m_height;

    uint32_t              m_paddedWidth;
    uint32_t              m_paddedHeight;

    H264::SliceHeader*    m_sliceHeaders;
    H264::SPS*            m_sps;
    H264::PPS*            m_pps;

    uint32_t              m_sliceHeaderCount;
    uint32_t              m_spsCount;
    uint32_t              m_ppsCount;

    uint32_t              m_frameCount;
    H264VideoFrameInfo*   m_frames;

    float                 m_fps;
    double                m_duration;

protected:

public:
    H264VideoInfo(const MP4D_demux_t& a_demux, const MP4D_track_t& a_trackInfo, uint32_t a_trackIndex, FileHandle* a_handle);
    virtual ~H264VideoInfo();

    virtual bool IsValid() const
    {
        return m_frames != nullptr && m_sps != nullptr && m_pps != nullptr;
    }

    virtual e_VideoProfile GetVideoProfile() const
    {
        return VideoProfile_H264;
    }

    virtual float GetFPS() const
    {
        return m_fps;
    }
    virtual double GetDuraction() const
    {
        return m_duration;
    }

    virtual uint32_t GetWidth() const
    {
        return m_width;
    }
    virtual uint32_t GetHeight() const
    {
        return m_height;
    }

    inline uint32_t GetPaddedWidth() const
    {
        return m_paddedWidth;
    }
    inline uint32_t GetPaddedHeight() const
    {
        return m_paddedHeight;
    }

    inline const H264::SliceHeader* GetSliceHeaderData() const
    {
        return m_sliceHeaders;
    }
    inline uint32_t GetSliceHeaderCount() const
    {
        return m_sliceHeaderCount;
    }

    inline const H264VideoFrameInfo* GetFrames() const
    {
        return m_frames;
    }
    inline uint32_t GetFrameCount() const
    {
        return m_frameCount;
    }

    inline uint32_t GetSPSCount() const
    {
        return m_spsCount;
    }
    inline H264::SPS* GetSPSData() const
    {
        return m_sps;
    }

    inline uint32_t GetPPSCount() const
    {
        return m_ppsCount;
    }
    inline H264::PPS* GetPPSData() const
    {
        return m_pps;
    }

    virtual bool GetVideoClipData(FileHandle* a_handle, uint32_t a_startIndex, uint32_t a_endIndex, uint32_t a_alignment, uint8_t** a_data, uint32_t* a_size);
};

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