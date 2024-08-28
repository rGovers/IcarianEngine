// Icarian Engine - C# Game Engine
// 
// License at end of file.

#include "Audio/AudioEngine.h"

#include "Audio/AudioClips/AudioClip.h"
#include "Audio/AudioEngineBindings.h"
#include "Core/Bitfield.h"
#include "Core/IcarianError.h"
#include "IcarianError.h"
#include "Logger.h"
#include "ObjectManager.h"
#include "Profiler.h"
#include "Trace.h"

static AudioEngine* Instance = nullptr;

AudioEngine::AudioEngine() :
    // Should allocate 1MB for the Ring Allocator hopefully it is enough otherwise may need to revisit
    // Do not want to allocate in audio callbacks due to latency
    m_allocator(1 << 20)
{
    IERRBLOCK;

    IERRDEFER(
    {
        Logger::Error("Failed to initialize audio engine");

        m_init = false;

    });

    TRACE("Creating AudioEngine...");
    Instance = this;

    m_init = true;

    m_bindings = new AudioEngineBindings(this);   

    ma_engine_config config = ma_engine_config_init();
    // TODO: Multi listener
    config.listenerCount = 1;

    IERRCHECK(ma_engine_init(NULL, &m_engine) == MA_SUCCESS);
    IERRDEFER(ma_engine_uninit(&m_engine));
}
AudioEngine::~AudioEngine()
{
    delete m_bindings;

    TRACE("Destroying AudioEngine...");
    if (m_init)
    {
        ma_engine_stop(&m_engine);
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

    for (uint32_t i = 0; i < m_audioStreams.Size(); ++i)
    {
        if (m_audioStreams.Exists(i))
        {
            MAISource* source = m_audioStreams[i];

            ma_sound_uninit(&source->MASound);
            ma_data_source_uninit(&source->MABaseSource);

            delete source;

            IWARN("AudioStream was not destroyed");
        }
    }

    if (m_init)
    {
        ma_engine_uninit(&m_engine);
    }
}

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

    IERROR("Invalid audio format");

    return 0;
}

constexpr static ma_format GetMAFormat(e_AudioFormat a_format)
{
    switch (a_format)
    {
    case AudioFormat_U8:
    {
        return ma_format_u8;
    }
    case AudioFormat_S16:
    {
        return ma_format_s16;
    }
    }

    IERROR("Invalid audio format");

    return ma_format_u8;
}

static ma_result MAIDataSourceRead(ma_data_source* a_dataSource, void* a_framesOut, ma_uint64 a_frameCount, ma_uint64* a_framesRead)
{
    return Instance->DataSourceRead(a_dataSource, a_framesOut, a_frameCount, a_framesRead);
}

ma_result AudioEngine::DataSourceRead(ma_data_source* a_dataSource, void* a_framesOut, ma_uint64 a_frameCount, ma_uint64* a_framesRead)
{
    IVERIFY(a_dataSource != NULL);

    *a_framesRead = 0;

    const MAISource* source = (MAISource*)a_dataSource;
    IVERIFY(m_audioSources.Exists(source->SourceAddr));

    TLockArray<AudioSourceBuffer> sources = m_audioSources.ToLockArray();
    AudioSourceBuffer& buffer = sources[source->SourceAddr];

    IVERIFY(m_audioClips.Exists(buffer.AudioClipAddr));

    TLockArray<AudioClip*> clips = m_audioClips.ToLockArray();
    AudioClip* clip = clips[buffer.AudioClipAddr];

    const uint64_t size = clip->GetSampleSize();
    const uint32_t formatSize = GetFormatSize(clip->GetAudioFormat());
    const uint32_t channelCount = clip->GetChannelCount();
    const uint32_t stride = formatSize * channelCount;

    uint32_t readSize;
    const uint8_t* dat = clip->GetAudioData(&m_allocator, buffer.SampleOffset, (uint32_t)a_frameCount, &readSize);

    memcpy(a_framesOut, dat, (uint64_t)readSize * stride);

    buffer.SampleOffset += readSize;

    *a_framesRead = (ma_uint64)readSize;

    return MA_SUCCESS;
}

static ma_result MAIDataSourceSeek(ma_data_source* a_dataSource, ma_uint64 a_frameIndex)
{
    return Instance->DataSourceSeek(a_dataSource, a_frameIndex);
}

