// Icarian Engine - C# Game Engine
// 
// License at end of file.

#pragma once

#include <cstdint>

class FileHandle;

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

class VideoInfo
{
private:

protected:

public:
    virtual ~VideoInfo() { }

    virtual bool IsValid() const
    {
        return false;
    }

    virtual e_VideoProfile GetVideoProfile() const
    {
        return VideoProfile_Null;
    }

    virtual float GetFPS() const
    {
        return 0;
    }
    virtual double GetDuration() const
    {
        return 0;
    }

    virtual uint32_t GetWidth() const
    {
        return -1;
    }
    virtual uint32_t GetHeight() const
    {
        return -1;
    }

    virtual bool GetVideoClipData(FileHandle* a_handle, double a_inTimeStamp, uint32_t* a_startIndex, uint32_t* a_endIndex, uint8_t** a_data, uint32_t* a_size)
    {
        return false;
    }
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