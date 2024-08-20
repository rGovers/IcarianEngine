// Icarian Engine - C# Game Engine
// 
// License at end of file.

#include "Audio/AudioEngine.h"

#include "Audio/AudioClips/AudioClip.h"
#include "Audio/AudioEngineBindings.h"
#include "Core/Bitfield.h"
#include "Core/IcarianDefer.h"
#include "IcarianError.h"
#include "Profiler.h"
#include "Trace.h"

static AudioEngine* Instance = nullptr;

static void OutCallback(ma_device* a_device, void* a_output, const void* a_input, ma_uint32 a_frameCount)
{
    Instance->AudioOutCallback(a_device, a_output, a_frameCount);
}

AudioEngine::AudioEngine() :
    // Should allocate 1MB for the Ring Allocator hopefully it is enough otherwise may need to revisit
    // Do not want to allocate in audio callbacks due to latency
    m_allocator(1 << 20)
{
    TRACE("Creating AudioEngine...");
    Instance = this;

    m_bindings = new AudioEngineBindings(this);   

    ma_device_config deviceConfig = ma_device_config_init(ma_device_type_playback);
    deviceConfig.playback.format = ma_format_s16;
    deviceConfig.playback.channels = 2;
    deviceConfig.sampleRate = SampleRate;
    deviceConfig.dataCallback = OutCallback;

    if (ma_device_init(NULL, &deviceConfig, &m_device) != MA_SUCCESS)
    {
        goto Error;
    }

    ma_device_start(&m_device);

    m_init = true;

    return;

Error:;
    Logger::Error("Failed to initialise audio engine");

    m_init = false;
}
AudioEngine::~AudioEngine()
{
    delete m_bindings;

    TRACE("Destroying AudioEngine...");

    if (!m_init)
    {
        ma_device_stop(&m_device);
    }

    for (uint32_t i = 0; i < m_audioClips.Size(); ++i)
    {
        if (m_audioClips.Exists(i))
        {
            IWARN("AudioClip was not destroyed.");

            delete m_audioClips[i];
        }
    }

    for (uint32_t i = 0; i < m_audioSources.Size(); ++i)
    {
        if (m_audioSources.Exists(i))
        {
            IWARN("AudioSource was not destroyed.");
        }
    }

    for (uint32_t i = 0; i < m_audioListeners.Size(); ++i)
    {
        if (m_audioListeners.Exists(i))
        {
            IWARN("AudioListener was not destroyed.");
        }
    }

    for (uint32_t i = 0; i < m_audioMixers.Size(); ++i)
    {
        if (m_audioMixers.Exists(i))
        {
            IWARN("AudioMixer was not destroyed.");
        }
    }

    if (!m_init)
    {
        ma_device_uninit(&m_device);
    }
}

