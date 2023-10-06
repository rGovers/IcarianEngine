#include "Audio/AudioEngineBindings.h"

#include "Audio/AudioClips/OGGAudioClip.h"
#include "Audio/AudioEngine.h"
#include "Flare/IcarianAssert.h"
#include "Flare/IcarianDefer.h"
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
    F(AudioSourceBuffer, IcarianEngine.Audio, AudioSource, GetAudioSourceBuffer, { return Instance->GetAudioSourceBuffer(a_addr); }, uint32_t a_addr) \
    F(void, IcarianEngine.Audio, AudioSource, SetAudioSourceBuffer, { Instance->SetAudioSourceBuffer(a_addr, a_buffer); }, uint32_t a_addr, AudioSourceBuffer a_buffer)

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

    return -1;
}
void AudioEngineBindings::DestroyAudioClip(uint32_t a_addr) const
{
    TRACE("Destroying AudioClip");
    ICARIAN_ASSERT_MSG(a_addr < m_engine->m_audioClips.Size(), "DestroyAudioClip out of bounds.");
    ICARIAN_ASSERT_MSG(m_engine->m_audioClips[a_addr] != nullptr, "DestroyAudioClip value does not exist.");

    const AudioClip* clip = m_engine->m_audioClips[a_addr];
    IDEFER(delete clip);

    m_engine->m_audioClips.Erase(a_addr);
}

float AudioEngineBindings::GetAudioClipDuration(uint32_t a_addr) const
{
    ICARIAN_ASSERT_MSG(a_addr < m_engine->m_audioClips.Size(), "GetAudioClipDuration out of bounds.");
    ICARIAN_ASSERT_MSG(m_engine->m_audioClips[a_addr] != nullptr, "GetAudioClipDuration value does not exist.");

    const AudioClip* clip = m_engine->m_audioClips[a_addr];
    return clip->GetDuration();
}
uint32_t AudioEngineBindings::GetAudioClipSampleRate(uint32_t a_addr) const
{
    ICARIAN_ASSERT_MSG(a_addr < m_engine->m_audioClips.Size(), "GetAudioClipSampleRate out of bounds.");
    ICARIAN_ASSERT_MSG(m_engine->m_audioClips[a_addr] != nullptr, "GetAudioClipSampleRate value does not exist.");

    const AudioClip* clip = m_engine->m_audioClips[a_addr];
    return clip->GetSampleRate();
}
uint32_t AudioEngineBindings::GetAudioClipChannelCount(uint32_t a_addr) const
{
    ICARIAN_ASSERT_MSG(a_addr < m_engine->m_audioClips.Size(), "GetAudioClipChannelCount out of bounds.");
    ICARIAN_ASSERT_MSG(m_engine->m_audioClips[a_addr] != nullptr, "GetAudioClipChannelCount value does not exist.");

    const AudioClip* clip = m_engine->m_audioClips[a_addr];
    return clip->GetChannelCount();
}

uint32_t AudioEngineBindings::GenerateAudioSource(uint32_t a_transformAddr, uint32_t a_clipAddr) const
{
    TRACE("Creating AudioSource");
    AudioSourceBuffer buffer;
    buffer.TransformAddr = a_transformAddr;
    buffer.AudioClipAddr = a_clipAddr;
    buffer.SampleOffset = 0;
    buffer.Flags = 0;
    for (uint32_t i = 0; i < AudioSourceBuffer::BufferCount; ++i)
    {
        buffer.Buffers[i] = -1;
    }
    buffer.Source = -1;

    return m_engine->m_audioSources.PushVal(buffer);
}
void AudioEngineBindings::DestroyAudioSource(uint32_t a_addr) const
{
    TRACE("Destroying AudioSource");
    ICARIAN_ASSERT_MSG(a_addr < m_engine->m_audioSources.Size(), "DestroyAudioSource out of bounds.");

    const AudioSourceBuffer buffer = m_engine->m_audioSources[a_addr];
    IDEFER(
    {
        alSourceStop(buffer.Source);

        for (uint32_t i = 0; i < AudioSourceBuffer::BufferCount; ++i)
        {
            if (buffer.Buffers[i] != -1)
            {
                alDeleteBuffers(1, &buffer.Buffers[i]);
            }
        }

        if (buffer.Source != -1)
        {
            alDeleteSources(1, &buffer.Source);
        }
    });

    m_engine->m_audioSources.Erase(a_addr);
}

void AudioEngineBindings::PlayAudioSource(uint32_t a_addr) const
{
    TRACE("Playing AudioSource");
    ICARIAN_ASSERT_MSG(a_addr < m_engine->m_audioSources.Size(), "PlayAudioSource out of bounds.");

    AudioSourceBuffer buffer = m_engine->m_audioSources[a_addr];
    buffer.Flags |= (0b1 << AudioSourceBuffer::PlayBitOffset);
    m_engine->m_audioSources.LockSet(a_addr, buffer);
}
void AudioEngineBindings::SetLoopAudioSource(uint32_t a_addr, bool a_loop) const
{
    ICARIAN_ASSERT_MSG(a_addr < m_engine->m_audioSources.Size(), "SetLoopAudioSource out of bounds.");

    AudioSourceBuffer buffer = m_engine->m_audioSources[a_addr];
    if (a_loop)
    {
        buffer.Flags |= (0b1 << AudioSourceBuffer::LoopBitOffset);
    }
    else
    {
        buffer.Flags &= ~(0b1 << AudioSourceBuffer::LoopBitOffset);
    }
    m_engine->m_audioSources.LockSet(a_addr, buffer);
}

AudioSourceBuffer AudioEngineBindings::GetAudioSourceBuffer(uint32_t a_addr) const
{
    ICARIAN_ASSERT_MSG(a_addr < m_engine->m_audioSources.Size(), "GetAudioSourceBuffer out of bounds.");

    return m_engine->m_audioSources[a_addr];
}
void AudioEngineBindings::SetAudioSourceBuffer(uint32_t a_addr, const AudioSourceBuffer& a_buffer) const
{
    ICARIAN_ASSERT_MSG(a_addr < m_engine->m_audioSources.Size(), "SetAudioSourceBuffer out of bounds.");

    m_engine->m_audioSources[a_addr] = a_buffer;
}