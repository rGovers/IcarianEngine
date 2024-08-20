// Icarian Engine - C# Game Engine
// 
// License at end of file.

#include "Audio/AudioEngineBindings.h"

#include "Audio/AudioClips/OGGAudioClip.h"
#include "Audio/AudioClips/WAVAudioClip.h"
#include "Audio/AudioEngine.h"
#include "Core/Bitfield.h"
#include "Core/IcarianAssert.h"
#include "Core/IcarianDefer.h"
#include "IcarianError.h"
#include "Runtime/RuntimeManager.h"
#include "Trace.h"

static AudioEngineBindings* Instance = nullptr;

#define AUDIOENGINE_BINDING_FUNCTION_TABLE(F) \
    F(void, IcarianEngine.Audio, AudioClip, DestroyAudioClip, { Instance->DestroyAudioClip(a_addr); }, uint32_t a_addr) \
    F(float, IcarianEngine.Audio, AudioClip, GetAudioClipDuration, { return Instance->GetAudioClipDuration(a_addr); }, uint32_t a_addr) \
    F(uint32_t, IcarianEngine.Audio, AudioClip, GetAudioClipSampleRate, { return Instance->GetAudioClipSampleRate(a_addr); }, uint32_t a_addr) \
    F(uint32_t, IcarianEngine.Audio, AudioClip, GetAudioClipChannelCount, { return Instance->GetAudioClipChannelCount(a_addr); }, uint32_t a_addr) \
    \
    F(uint32_t, IcarianEngine.Audio, AudioSource, GenerateAudioSource, { return Instance->GenerateAudioSource(a_transformAddr, a_clipAddr); }, uint32_t a_transformAddr, uint32_t a_clipAddr) \
    F(void, IcarianEngine.Audio, AudioSource, DestroyAudioSource, { Instance->DestroyAudioSource(a_addr); }, uint32_t a_addr) \
    F(void, IcarianEngine.Audio, AudioSource, PlayAudioSource, { Instance->PlayAudioSource(a_addr); }, uint32_t a_addr) \
    F(void, IcarianEngine.Audio, AudioSource, SetLoopAudioSource, { Instance->SetLoopAudioSource(a_addr, (bool)a_loop); }, uint32_t a_addr, uint32_t a_loop) \
    F(uint32_t, IcarianEngine.Audio, AudioSource, GetAudioSourcePlayingState, { return Instance->GetAudioSourcePlayingState(a_addr); }, uint32_t a_addr) \
    F(AudioSourceBuffer, IcarianEngine.Audio, AudioSource, GetAudioSourceBuffer, { return Instance->GetAudioSourceBuffer(a_addr); }, uint32_t a_addr) \
    F(void, IcarianEngine.Audio, AudioSource, SetAudioSourceBuffer, { Instance->SetAudioSourceBuffer(a_addr, a_buffer); }, uint32_t a_addr, AudioSourceBuffer a_buffer) \
    \
    F(uint32_t, IcarianEngine.Audio, AudioMixer, GenerateAudioMixer, { return Instance->GenerateAudioMixer(); }) \
    F(void, IcarianEngine.Audio, AudioMixer, DestroyAudioMixer, { Instance->DestroyAudioMixer(a_addr); }, uint32_t a_addr) \
    F(AudioMixerBuffer, IcarianEngine.Audio, AudioMixer, GetAudioMixerBuffer, { return Instance->GetAudioMixerBuffer(a_addr); }, uint32_t a_addr) \
    F(void, IcarianEngine.Audio, AudioMixer, SetAudioMixerBuffer, { Instance->SetAudioMixerBuffer(a_addr, a_buffer); }, uint32_t a_addr, AudioMixerBuffer a_buffer) \
    \
    F(uint32_t, IcarianEngine.Audio, AudioListener, GenerateAudioListener, { return Instance->GenerateAudioListener(a_transformAddr); }, uint32_t a_transformAddr) \
    F(void, IcarianEngine.Audio, AudioListener, DestroyAudioListener, { Instance->DestroyAudioListener(a_addr); }, uint32_t a_addr) \

AUDIOENGINE_BINDING_FUNCTION_TABLE(RUNTIME_FUNCTION_DEFINITION)

RUNTIME_FUNCTION(uint32_t, AudioClip, GenerateFromFile,
{
    char* path = mono_string_to_utf8(a_path);
    IDEFER(mono_free(path));

    return Instance->GenerateAudioClipFromFile(path);
}, MonoString* a_path)

AudioEngineBindings::AudioEngineBindings(AudioEngine* a_engine)
{   
    m_engine = a_engine;

    BIND_FUNCTION(IcarianEngine.Audio, AudioClip, GenerateFromFile);

    Instance = this;

    AUDIOENGINE_BINDING_FUNCTION_TABLE(RUNTIME_FUNCTION_ATTACH);
}
AudioEngineBindings::~AudioEngineBindings()
{
    
}

