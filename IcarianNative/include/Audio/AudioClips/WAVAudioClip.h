#pragma once

#include "Audio/AudioClips/AudioClip.h"

#include <filesystem>

class WAVAudioClip : public AudioClip
{
private:
    std::filesystem::path m_path;

    uint32_t              m_sampleRate;
    uint32_t              m_channelCount;
    uint64_t              m_sampleSize;
    float                 m_duration;

    uint64_t              m_dataOffset;
    uint64_t              m_dataSize;

protected:

public:
    WAVAudioClip(const std::filesystem::path& a_path);
    virtual ~WAVAudioClip();

    virtual float GetDuration() const;

    virtual uint32_t GetSampleRate() const;
    virtual uint32_t GetChannelCount() const;
    virtual uint64_t GetSampleSize() const;

    virtual unsigned char* GetAudioData(uint64_t a_sampleOffset, uint32_t a_sampleSize, uint32_t* a_outSampleSize);
};