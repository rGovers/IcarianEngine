#pragma once

#include <cstdint>

enum e_AudioFormat : uint16_t
{
    AudioFormat_S16
};

class AudioClip
{
private:

protected:

public:
    virtual ~AudioClip() = default;

    virtual float GetDuration() const = 0;

    virtual uint32_t GetSampleRate() const = 0;
    virtual uint32_t GetChannelCount() const = 0;
    virtual uint64_t GetSampleSize() const = 0;

    virtual e_AudioFormat GetAudioFormat() const
    {
        return AudioFormat_S16;
    }

    virtual unsigned char* GetAudioData(uint64_t a_sampleOffset, uint32_t a_sampleSize, uint32_t* a_outSampleSize) = 0;
};