uint32_t AudioEngineBindings::GenerateAudioClipFromFile(const std::filesystem::path& a_path) const
{
    TRACE("Creating AudioClip");
    const std::filesystem::path ext = a_path.extension();

    if (ext == ".ogg")
    {
        return m_engine->m_audioClips.PushVal(new OGGAudioClip(a_path));
    }
    else if (ext == ".wav")
    {
        return m_engine->m_audioClips.PushVal(new WAVAudioClip(a_path));
    }

    return -1;
}
void AudioEngineBindings::DestroyAudioClip(uint32_t a_addr) const
{
    TRACE("Destroying AudioClip");
    IVERIFY(a_addr < m_engine->m_audioClips.Size());
    IVERIFY(m_engine->m_audioClips.Exists(a_addr));

    const AudioClip* clip = m_engine->m_audioClips[a_addr];
    IDEFER(delete clip);

    m_engine->m_audioClips.Erase(a_addr);
}

float AudioEngineBindings::GetAudioClipDuration(uint32_t a_addr) const
{
    IVERIFY(a_addr < m_engine->m_audioClips.Size());
    IVERIFY(m_engine->m_audioClips.Exists(a_addr));

    const AudioClip* clip = m_engine->m_audioClips[a_addr];
    return clip->GetDuration();
}
uint32_t AudioEngineBindings::GetAudioClipSampleRate(uint32_t a_addr) const
{
    IVERIFY(a_addr < m_engine->m_audioClips.Size());
    IVERIFY(m_engine->m_audioClips.Exists(a_addr));

    const AudioClip* clip = m_engine->m_audioClips[a_addr];
    return clip->GetSampleRate();
}
uint32_t AudioEngineBindings::GetAudioClipChannelCount(uint32_t a_addr) const
{
    IVERIFY(a_addr < m_engine->m_audioClips.Size());
    IVERIFY(m_engine->m_audioClips.Exists(a_addr));

    const AudioClip* clip = m_engine->m_audioClips[a_addr];
    return clip->GetChannelCount();
}

uint32_t AudioEngineBindings::GenerateAudioSource(uint32_t a_transformAddr, uint32_t a_clipAddr) const
{
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
    TRACE("Destroying AudioSource");
    IVERIFY(a_addr < m_engine->m_audioSources.Size());
    IVERIFY(m_engine->m_audioSources.Exists(a_addr));

    const AudioSourceBuffer buffer = m_engine->m_audioSources[a_addr];

    m_engine->m_audioSources.Erase(a_addr);
}

void AudioEngineBindings::PlayAudioSource(uint32_t a_addr) const
{
    TRACE("Playing AudioSource");
    IVERIFY(a_addr < m_engine->m_audioSources.Size());
    IVERIFY(m_engine->m_audioSources.Exists(a_addr));

    AudioSourceBuffer buffer = m_engine->m_audioSources[a_addr];
    ISETBIT(buffer.Flags, AudioSourceBuffer::PlayBitOffset);

    m_engine->m_audioSources.LockSet(a_addr, buffer);
}
void AudioEngineBindings::SetLoopAudioSource(uint32_t a_addr, bool a_loop) const
{
    IVERIFY(a_addr < m_engine->m_audioSources.Size());
    IVERIFY(m_engine->m_audioSources.Exists(a_addr));

    AudioSourceBuffer buffer = m_engine->m_audioSources[a_addr];
    ITOGGLEBIT(a_loop, buffer.Flags, AudioSourceBuffer::LoopBitOffset);

    m_engine->m_audioSources.LockSet(a_addr, buffer);
}
bool AudioEngineBindings::GetAudioSourcePlayingState(uint32_t a_addr) const
{
    IVERIFY(a_addr < m_engine->m_audioSources.Size());
    IVERIFY(m_engine->m_audioSources.Exists(a_addr));

    const AudioSourceBuffer buffer = m_engine->m_audioSources[a_addr];
    return buffer.Flags & (0b1 << AudioSourceBuffer::PlayingBitOffset);
}
AudioSourceBuffer AudioEngineBindings::GetAudioSourceBuffer(uint32_t a_addr) const
{
    IVERIFY(a_addr < m_engine->m_audioSources.Size());
    IVERIFY(m_engine->m_audioSources.Exists(a_addr));

    return m_engine->m_audioSources[a_addr];
}
void AudioEngineBindings::SetAudioSourceBuffer(uint32_t a_addr, const AudioSourceBuffer& a_buffer) const
{
    IVERIFY(a_addr < m_engine->m_audioSources.Size());
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
    TRACE("Destroying AudioMixer");
    IVERIFY(a_addr < m_engine->m_audioMixers.Size());
    IVERIFY(m_engine->m_audioMixers.Exists(a_addr));

    m_engine->m_audioMixers.Erase(a_addr);
}
AudioMixerBuffer AudioEngineBindings::GetAudioMixerBuffer(uint32_t a_addr) const
{
    IVERIFY(a_addr < m_engine->m_audioMixers.Size());
    IVERIFY(m_engine->m_audioMixers.Exists(a_addr));

    return m_engine->m_audioMixers[a_addr];
}
void AudioEngineBindings::SetAudioMixerBuffer(uint32_t a_addr, const AudioMixerBuffer& a_buffer) const
{
    IVERIFY(a_addr < m_engine->m_audioMixers.Size());
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
    TRACE("Destroying AudioListener");
    IVERIFY(a_addr < m_engine->m_audioListeners.Size());
    IVERIFY(m_engine->m_audioListeners.Exists(a_addr));

    m_engine->m_audioListeners.Erase(a_addr);
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