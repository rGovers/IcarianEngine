#include "Audio/AudioEngine.h"

#include <cstddef>
#include <string.h>

#include "Audio/AudioClips/AudioClip.h"
#include "Audio/AudioEngineBindings.h"
#include "Flare/IcarianDefer.h"
#include "Logger.h"
#include "Trace.h"

AudioEngine::AudioEngine()
{
    m_bindings = new AudioEngineBindings(this);   
    
    TRACE("Creating AudioEngine...");
    m_device = alcOpenDevice(NULL);
    if (m_device == NULL)
    {
        Logger::Warning("Failed to create AudioEngine: No default audio device.");

        return;
    }

    m_context = alcCreateContext(m_device, NULL);
    if (m_context == NULL)
    {
        alcCloseDevice(m_device);
        m_device = NULL;

        Logger::Warning("Failed to create AudioEngine: No default audio context.");

        return;
    }

    if (!alcMakeContextCurrent(m_context))
    {
        alcDestroyContext(m_context);
        alcCloseDevice(m_device);

        m_device = NULL;
        m_context = NULL;

        Logger::Warning("Failed to create AudioEngine: Failed to make context current.");

        return;
    }
}
AudioEngine::~AudioEngine()
{
    delete m_bindings;

    for (uint32_t i = 0; i < m_audioClips.Size(); ++i)
    {
        if (m_audioClips.Exists(i))
        {
            Logger::Warning("AudioClip was not destroyed.");

            delete m_audioClips[i];
        }
    }

    for (uint32_t i = 0; i < m_audioSources.Size(); ++i)
    {
        if (m_audioSources.Exists(i))
        {
            Logger::Warning("AudioSource was not destroyed.");

            if (m_audioSources[i].Source != -1)
            {
                alSourceStop(m_audioSources[i].Source);
                alDeleteSources(1, &m_audioSources[i].Source);
            }
            if (m_audioSources[i].Buffers[0] != -1)
            {
                alDeleteBuffers(AudioSourceBuffer::BufferCount, m_audioSources[i].Buffers);
            }
        }
    }

    TRACE("Destroying AudioEngine...");
    alcMakeContextCurrent(NULL);
    alcDestroyContext(m_context);
    alcCloseDevice(m_device);
}

static uint64_t FillBuffers(ALuint* a_buffer, uint32_t a_bufferCount, AudioClip* a_clip, uint64_t a_sampleOffset, uint32_t a_sampleSize, bool a_canLoop)
{
    uint32_t outSampleSize;
    const uint32_t channelCount = a_clip->GetChannelCount();  
    const uint32_t sampleRate = a_clip->GetSampleRate();
    const e_AudioFormat format = a_clip->GetAudioFormat();

    const uint64_t maxSampleOffset = a_clip->GetSampleSize();

    for (uint32_t i = 0; i < a_bufferCount; ++i)
    {
        unsigned char* data = a_clip->GetAudioData(a_sampleOffset, a_sampleSize, &outSampleSize);
        IDEFER(delete[] data);
        
        // Unlikely but if the sample size is less than the requested sample size, we need to loop.
        // Not the most efficient way to do this but it works. KISS, right?
        while (outSampleSize < a_sampleSize)
        {
            if (a_canLoop)
            {
                a_sampleOffset = 0;

                uint32_t nextOutSampleSize;
                const unsigned char* oldData = data;
                const unsigned char* nextData = a_clip->GetAudioData(0, a_sampleSize - outSampleSize, &nextOutSampleSize);
                IDEFER(delete[] nextData);
                IDEFER(delete[] oldData);

                unsigned char* newData = new unsigned char[(outSampleSize + nextOutSampleSize) * channelCount * sizeof(int16_t)];

                switch (format)
                {
                case AudioFormat_S16:
                {
                    // Should be the same but still want to run well in DEBUG mode so using memcpy.
                    const uint32_t count = outSampleSize * channelCount * sizeof(int16_t);
                    memcpy(newData, data, count);

                    const uint32_t nextCount = nextOutSampleSize * channelCount * sizeof(int16_t);
                    memcpy(newData + count, nextData, nextCount);

                    break;
                }
                }

                data = newData;
                outSampleSize += nextOutSampleSize;
            }
            else
            {
                break;
            }       
        }

        switch (format)
        {
        case AudioFormat_S16:
        {
            switch (channelCount)
            {
            case 1:
            {
                alBufferData(a_buffer[i], AL_FORMAT_MONO16, data, outSampleSize * channelCount * sizeof(int16_t), sampleRate);

                break;
            }
            case 2:
            {
                alBufferData(a_buffer[i], AL_FORMAT_STEREO16, data, outSampleSize * channelCount * sizeof(int16_t), sampleRate);

                break;
            }
            }

            break;
        }
        }

        // a_sampleOffset += outSampleSize;
        a_sampleOffset = (a_sampleOffset + outSampleSize) % maxSampleOffset;
        // a_sampleOffset = (a_sampleOffset + a_sampleSize) % maxSampleOffset;
    }

    return a_sampleOffset;
}

void AudioEngine::Update()
{
    std::vector<bool> state = m_audioSources.ToStateVector();
    TLockArray<AudioSourceBuffer> buffers = m_audioSources.ToLockArray();

    const uint32_t bufferCount = (uint32_t)state.size();
    // for (AudioSourceBuffer& buffer : buffers)
    for (uint32_t i = 0; i < bufferCount; ++i)
    {
        if (!state[i])
        {
            continue;
        }

        AudioSourceBuffer& buffer = buffers[i];

        const bool canLoop = buffer.Flags & 0b1 << AudioSourceBuffer::LoopBitOffset;

        if (buffer.Flags & 0b1 << AudioSourceBuffer::PlayBitOffset)
        {
            AudioClip* clip = m_audioClips[buffer.AudioClipAddr];

            if (buffer.Buffers[0] == -1)
            {
                alGenBuffers(AudioSourceBuffer::BufferCount, buffer.Buffers);                
            }
            
            if (buffer.Source == -1)
            {
                alGenSources(1, &buffer.Source);
            }
            else
            {
                alSourceStop(buffer.Source);
            }

            buffer.SampleOffset = FillBuffers(buffer.Buffers, AudioSourceBuffer::BufferCount, clip, 0, AudioBufferSampleSize, canLoop);

            alSourceQueueBuffers(buffer.Source, AudioSourceBuffer::BufferCount, buffer.Buffers);

            alSourcePlay(buffer.Source);

            buffer.Flags &= ~(0b1 << AudioSourceBuffer::PlayBitOffset);
            buffer.Flags |= (0b1 << AudioSourceBuffer::PlayingBitOffset);
        }
        else if (buffer.Flags & 0b1 << AudioSourceBuffer::PlayingBitOffset)
        {
            ALint processed;
            alGetSourcei(buffer.Source, AL_BUFFERS_PROCESSED, &processed);

            if (processed > 0)
            {
                ALuint queueBuffer;
                alSourceUnqueueBuffers(buffer.Source, 1, &queueBuffer);

                AudioClip* clip = m_audioClips[buffer.AudioClipAddr];

                buffer.SampleOffset = FillBuffers(&queueBuffer, 1, clip, buffer.SampleOffset, AudioBufferSampleSize, canLoop);

                alSourceQueueBuffers(buffer.Source, 1, &queueBuffer);
            }

            if (!canLoop)
            {
                ALint state;
                alGetSourcei(buffer.Source, AL_SOURCE_STATE, &state);

                if (state == AL_STOPPED)
                {
                    buffer.Flags &= ~(0b1 << AudioSourceBuffer::PlayingBitOffset);
                }
            }
        }
    }
}