void AudioEngine::AudioOutCallback(ma_device* a_device, void* a_output, ma_uint32 a_frameCount)
{
    memset(a_output, 0, a_frameCount * 2 * sizeof(int16_t));

    int16_t* out = (int16_t*)a_output;

    const Array<bool> state = m_audioSources.ToStateArray();
    TLockArray<AudioSourceBuffer> sources = m_audioSources.ToLockArray();
    const uint32_t size = state.Size();

    printf("?\n");

    // TODO: Doing this in the callback as a quick and dirty to see if it works down the line probably want to create a swap buffer and read from the swap buffer in the callback
    // Can probably dispatch the audio listeners onto the thread pool that way need to look into it however as I have to merge to passes from the thread pool

    for (uint32_t i = 0; i < size; ++i)
    {
        if (!state[i])
        {
            continue;
        }

        AudioSourceBuffer& buffer = sources[i];

        if (IISBITSET(buffer.Flags, AudioSourceBuffer::PlayBitOffset))
        {
            buffer.SampleOffset = 0;

            ICLEARBIT(buffer.Flags, AudioSourceBuffer::PlayBitOffset);
            ISETBIT(buffer.Flags, AudioSourceBuffer::PlayingBitOffset);
        }

        if (!IISBITSET(buffer.Flags, AudioSourceBuffer::PlayingBitOffset))
        {
            continue;
        }

        if (buffer.AudioClipAddr == -1)
        {
            ICLEARBIT(buffer.Flags, AudioSourceBuffer::PlayingBitOffset);

            Logger::Warning("Playing audio source with no audio clip");

            continue;
        }

        IVERIFY(buffer.AudioClipAddr < m_audioClips.Size());
        IVERIFY(m_audioClips.Exists(buffer.AudioClipAddr));

        AudioClip* clip = m_audioClips[buffer.AudioClipAddr];
        const uint32_t sampleRate = clip->GetSampleRate();
        const uint32_t channelCount = clip->GetChannelCount();
        const float invChannel = 1.0f / channelCount;

        switch (clip->GetAudioFormat())
        {
        case AudioFormat_U8:
        {
            break;
        }
        case AudioFormat_S16:
        {
            printf("c \n");
            const bool sampleMatches = sampleRate == SampleRate;
            if (IISBITSET(buffer.Flags, AudioSourceBuffer::LoopBitOffset))
            {
                uint32_t readSamples = 0;
                while (true)
                {
                    uint32_t samples;
                    const uint8_t* dat = clip->GetAudioData(&m_allocator, buffer.SampleOffset, a_frameCount - readSamples, &samples);
                    const int16_t* readDat = (int16_t*)dat;

                    for (uint32_t i = 0; i < samples; ++i)
                    {
                        for (uint32_t j = 0; j < channelCount; ++j)
                        {
                            const int16_t val = (int16_t)(readDat[i * channelCount + j] * invChannel);

                            for (uint32_t k = 0; k < 2; ++k)
                            {
                                out[(i + readSamples) * 2 + k] += val;
                            }
                        }
                    }

                    if (readSamples >= a_frameCount)
                    {
                        buffer.SampleOffset += samples;

                        break;
                    }

                    buffer.SampleOffset = 0;
                }
            }
            else
            {
                uint32_t samples;
                const uint8_t* dat = clip->GetAudioData(&m_allocator, buffer.SampleOffset, a_frameCount, &samples);
                const int16_t* readDat = (int16_t*)dat;

                for (uint32_t i = 0; i < samples; ++i)
                {
                    for (uint32_t j = 0; j < channelCount; ++j)
                    {
                        const int16_t val = (int16_t)(readDat[i * channelCount + j] * invChannel);

                        for (uint32_t k = 0; k < 2; ++k)
                        {
                            out[i * 2 + k] += val;
                        }
                    }
                }

                buffer.SampleOffset += samples;

                if (samples < a_frameCount)
                {
                    ICLEARBIT(buffer.Flags, AudioSourceBuffer::PlayingBitOffset);
                }
            }

            break;
        }
        }
    }
}

void AudioEngine::Update()
{
    if (!m_init)
    {
        return;
    }

    // {
    //     PROFILESTACK("Audio Sources");
    //
    //     const Array<bool> state = m_audioSources.ToStateArray();
    //     TLockArray<AudioSourceBuffer> a = m_audioSources.ToLockArray();
    //     const uint32_t size = state.Size();
    //
    //     for (uint32_t i = 0; i < size; ++i)
    //     {
    //         const bool exists = state[i];
    //         if (!exists)
    //         {
    //             continue;
    //         }
    //
    //         AudioSourceBuffer& buffer = a[i];
    //         if (IISBITSET(buffer.Flags, AudioSourceBuffer::PlayBitOffset))
    //         {
    //
    //
    //             ICLEARBIT(buffer.Flags, AudioSourceBuffer::PlayBitOffset);
    //             ISETBIT(buffer.Flags, AudioSourceBuffer::PlayingBitOffset);
    //         }
    //     }
    // }
}

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
