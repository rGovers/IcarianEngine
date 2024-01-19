#pragma once

#include "Audio/AudioClips/AudioClip.h"

#include <filesystem>

#define STB_VORBIS_HEADER_ONLY
#include <stb_vorbis.c>

class OGGAudioClip : public AudioClip
{
private:
    std::filesystem::path m_path;

    stb_vorbis*           m_stream;
    stb_vorbis_info       m_info;

    uint64_t              m_sampleSize;
    float                 m_duration;

protected:

public:
    OGGAudioClip(const std::filesystem::path& a_path);
    virtual ~OGGAudioClip();

    virtual float GetDuration() const;

    virtual uint32_t GetSampleRate() const;
    virtual uint32_t GetChannelCount() const;
    virtual uint64_t GetSampleSize() const;

    virtual unsigned char* GetAudioData(RingAllocator* a_allocator, uint64_t a_sampleOffset, uint32_t a_sampleSize, uint32_t* a_outSampleSize);
};