ma_result AudioEngine::DataSourceSeek(ma_data_source* a_dataSource, ma_uint64 a_frameIndex)
{
    IVERIFY(a_dataSource != NULL);

    const MAISource* source = (MAISource*)a_dataSource;
    IVERIFY(m_audioSources.Exists(source->SourceAddr));

    TLockArray<AudioSourceBuffer> sources = m_audioSources.ToLockArray();
    AudioSourceBuffer& buffer = sources[source->SourceAddr];

    buffer.SampleOffset = (ma_uint64)a_frameIndex;

    return MA_SUCCESS;
}

static ma_result MAIDataSourceGetDataFormat(ma_data_source* a_dataSource, ma_format* a_format, ma_uint32* a_channels, ma_uint32* a_sampleRate, ma_channel* a_channelMap, size_t a_channelMapCap)
{
    IVERIFY(a_dataSource != NULL);

    const MAISource* source = (MAISource*)a_dataSource;

    *a_channels = (ma_uint32)source->ChannelCount;
    *a_format = source->MAFormat;
    *a_sampleRate = source->SampleRate;

    if (a_channelMap != NULL)
    {
        switch (source->ChannelCount)
        {
        // Mono
        case 1:
        {
            a_channelMap[0] = MA_CHANNEL_MONO;

            break;
        }
        // Stereo
        case 2:
        {
            a_channelMap[0] = MA_CHANNEL_FRONT_LEFT;
            a_channelMap[1] = MA_CHANNEL_FRONT_RIGHT;

            break;
        }
        // LRC or Stereo LFE not sure which is more suitable as I have seen both may change to Stereo LFE for musics mixes need to poke audio guy
        case 3:
        {
            a_channelMap[0] = MA_CHANNEL_FRONT_LEFT;
            a_channelMap[1] = MA_CHANNEL_FRONT_RIGHT;
            a_channelMap[2] = MA_CHANNEL_FRONT_CENTER;

            break;
        }
        // Could be LRC LFE, LR double center, or LR BL BR not sure which it more suitable
        case 4:
        {
            a_channelMap[0] = MA_CHANNEL_FRONT_LEFT;
            a_channelMap[1] = MA_CHANNEL_FRONT_RIGHT;
            a_channelMap[2] = MA_CHANNEL_FRONT_CENTER;
            a_channelMap[3] = MA_CHANNEL_BACK_CENTER;

            break;
        }
        // 5.0?
        case 5:
        {
            a_channelMap[0] = MA_CHANNEL_FRONT_LEFT;
            a_channelMap[1] = MA_CHANNEL_FRONT_RIGHT;
            a_channelMap[2] = MA_CHANNEL_FRONT_CENTER;
            a_channelMap[3] = MA_CHANNEL_BACK_LEFT;
            a_channelMap[4] = MA_CHANNEL_BACK_RIGHT;

            break;
        }
        // 5.1
        case 6:
        {
            a_channelMap[0] = MA_CHANNEL_FRONT_LEFT;
            a_channelMap[1] = MA_CHANNEL_FRONT_RIGHT;
            a_channelMap[2] = MA_CHANNEL_FRONT_CENTER;
            a_channelMap[3] = MA_CHANNEL_LFE;
            a_channelMap[4] = MA_CHANNEL_BACK_LEFT;
            a_channelMap[5] = MA_CHANNEL_BACK_RIGHT;

            break;
        }
        // Could be 7.0 or 6.1 with double center
        case 7:
        {
            a_channelMap[0] = MA_CHANNEL_FRONT_LEFT;
            a_channelMap[1] = MA_CHANNEL_FRONT_RIGHT;
            a_channelMap[2] = MA_CHANNEL_FRONT_CENTER;
            a_channelMap[3] = MA_CHANNEL_LFE;
            a_channelMap[4] = MA_CHANNEL_BACK_CENTER;
            a_channelMap[5] = MA_CHANNEL_BACK_LEFT;
            a_channelMap[6] = MA_CHANNEL_BACK_RIGHT;

            break;
        }
        // 7.1
        case 8:
        {
            a_channelMap[0] = MA_CHANNEL_FRONT_LEFT;
            a_channelMap[1] = MA_CHANNEL_FRONT_RIGHT;
            a_channelMap[2] = MA_CHANNEL_FRONT_CENTER;
            a_channelMap[3] = MA_CHANNEL_LFE;
            a_channelMap[4] = MA_CHANNEL_BACK_LEFT;
            a_channelMap[5] = MA_CHANNEL_BACK_RIGHT;
            a_channelMap[6] = MA_CHANNEL_SIDE_LEFT;
            a_channelMap[7] = MA_CHANNEL_SIDE_RIGHT;

            break;
        }
        default:
        {
            IERROR("Invalid channel config");
        }
        }
    }
    
    return MA_SUCCESS;

    // return Instance->DataSourceGetDataFormat(a_dataSource, a_format, a_channels, a_sampleRate, a_channelMap, a_channelMapCap);
}

