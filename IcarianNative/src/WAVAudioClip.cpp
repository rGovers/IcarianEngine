// Icarian Engine - C# Game Engine
// 
// License at end of file.

#include "Audio/AudioClips/WAVAudioClip.h"

#define GLM_FORCE_SWIZZLE 
#include <glm/glm.hpp>

#include "Core/IcarianError.h"
#include "DataTypes/Allocators/MallocAllocator.h"
#include "FileCache.h"
#include "IcarianError.h"

constexpr static uint32_t GetFormatSize(e_AudioFormat a_format)
{
    switch (a_format)
    {
    case AudioFormat_U8:
    {
        return sizeof(uint8_t);
    }
    case AudioFormat_S16:
    {
        return sizeof(int16_t);
    }
    }

    IERROR("Invalid format");

    return 0;
}

// Not accounting for endian not an issue currently but may become an issue down the line
WAVAudioClip::WAVAudioClip(const COWU8String& a_path, Allocator* a_allocator) :
    m_path(a_path, a_allocator)
{
    IERRBLOCK;

    m_sampleRate = 0;
    m_channelCount = 0;

    FileHandle* handle = FileCache::LoadFile(m_path);
    IERRCHECK(handle != nullptr);
    IDEFER(MallocAllocator::Instance->Destroy(handle));

    char buffer[16];
    if (handle->Read(buffer, 12) != 12)
    {
        return;
    }

    constexpr char RIFFStr[] = "RIFF";
    constexpr char WAVEStr[] = "WAVE";
    if (*(uint32_t*)RIFFStr != *(uint32_t*)buffer && *(uint32_t*)WAVEStr != *(uint32_t*)(buffer + 8))
    {
        return;
    }

    constexpr char FMTStr[] = "fmt ";
    constexpr char DATAStr[] = "data";

    const uint32_t fmtInt = *(uint32_t*)FMTStr;
    const uint32_t dataInt = *(uint32_t*)DATAStr;

    while (true)
    {
        handle->Read(buffer, 4);
        const uint32_t bufferInt = *(uint32_t*)buffer;

        handle->Read(buffer, 4);
        const uint32_t chunkSize = *(uint32_t*)buffer;

        if (chunkSize == 0)
        {
            IERROR("WAV invalid chunk size");
        }

        if (bufferInt == fmtInt)
        {
            // WAVE format
            handle->Read(buffer, 2);

            handle->Read(buffer, 2);
            m_channelCount = *(uint16_t*)buffer;

            handle->Read(buffer, 4);
            m_sampleRate = *(uint32_t*)buffer;

            // Byte rate
            handle->Read(buffer, 4);

            // Block align
            handle->Read(buffer, 2);

            handle->Read(buffer, 2);
            const uint16_t bitsPerSample = *(uint16_t*)buffer;

            switch (bitsPerSample)
            {
            case 8:
            {
                m_format = AudioFormat_U8;

                break;
            }
            case 16:
            {
                m_format = AudioFormat_S16;

                break;
            }
            default:
            {
                IERROR("Wav invalid bits per sample");
            }
            }

            // There is several forms of the fmt chunk and we do not care about the extended format
            const int32_t offset = chunkSize - 16;
            if (offset > 0)
            {
                handle->Ignore(offset);
            }
        }
        // Seems some applications take the spec as a guideline and not rules so just have to hope stumble across the fmt chunk before reaching the data chunk
        // Hopefully does not bite me in the ass continuing to follow the spec and expecting data last
        // Will deal with that later if it becomes an issue
        else if (bufferInt == dataInt)
        {
            m_dataOffset = handle->GetOffset();
            m_dataSize = chunkSize;

            if (m_dataSize == 0 || m_channelCount == 0 || m_sampleRate == 0)
            {
                IERROR("WAV invalid file");
            }

            break;
        }
        else
        {
            handle->Ignore(chunkSize);
        }
    }
}
WAVAudioClip::~WAVAudioClip()
{

}

float WAVAudioClip::GetDuration() const
{
    return (float)((double)GetSampleSize() / m_sampleRate);
}

uint32_t WAVAudioClip::GetSampleRate() const
{
    return m_sampleRate;
}
uint32_t WAVAudioClip::GetChannelCount() const
{
    return m_channelCount;
}
uint64_t WAVAudioClip::GetSampleSize() const
{
    return m_dataSize / m_channelCount / GetFormatSize(m_format);
}

e_AudioFormat WAVAudioClip::GetAudioFormat() const
{
    return m_format;
}

uint8_t* WAVAudioClip::GetAudioData(Allocator* a_allocator, uint64_t a_sampleOffset, uint32_t a_sampleSize, uint32_t* a_outSampleSize)
{
    IERRBLOCK;

    *a_outSampleSize = 0;

    FileHandle* handle = FileCache::LoadFile(m_path);
    IERRCHECKRET(handle != nullptr, nullptr);
    IDEFER(MallocAllocator::Instance->Destroy(handle));

    const uint32_t formatSize = GetFormatSize(m_format);

    const uint64_t seekOffset = a_sampleOffset * m_channelCount * formatSize;
    IERRCHECKRET(handle->Seek(m_dataOffset + seekOffset), nullptr);

    const uint64_t remainingSamples = GetSampleSize() - a_sampleOffset;
    const uint64_t samplesToRead = glm::min((uint64_t)a_sampleSize, remainingSamples);
    const uint64_t size = samplesToRead * m_channelCount;
    const uint64_t sampleSize = size * formatSize;

    uint8_t* data = a_allocator->TAllocate<uint8_t>(sampleSize);
    IERRDEFER(a_allocator->Free(data));
    IERRCHECKRET(handle->Read(data, sampleSize) == sampleSize, nullptr);

    *a_outSampleSize = (uint32_t)samplesToRead;

    return data;
}

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
