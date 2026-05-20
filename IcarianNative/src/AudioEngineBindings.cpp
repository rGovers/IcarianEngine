// Icarian Engine - C# Game Engine
// 
// License at end of file.

#include "Audio/AudioEngineBindings.h"

#include "Audio/AudioClips/OGGAudioClip.h"
#include "Audio/AudioClips/WAVAudioClip.h"
#include "Audio/AudioEngine.h"
#include "Core/Bitfield.h"
#include "Core/IcarianDefer.h"
#include "Core/IcarianError.h"
#include "Core/StringUtils.h"
#include "DataTypes/Allocators/ComplexAllocator.h"
#include "IcarianError.h"
#include "IO.h"
#include "Runtime/RuntimeManager.h"
#include "Trace.h"

#include "EngineAudioClipInterop.h"
#include "EngineAudioListenerInterop.h"
#include "EngineAudioMixerInterop.h"
#include "EngineAudioSourceInterop.h"

static AudioEngineBindings* Instance = nullptr;

ENGINE_AUDIOCLIP_EXPORT_TABLE(RUNTIME_FUNCTION_DEFINITION);
ENGINE_AUDIOLISTENER_EXPORT_TABLE(RUNTIME_FUNCTION_DEFINITION);
ENGINE_AUDIOMIXER_EXPORT_TABLE(RUNTIME_FUNCTION_DEFINITION);
ENGINE_AUDIOSOURCE_EXPORT_TABLE(RUNTIME_FUNCTION_DEFINITION);

AudioEngineBindings::AudioEngineBindings(AudioEngine* a_engine)
{
    m_engine = a_engine;

    Instance = this;

    ENGINE_AUDIOCLIP_EXPORT_TABLE(RUNTIME_FUNCTION_ATTACH);
    ENGINE_AUDIOLISTENER_EXPORT_TABLE(RUNTIME_FUNCTION_ATTACH);
    ENGINE_AUDIOMIXER_EXPORT_TABLE(RUNTIME_FUNCTION_ATTACH);
    ENGINE_AUDIOSOURCE_EXPORT_TABLE(RUNTIME_FUNCTION_ATTACH);
}
AudioEngineBindings::~AudioEngineBindings()
{

}

uint32_t AudioEngineBindings::GenerateAudioClipFromFile(const char* a_path) const
{
    const COWU8String str = COWU8String(a_path, m_engine->m_allocator);

    return GenerateAudioClipFromFile(str);
}
uint32_t AudioEngineBindings::GenerateAudioClipFromFile(const COWU8String& a_path) const
{
    IERRBLOCK;

    TRACE("Creating AudioClip");

    const COWU8String ext = IO::GetExtension(a_path, m_engine->m_allocator);

    Allocator* allocator = m_engine->GetAllocator();
    AudioClip* clip = ILAMBDA(
    {
        switch (StringHash<uint32_t>(ext.CStr()))
        {
        case StringHash<uint32_t>(".ogg"):
        {
            ILRETURN (AudioClip*)allocator->Create<OGGAudioClip>(a_path);
        }
        case StringHash<uint32_t>(".wav"):
        {
            ILRETURN (AudioClip*)allocator->Create<WAVAudioClip>(a_path, m_engine->m_allocator);
        }
        default:
        {
            break;
        }
        }

        ILRETURN (AudioClip*)nullptr;
    });

    IERRCHECKRET(clip != nullptr, -1);

    IERRDEFER(allocator->Destroy(clip));

    IERRCHECKRET(clip->GetSampleSize() > 0, -1);
    IERRCHECKRET(clip->GetSampleRate() > 0, -1);

    return m_engine->m_audioClips.PushVal(clip);
}
void AudioEngineBindings::DestroyAudioClip(uint32_t a_addr) const
{
    IVERIFY(m_engine->m_audioClips.Exists(a_addr));

    TRACE("Destroying AudioClip");
    Allocator* allocator = m_engine->GetAllocator();

    AudioClip* clip = m_engine->m_audioClips[a_addr];
    IDEFER(allocator->Destroy(clip));

    m_engine->m_audioClips.Erase(a_addr);
}

float AudioEngineBindings::GetAudioClipDuration(uint32_t a_addr) const
{
    IVERIFY(m_engine->m_audioClips.Exists(a_addr));

    const TReadLockArray<AudioClip*> a = m_engine->m_audioClips.ToReadLockArray();
    const AudioClip* clip = a[a_addr];

    return clip->GetDuration();
}
uint32_t AudioEngineBindings::GetAudioClipSampleRate(uint32_t a_addr) const
{
    IVERIFY(m_engine->m_audioClips.Exists(a_addr));

    const AudioClip* clip = m_engine->m_audioClips[a_addr];
    return clip->GetSampleRate();
}
uint32_t AudioEngineBindings::GetAudioClipChannelCount(uint32_t a_addr) const
{
    IVERIFY(m_engine->m_audioClips.Exists(a_addr));

    const TReadLockArray<AudioClip*> a = m_engine->m_audioClips.ToReadLockArray();
    const AudioClip* clip = a[a_addr];

    return clip->GetChannelCount();
}

