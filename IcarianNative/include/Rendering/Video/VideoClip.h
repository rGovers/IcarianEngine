// Icarian Engine - C# Game Engine
// 
// License at end of file.

#pragma once

#include <filesystem>

class VideoInfo;

enum e_VideoUpdateMode
{
    VideoUpdateMode_Audio,
    VideoUpdateMode_Video
};

class VideoClip
{
private:
    std::filesystem::path m_path;

    VideoInfo*            m_videoInfo;

    e_VideoUpdateMode     m_updateMode;

    double                m_time;

protected:

public:
    VideoClip(const std::filesystem::path& a_path);
    ~VideoClip();

    inline bool IsValid() const
    {
        return m_videoInfo != nullptr;
    }

    inline e_VideoUpdateMode GetUpdateMode() const
    {
        return m_updateMode;
    }

    inline const VideoInfo* GetVideoInfo() const
    {
        return m_videoInfo;
    }

    inline double GetTime() const
    {
        return m_time;
    }
    inline void SetTime(double a_time)
    {
        m_time = a_time;
    }

    bool GetVideoClipData(uint32_t a_startIndex, uint32_t a_endIndex, uint32_t a_alignment, uint8_t** a_data, uint32_t* a_size) const;
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