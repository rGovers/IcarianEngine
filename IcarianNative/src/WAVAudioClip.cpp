#include "Audio/AudioClips/WAVAudioClip.h"

#define GLM_FORCE_SWIZZLE 
#include <glm/glm.hpp>

#include "DataTypes/RingAllocator.h"
#include "FileCache.h"
#include "IcarianError.h"

// Not accounting for endian not an issue currently but may become an issue down the line
WAVAudioClip::WAVAudioClip(const std::filesystem::path& a_path)
{
    m_path = a_path;

    m_duration = 0.0f;
    m_sampleRate = 0;
    m_channelCount = 0;
    m_sampleSize = 0;

    FileHandle* handle = FileCache::LoadFile(m_path);
    if (handle != nullptr)
    {
        IDEFER(delete handle);

        char buffer[128];
        handle->Read(buffer, 12);

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

                // Bits per sample
                handle->Read(buffer, 2);

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

                m_sampleSize = m_dataSize / m_channelCount / sizeof(int16_t);
                m_duration = (float)m_sampleSize / (float)m_sampleRate;

                break;
            }
            else
            {
                handle->Ignore(chunkSize);
            }
        }        
    }
}
WAVAudioClip::~WAVAudioClip()
{

}

float WAVAudioClip::GetDuration() const
{
    return m_duration;
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
    return m_sampleSize;
}

unsigned char* WAVAudioClip::GetAudioData(RingAllocator* a_allocator, uint64_t a_sampleOffset, uint32_t a_sampleSize, uint32_t* a_outSampleSize)
{
    const uint64_t remainingSamples = m_sampleSize - a_sampleOffset;
    const uint64_t samplesToRead = glm::min((uint64_t)a_sampleSize, remainingSamples);

    *a_outSampleSize = (uint32_t)samplesToRead;

    FileHandle* handle = FileCache::LoadFile(m_path);
    if (handle != nullptr)
    {
        IDEFER(delete handle);
        
        handle->Seek(m_dataOffset + (a_sampleOffset * m_channelCount * sizeof(int16_t)));

        const uint64_t count = samplesToRead * m_channelCount;
        // int16_t* data = new int16_t[count];
        int16_t* data = a_allocator->Allocate<int16_t>(count);
        handle->Read(data, count * sizeof(int16_t));

        return (unsigned char*)data;
    }

    return nullptr;
}