uint32_t AudioEngineBindings::GenerateAudioSource(uint32_t a_transformAddr, uint32_t a_clipAddr) const
{
    IVERIFY(a_transformAddr != uint32_t(-1));
    IVERIFY(a_clipAddr != uint32_t(-1));

    TRACE("Creating AudioSource");
    const AudioSourceBuffer buffer
    {
        .TransformAddr = a_transformAddr,
        .AudioClipAddr = a_clipAddr,
        .AudioMixerAddr = (uint32_t)-1,
        .AudioStream = (uint32_t)-1
    };

    return m_engine->m_audioSources.PushVal(buffer);
}
void AudioEngineBindings::DestroyAudioSource(uint32_t a_addr) const
{
    IVERIFY(m_engine->m_audioSources.Exists(a_addr));

    TRACE("Destroying AudioSource");
    AudioSourceBuffer buffer = m_engine->m_audioSources[a_addr];
    IDEFER(
    {
        if (buffer.AudioStream != uint32_t(-1))
        {
            IDEFER(buffer.AudioStream = -1);

            IVERIFY(m_engine->m_audioStreams.Exists(buffer.AudioStream));

            MAISource* source = m_engine->m_audioStreams[buffer.AudioStream];
            IDEFER(
            {
                ma_sound_stop(&source->MASound);
                ma_sound_uninit(&source->MASound);
                ma_data_source_uninit(&source->MABaseSource);

                m_engine->m_allocator->Destroy(source);
            });

            m_engine->m_audioStreams.Erase(buffer.AudioStream);
        }
    });

    m_engine->m_audioSources.Erase(a_addr);
}

void AudioEngineBindings::PlayAudioSource(uint32_t a_addr) const
{
    TRACE("Playing AudioSource");
    IVERIFY(m_engine->m_audioSources.Exists(a_addr));

    AudioSourceBuffer buffer = m_engine->m_audioSources[a_addr];
    ISETBIT(buffer.Flags, AudioSourceBuffer::PlayBitOffset);

    m_engine->m_audioSources.LockSet(a_addr, buffer);
}
void AudioEngineBindings::SetLoopAudioSource(uint32_t a_addr, bool a_loop) const
{
    IVERIFY(m_engine->m_audioSources.Exists(a_addr));

    AudioSourceBuffer buffer = m_engine->m_audioSources[a_addr];
    ITOGGLEBIT(a_loop, buffer.Flags, AudioSourceBuffer::LoopBitOffset);

    m_engine->m_audioSources.LockSet(a_addr, buffer);
}
bool AudioEngineBindings::GetAudioSourcePlayingState(uint32_t a_addr) const
{
    IVERIFY(m_engine->m_audioSources.Exists(a_addr));

    const AudioSourceBuffer buffer = m_engine->m_audioSources[a_addr];
    return IISBITSET(buffer.Flags, AudioSourceBuffer::PlayBitOffset);
}
AudioSourceBuffer AudioEngineBindings::GetAudioSourceBuffer(uint32_t a_addr) const
{
    IVERIFY(m_engine->m_audioSources.Exists(a_addr));

    return m_engine->m_audioSources[a_addr];
}
void AudioEngineBindings::SetAudioSourceBuffer(uint32_t a_addr, const AudioSourceBuffer& a_buffer) const
{
    IVERIFY(m_engine->m_audioSources.Exists(a_addr));

    m_engine->m_audioSources.LockSet(a_addr, a_buffer);
}

uint32_t AudioEngineBindings::GenerateAudioMixer() const
{
    TRACE("Creating AudioMixer");
    const AudioMixerBuffer buffer = 
    {
        .Gain = 1.0f
    };

    return m_engine->m_audioMixers.PushVal(buffer);
}
void AudioEngineBindings::DestroyAudioMixer(uint32_t a_addr) const
{
    IVERIFY(m_engine->m_audioMixers.Exists(a_addr));

    TRACE("Destroying AudioMixer");
    m_engine->m_audioMixers.Erase(a_addr);
}
AudioMixerBuffer AudioEngineBindings::GetAudioMixerBuffer(uint32_t a_addr) const
{
    IVERIFY(m_engine->m_audioMixers.Exists(a_addr));

    return m_engine->m_audioMixers[a_addr];
}
void AudioEngineBindings::SetAudioMixerBuffer(uint32_t a_addr, const AudioMixerBuffer& a_buffer) const
{
    IVERIFY(m_engine->m_audioMixers.Exists(a_addr));

    m_engine->m_audioMixers.LockSet(a_addr, a_buffer);
}

uint32_t AudioEngineBindings::GenerateAudioListener(uint32_t a_transformAddr) const
{
    TRACE("Creating AudioListener");
    const AudioListenerBuffer buffer 
    {
        .TransformAddr = a_transformAddr
    };

    return m_engine->m_audioListeners.PushVal(buffer);
}
void AudioEngineBindings::DestroyAudioListener(uint32_t a_addr) const
{
    IVERIFY(m_engine->m_audioListeners.Exists(a_addr));

    TRACE("Destroying AudioListener");
    m_engine->m_audioListeners.Erase(a_addr);
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
