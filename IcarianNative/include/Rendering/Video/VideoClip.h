#pragma once

#include <filesystem>

#include "DataTypes/Array.h"
#include "Rendering/Video/H264.h"

enum e_VideoProfile
{
    VideoProfile_Null,
    VideoProfile_H264,
};

enum e_VideoFrameType
{
    VideoFrameType_Intra,
    VideoFrameType_Predictive
};

struct VideoFrameInfo
{
    e_VideoFrameType Type;
    union
    {
        uint64_t Priority;

        struct 
        {
            uint32_t POC;
            uint32_t GOP;
        };
    } PrioData;
    uint32_t Size;
    double TimeStamp;
    uint64_t Offset;
};

class VideoClip
{
private:
    std::filesystem::path m_path;

    e_VideoProfile        m_videoProfile;

    uint32_t              m_width;
    uint32_t              m_height;

    uint32_t              m_paddedWidth;
    uint32_t              m_paddedHeight;

    Array<H264::SPS>      m_sps;
    Array<H264::PPS>      m_pps;

    uint32_t              m_frameCount;
    VideoFrameInfo*       m_frames;

    double                m_duration;
    float                 m_fps;

protected:

public:
    VideoClip(const std::filesystem::path& a_path);
    ~VideoClip();

    inline e_VideoProfile GetVideoProfile() const
    {
        return m_videoProfile;
    }

    inline uint32_t GetFrameCount() const
    {
        return m_frameCount;
    }
    inline const VideoFrameInfo* GetFrameInfo() const
    {
        return m_frames;
    }

    inline uint32_t GetWidth() const
    {
        return m_width;
    }
    inline uint32_t GetHeight() const
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

    inline double GetDuration() const
    {
        return m_duration;
    }
    inline float GetFPS() const
    {
        return m_fps;
    }

    inline const H264::PPS* GetPPSData() const
    {
        return m_pps.Data();
    }
    inline uint32_t GetPPSCount() const
    {
        return m_pps.Size();
    }

    inline const H264::SPS* GetSPSData() const
    {
        return m_sps.Data();
    }
    inline uint32_t GetSPSCount() const
    {
        return m_sps.Size();
    }

    bool GetVideoClipData(double a_inTimeStamp, uint32_t* a_startIndex, uint32_t* a_endIndex, uint8_t** a_data, uint32_t* a_size) const;
};
