// Icarian Engine - C# Game Engine
// 
// License at end of file.

#pragma once

#include "Audio/AudioClips/AudioClip.h"

#include <filesystem>

class WAVAudioClip : public AudioClip
{
private:
    std::filesystem::path m_path;

    uint64_t              m_dataOffset;
    uint64_t              m_dataSize;

    uint32_t              m_sampleRate;
    uint32_t              m_channelCount;

    e_AudioFormat         m_format;

protected:

public:
    WAVAudioClip(const std::filesystem::path& a_path);
    virtual ~WAVAudioClip();

    virtual float GetDuration() const;

    virtual uint32_t GetSampleRate() const;
    virtual uint32_t GetChannelCount() const;
    virtual uint64_t GetSampleSize() const;

    virtual e_AudioFormat GetAudioFormat() const;

    virtual uint8_t* GetAudioData(RingAllocator* a_allocator, uint64_t a_sampleOffset, uint32_t a_sampleSize, uint32_t* a_outSampleSize);
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