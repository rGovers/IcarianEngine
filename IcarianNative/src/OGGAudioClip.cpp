#include "Audio/AudioClips/OGGAudioClip.h"

#include "DataTypes/RingAllocator.h"

OGGAudioClip::OGGAudioClip(const std::filesystem::path& a_path) : AudioClip()
{
    m_path = a_path;

    m_stream = stb_vorbis_open_filename(m_path.string().c_str(), NULL, NULL);
    m_info = stb_vorbis_get_info(m_stream);

    // I do not know how or why but get the sample size before doing anything else or else it will break
    // I have no idea why this is the case
    // This engine is held together by duct tape, glue and a lot of hope
    m_sampleSize = (uint64_t)stb_vorbis_stream_length_in_samples(m_stream);
    // I have to calculate the duration from the sample size and sample rate because reasons
    m_duration = m_sampleSize / (float)m_info.sample_rate;
}
OGGAudioClip::~OGGAudioClip()
{
    stb_vorbis_close(m_stream);
}

float OGGAudioClip::GetDuration() const
{
    return m_duration;
}

uint32_t OGGAudioClip::GetSampleRate() const
{
    return (uint32_t)m_info.sample_rate;
}
uint32_t OGGAudioClip::GetChannelCount() const
{
    return (uint32_t)m_info.channels;
}
uint64_t OGGAudioClip::GetSampleSize() const
{
    return m_sampleSize * m_info.channels;
}

uint8_t* OGGAudioClip::GetAudioData(RingAllocator* a_allocator, uint64_t a_sampleOffset, uint32_t a_sampleSize, uint32_t* a_outSampleSize)
{
    uint8_t* buffer = (uint8_t*)a_allocator->Allocate<int16_t>(a_sampleSize * m_info.channels);

    stb_vorbis_seek(m_stream, (int)a_sampleOffset);

    *a_outSampleSize = (uint32_t)stb_vorbis_get_samples_short_interleaved(m_stream, m_info.channels, (short*)buffer, a_sampleSize * m_info.channels);

    return buffer;
}