ma_result AudioEngine::DataSourceGetDataFormat(ma_data_source* a_dataSource, ma_format* a_format, ma_uint32* a_channels, ma_uint32* a_sampleRate, ma_channel* a_channelMap, size_t a_channelMapCap)
{
    IVERIFY(a_dataSource != NULL);

    const MAISource* source = (MAISource*)a_dataSource;
    IVERIFY(m_audioSources.Exists(source->SourceAddr));

    const TReadLockArray<AudioSourceBuffer> sources = m_audioSources.ToReadLockArray();
    const AudioSourceBuffer& buffer = sources[source->SourceAddr];

    IVERIFY(m_audioClips.Exists(buffer.AudioClipAddr));

    const TReadLockArray<AudioClip*> clips = m_audioClips.ToReadLockArray();
    const AudioClip* clip = clips[buffer.AudioClipAddr];

    const uint32_t channelCount = clip->GetChannelCount();

    *a_channels = (ma_uint32)channelCount;
    *a_format = GetMAFormat(clip->GetAudioFormat());
    *a_sampleRate = (ma_uint32)clip->GetSampleRate();

    // Audio guys get veto power on the configurations
    switch (channelCount)
    {
    // Mono
    case 1:
    {
        a_channelMap[0] = MA_CHANNEL_MONO;

        break;
    }
    // Stereo
    case 2:
    {
        a_channelMap[0] = MA_CHANNEL_FRONT_LEFT;
        a_channelMap[1] = MA_CHANNEL_FRONT_RIGHT;

        break;
    }
    // LRC or Stereo LFE not sure which is more suitable as I have seen both may change to Stereo LFE for musics mixes need to poke audio guy
    case 3:
    {
        a_channelMap[0] = MA_CHANNEL_FRONT_LEFT;
        a_channelMap[1] = MA_CHANNEL_FRONT_RIGHT;
        a_channelMap[2] = MA_CHANNEL_FRONT_CENTER;

        break;
    }
    // Could be LRC LFE, LR double center, or LR BL BR not sure which it more suitable
    case 4:
    {
        a_channelMap[0] = MA_CHANNEL_FRONT_LEFT;
        a_channelMap[1] = MA_CHANNEL_FRONT_RIGHT;
        a_channelMap[2] = MA_CHANNEL_FRONT_CENTER;
        a_channelMap[3] = MA_CHANNEL_BACK_CENTER;

        break;
    }
    // 5.0?
    case 5:
    {
        a_channelMap[0] = MA_CHANNEL_FRONT_LEFT;
        a_channelMap[1] = MA_CHANNEL_FRONT_RIGHT;
        a_channelMap[2] = MA_CHANNEL_FRONT_CENTER;
        a_channelMap[3] = MA_CHANNEL_BACK_LEFT;
        a_channelMap[4] = MA_CHANNEL_BACK_RIGHT;

        break;
    }
    // 5.1
    case 6:
    {
        a_channelMap[0] = MA_CHANNEL_FRONT_LEFT;
        a_channelMap[1] = MA_CHANNEL_FRONT_RIGHT;
        a_channelMap[2] = MA_CHANNEL_FRONT_CENTER;
        a_channelMap[3] = MA_CHANNEL_LFE;
        a_channelMap[4] = MA_CHANNEL_BACK_LEFT;
        a_channelMap[5] = MA_CHANNEL_BACK_RIGHT;

        break;
    }
    // Could be 7.0 or 6.1 with double center
    case 7:
    {
        a_channelMap[0] = MA_CHANNEL_FRONT_LEFT;
        a_channelMap[1] = MA_CHANNEL_FRONT_RIGHT;
        a_channelMap[2] = MA_CHANNEL_FRONT_CENTER;
        a_channelMap[3] = MA_CHANNEL_LFE;
        a_channelMap[4] = MA_CHANNEL_BACK_CENTER;
        a_channelMap[5] = MA_CHANNEL_BACK_LEFT;
        a_channelMap[6] = MA_CHANNEL_BACK_RIGHT;

        break;
    }
    // 7.1
    case 8:
    {
        a_channelMap[0] = MA_CHANNEL_FRONT_LEFT;
        a_channelMap[1] = MA_CHANNEL_FRONT_RIGHT;
        a_channelMap[2] = MA_CHANNEL_FRONT_CENTER;
        a_channelMap[3] = MA_CHANNEL_LFE;
        a_channelMap[4] = MA_CHANNEL_BACK_LEFT;
        a_channelMap[5] = MA_CHANNEL_BACK_RIGHT;
        a_channelMap[6] = MA_CHANNEL_SIDE_LEFT;
        a_channelMap[7] = MA_CHANNEL_SIDE_RIGHT;

        break;
    }
    default:
    {
        IERROR("Invalid channel config");
    }
    }

    return MA_SUCCESS;
    // Return the format of the data here.
}

