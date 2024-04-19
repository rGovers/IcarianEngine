#include "Audio/AudioEngine.h"

#include <cstddef>
#include <string.h>

#include "Audio/AudioClips/AudioClip.h"
#include "Audio/AudioEngineBindings.h"
#include "DataTypes/RingAllocator.h"
#include "Logger.h"
#include "ObjectManager.h"
#include "Profiler.h"
#include "Trace.h"

static constexpr char ALCString[] = "ALC";
static constexpr uint32_t ALCStringLength = sizeof(ALCString) - 1;

static bool IsExtensionSupported(const std::string_view& a_extension)
{
    if (strncmp(a_extension.data(), ALCString, ALCStringLength) == 0)
    {
        return (bool)alcIsExtensionPresent(NULL, a_extension.data());
    }

    return (bool)alIsExtensionPresent(a_extension.data());
}

AudioEngine::AudioEngine()
{
    m_bindings = new AudioEngineBindings(this);   

    TRACE("Creating AudioEngine...");
    m_device = alcOpenDevice(NULL);
    if (m_device == NULL)
    {
        Logger::Warning("Failed to create AudioEngine: No audio device.");

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

    // Flush Errors
    alGetError();
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

    for (uint32_t i = 0; i < m_audioListeners.Size(); ++i)
    {
        if (m_audioListeners.Exists(i))
        {
            Logger::Warning("AudioListener was not destroyed.");
        }
    }

    for (uint32_t i = 0; i < m_audioMixers.Size(); ++i)
    {
        if (m_audioMixers.Exists(i))
        {
            Logger::Warning("AudioMixer was not destroyed.");
        }
    }

    TRACE("Destroying AudioEngine...");
    alcMakeContextCurrent(NULL);
    if (m_context != NULL)
    {
        alcDestroyContext(m_context);
    }
    if (m_device != NULL)
    {
        alcCloseDevice(m_device);
    }
}

static constexpr ALenum GetALFormat(e_AudioFormat a_format, uint32_t a_channelCount)
{
    switch (a_format)
    {
    case AudioFormat_U8:
    {
        switch (a_channelCount)
        {
        case 1:
        {
            return AL_FORMAT_MONO8;
        }
        case 2:
        {
            return AL_FORMAT_STEREO8;
        }
        }
    }
    case AudioFormat_S16:
    {
        switch (a_channelCount)
        {
        case 1:
        {
            return AL_FORMAT_MONO16;
        }
        case 2:
        {
            return AL_FORMAT_STEREO16;
        }
        }
    }
    }

    return AL_FORMAT_MONO16;
}
static constexpr uint64_t GetAudioSize(e_AudioFormat a_format, uint32_t a_channelCount, uint32_t a_samples)
{
    switch (a_format)
    {
    case AudioFormat_U8:
    {
        return a_samples * a_channelCount * sizeof(uint8_t);
    }
    case AudioFormat_S16:
    {
        return a_samples * a_channelCount * sizeof(int16_t);
    }
    }

    return 0;
}

static uint64_t FillBuffers(RingAllocator* a_allocator, ALuint* a_buffer, uint32_t a_bufferCount, AudioClip* a_clip, uint64_t a_sampleOffset, uint32_t a_sampleSize, bool a_canLoop)
{
    uint32_t outSampleSize;
    const uint32_t channelCount = a_clip->GetChannelCount();  
    const uint32_t sampleRate = a_clip->GetSampleRate();
    const e_AudioFormat format = a_clip->GetAudioFormat();

    const uint64_t maxSampleOffset = a_clip->GetSampleSize();

    for (uint32_t i = 0; i < a_bufferCount; ++i)
    {
        // Do not need to de-allocate this as it is managed by the ring allocator.
        unsigned char* data = a_clip->GetAudioData(a_allocator, a_sampleOffset, a_sampleSize, &outSampleSize);
        if (data == nullptr)
        {
            break;
        }
        
        // Unlikely but if the sample size is less than the requested sample size, we need to loop.
        // Not the most efficient way to do this but it works. KISS, right?
        while (outSampleSize < a_sampleSize)
        {
            if (a_canLoop)
            {
                a_sampleOffset = 0;

                uint32_t nextOutSampleSize;
                const unsigned char* oldData = data;
                // Do not need to de-allocate this as it is managed by the ring allocator.
                const unsigned char* nextData = a_clip->GetAudioData(a_allocator, 0, a_sampleSize - outSampleSize, &nextOutSampleSize);
                if (nextData == nullptr)
                {
                    break;
                }

                // Again no need to de-allocate as it is managed by the ring allocator.
                unsigned char* newData = (unsigned char*)a_allocator->Allocate<int16_t>((outSampleSize + nextOutSampleSize) * channelCount);

                const uint32_t count = GetAudioSize(format, channelCount, outSampleSize);
                memcpy(newData, data, count);

                const uint32_t nextCount = GetAudioSize(format, channelCount, nextOutSampleSize);
                memcpy(newData + count, nextData, nextCount);

                data = newData;
                outSampleSize += nextOutSampleSize;
            }
            else
            {
                break;
            }       
        }

        alBufferData(a_buffer[i], GetALFormat(format, channelCount), data, GetAudioSize(format, channelCount, outSampleSize), sampleRate);

        a_sampleOffset = (a_sampleOffset + outSampleSize) % maxSampleOffset;
    }

    return a_sampleOffset;
}

static void SetSourceTransform(const AudioSourceBuffer& a_source)
{
    const glm::mat4 transform = ObjectManager::GetGlobalMatrix(a_source.TransformAddr);

    const glm::vec3 position = transform[3].xyz();
    alSource3f(a_source.Source, AL_POSITION, position.x, position.y, position.z);

    const glm::vec3 forward = transform[2].xyz();
    const glm::vec3 up = -transform[1].xyz();

    const ALfloat orientation[6] = 
    {
        forward.x, forward.y, forward.z,
        up.x,      up.y,      up.z
    };

    alSourcefv(a_source.Source, AL_ORIENTATION, orientation);
}

void AudioEngine::Update()
{
    if (m_device == NULL || m_context == NULL)
    {
        return;
    }

    const ALenum error = alGetError();
    if (error != AL_NO_ERROR)
    {
        Logger::Error(std::string("OpenAL Error: ") + alGetString(error));

        return;
    }

    {
        PROFILESTACK("Listener Update");

        const std::vector<bool> listenerState = m_audioListeners.ToStateVector();
        TLockArray<AudioListenerBuffer> listenerBuffers = m_audioListeners.ToLockArray();

        const uint32_t listenerBufferCount = (uint32_t)listenerState.size();
        for (uint32_t i = 0; i < listenerBufferCount; ++i)
        {
            if (!listenerState[i])
            {
                continue;
            }

            const AudioListenerBuffer& buffer = listenerBuffers[i];
            
            const glm::mat4 transform = ObjectManager::GetGlobalMatrix(buffer.TransformAddr);

            const glm::vec3 position = transform[3].xyz();

            alListener3f(AL_POSITION, position.x, position.y, position.z);

            const glm::vec3 forward = transform[2].xyz();
            const glm::vec3 up = -transform[1].xyz();

            const ALfloat orientation[6] = { forward.x, forward.y, forward.z, up.x, up.y, up.z };

            alListenerfv(AL_ORIENTATION, orientation);

            // To my knowledge, OpenAL only supports one listener so we can break here.
            break;
        }
    }

    {
        PROFILESTACK("Source Update");
        
        // 128KB should be enough for anyone.
        // Size is an educated guess based on the sample rate and sample size with several channels.
        // Has not caused any issues so far but may need to be adjusted if overruns occur.
        // Was looking for an excuse to write a ring allocator.
        // Just Ye' Ol' if allocation is too expensive just dont. It is that simple.
        RingAllocator allocator = RingAllocator(1024 * 128);

        const std::vector<bool> sourceState = m_audioSources.ToStateVector();
        TLockArray<AudioSourceBuffer> sourceBuffers = m_audioSources.ToLockArray();

        const uint32_t sourceBufferCount = (uint32_t)sourceState.size();
        for (uint32_t i = 0; i < sourceBufferCount; ++i)
        {
            if (!sourceState[i])
            {
                continue;
            }

            AudioSourceBuffer& buffer = sourceBuffers[i];

            const bool canLoop = buffer.Flags & 0b1 << AudioSourceBuffer::LoopBitOffset;

            if (buffer.Flags & 0b1 << AudioSourceBuffer::PlayBitOffset)
            {
                AudioClip* clip = m_audioClips[buffer.AudioClipAddr];

                if (clip->GetChannelCount() == 0)
                {
                    continue;
                }

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

                buffer.SampleOffset = FillBuffers(&allocator, buffer.Buffers, AudioSourceBuffer::BufferCount, clip, 0, AudioBufferSampleSize, canLoop);

                alSourceQueueBuffers(buffer.Source, AudioSourceBuffer::BufferCount, buffer.Buffers);

                SetSourceTransform(buffer);

                if (buffer.AudioMixerAddr != -1)
                {
                    const AudioMixerBuffer& mixer = m_audioMixers[buffer.AudioMixerAddr];

                    alSourcef(buffer.Source, AL_GAIN, mixer.Gain);
                }
                else
                {
                    alSourcef(buffer.Source, AL_GAIN, 1.0f);
                }

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

                    buffer.SampleOffset = FillBuffers(&allocator, &queueBuffer, 1, clip, buffer.SampleOffset, AudioBufferSampleSize, canLoop);

                    alSourceQueueBuffers(buffer.Source, 1, &queueBuffer);
                }

                SetSourceTransform(buffer);

                if (buffer.AudioMixerAddr != -1)
                {
                    const AudioMixerBuffer& mixer = m_audioMixers[buffer.AudioMixerAddr];

                    alSourcef(buffer.Source, AL_GAIN, mixer.Gain);
                }
                else
                {
                    alSourcef(buffer.Source, AL_GAIN, 1.0f);
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
}