static ma_result MAIDataSourceGetCursor(ma_data_source* a_dataSource, ma_uint64* a_cursor)
{
    return Instance->DataSourceGetCursor(a_dataSource, a_cursor);
}

ma_result AudioEngine::DataSourceGetCursor(ma_data_source* a_dataSource, ma_uint64* a_cursor)
{
    IVERIFY(a_dataSource != NULL);

    *a_cursor = 0;

    const MAISource* source = (MAISource*)a_dataSource;
    IVERIFY(m_audioSources.Exists(source->SourceAddr));

    *a_cursor = m_audioSources[source->SourceAddr].SampleOffset;

    return MA_SUCCESS;
}

static ma_result MAIDataSourceGetLength(ma_data_source* a_dataSource, ma_uint64* a_length)
{
    return Instance->DataSourceGetLength(a_dataSource, a_length);
}

ma_result AudioEngine::DataSourceGetLength(ma_data_source* a_dataSource, ma_uint64* a_length)
{
    IVERIFY(a_dataSource != NULL);

    *a_length = 0;

    const MAISource* source = (MAISource*)a_dataSource;
    IVERIFY(m_audioSources.Exists(source->SourceAddr));

    const TReadLockArray<AudioSourceBuffer> a = m_audioSources.ToReadLockArray();

    const AudioSourceBuffer& buffer = a[source->SourceAddr];
    // if (IISBITSET(buffer.Flags, AudioSourceBuffer::LoopBitOffset))
    // {
    //     // Code smell but this is what docs says so not gonna question it
    //     return MA_NOT_IMPLEMENTED;
    // }

    IVERIFY(m_audioClips.Exists(buffer.AudioClipAddr));

    const TReadLockArray<AudioClip*> clips = m_audioClips.ToReadLockArray();

    *a_length = (ma_uint64)clips[buffer.AudioClipAddr]->GetSampleSize();

    return MA_SUCCESS;
}

static ma_data_source_vtable DataSourceVTable =
{
    MAIDataSourceRead,
    MAIDataSourceSeek,
    MAIDataSourceGetDataFormat,
    MAIDataSourceGetCursor,
    MAIDataSourceGetLength
};

void AudioEngine::Update()
{
    if (!m_init)
    {
        return;
    }

    {
        PROFILESTACK("Audio Listeners");

        const Array<AudioListenerBuffer> a = m_audioListeners.ToActiveArray();
        const uint32_t size = a.Size();

        for (uint32_t i = 0; i < size; ++i)
        {
            const AudioListenerBuffer& buffer = a[i];

            IVERIFY(buffer.TransformAddr != -1);

            const glm::mat4 mat = ObjectManager::GetGlobalMatrix(buffer.TransformAddr);

            const glm::vec3 pos = mat[3].xyz();

            ma_engine_listener_set_position(&m_engine, i, pos.x, pos.y, pos.z);

            // Not the correct way to do this but works in the majority of instances
            const glm::vec3 forward = glm::normalize(mat[2].xyz());

            ma_engine_listener_set_world_up(&m_engine, i, 0, -1, 0);
            ma_engine_listener_set_direction(&m_engine, i, forward.x, forward.y, forward.z);

            // TODO: Listener cones
        }
    }

    {
        PROFILESTACK("Audio Sources");

        const Array<bool> states = m_audioSources.ToStateArray();
        TLockArray<AudioSourceBuffer> sources = m_audioSources.ToLockArray();
        const uint32_t size = states.Size();

        TLockArray<AudioClip*> clips = m_audioClips.ToLockArray();

        for (uint32_t i = 0; i < size; ++i)
        {
            if (!states[i])
            {
                continue;
            }

            AudioSourceBuffer& buffer = sources[i];

            if (IISBITSET(buffer.Flags, AudioSourceBuffer::PlayBitOffset))
            {
                ICLEARBIT(buffer.Flags, AudioSourceBuffer::PlayBitOffset);
                buffer.SampleOffset = 0;

                IVERIFY(buffer.AudioClipAddr != -1);

                const AudioClip* clip = clips[buffer.AudioClipAddr];

                if (buffer.AudioStream != -1)
                {
                    IDEFER(buffer.AudioStream = -1);

                    IVERIFY(m_audioStreams.Exists(buffer.AudioStream));

                    MAISource* source = m_audioStreams[buffer.AudioStream];
                    IDEFER(
                    {
                        ma_sound_uninit(&source->MASound);
                        ma_data_source_uninit(&source->MABaseSource);

                        delete source;
                    });

                    m_audioStreams.Erase(buffer.AudioStream);
                }

                ma_data_source_config dataConfig = ma_data_source_config_init();
                dataConfig.vtable = &DataSourceVTable;

                // Miniaudio will try to retrieve info while we have the lock and there is no way that I am aware of 
                // to provide ahead of time or defer retrieval so have to pass it through kinda annoying
                MAISource* source = new MAISource();
                source->SourceAddr = i,
                source->ChannelCount = clip->GetChannelCount(),
                source->SampleRate = clip->GetSampleRate(),
                source->MAFormat = GetMAFormat(clip->GetAudioFormat());

                if (ma_data_source_init(&dataConfig, &source->MABaseSource) != MA_SUCCESS)
                {
                    Logger::Warning("Failed to create audio data source");

                    break;
                }

                ma_uint32 flags = 0;
                flags |= MA_SOUND_FLAG_NO_PITCH;

                if (!IISBITSET(buffer.Flags, AudioSourceBuffer::SpatialBitOffset))
                {
                    flags |= MA_SOUND_FLAG_NO_SPATIALIZATION;
                }

                // If you where wondering where I got this number from no idea just picked a random number that seemed right probably change down the line
                if (clip->GetDuration() > 5.0)
                {
                    flags |= MA_SOUND_FLAG_STREAM;
                }

                if (ma_sound_init_from_data_source(&m_engine, &source->MABaseSource, flags, NULL, &source->MASound) != MA_SUCCESS)
                {
                    Logger::Warning("Failed to play audio source");

                    ma_data_source_uninit(&source->MABaseSource);

                    break;
                }

                if (IISBITSET(buffer.Flags, AudioSourceBuffer::LoopBitOffset))
                {
                    ma_sound_set_looping(&source->MASound, MA_TRUE);
                }

                ma_sound_start(&source->MASound);

                buffer.AudioStream = m_audioStreams.PushVal(source);

                ISETBIT(buffer.Flags, AudioSourceBuffer::PlayingBitOffset);
            }

            if (!IISBITSET(buffer.Flags, AudioSourceBuffer::PlayingBitOffset))
            {
                continue;
            }

            IVERIFY(m_audioStreams.Exists(buffer.AudioStream));

            MAISource* source = m_audioStreams[buffer.AudioStream];

            if (!ma_sound_is_playing(&source->MASound))
            {
                ICLEARBIT(buffer.Flags, AudioSourceBuffer::PlayingBitOffset);

                continue;
            }

            float gain = 1.0f;
            if (buffer.AudioMixerAddr != -1)
            {
                IVERIFY(m_audioMixers.Exists(buffer.AudioMixerAddr));

                const AudioMixerBuffer mixer = m_audioMixers[buffer.AudioMixerAddr];
                gain = mixer.Gain;
            }

            ma_sound_set_volume(&source->MASound, gain);

            if (ma_sound_is_spatialization_enabled(&source->MASound))
            {
                const glm::mat4 mat = ObjectManager::GetGlobalMatrix(buffer.TransformAddr);

                const glm::vec3 pos = mat[3].xyz();
                ma_sound_set_position(&source->MASound, pos.x, pos.y, pos.z);

                // Not the correct way to do this but works in the majority of instances
                const glm::vec3 forward = glm::normalize(mat[2].xyz());
                ma_sound_set_direction(&source->MASound, forward.x, forward.y, forward.z);
            }
        }
    }
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
