#pragma once

#include <cstdint>
#include <filesystem>

class AudioEngine;

#include "Audio/AudioSourceBuffer.h"

class AudioEngineBindings
{
private:
    AudioEngine* m_engine;

protected:

public:
    AudioEngineBindings(AudioEngine* a_engine);
    ~AudioEngineBindings();

    uint32_t GenerateAudioClipFromFile(const std::filesystem::path& a_path) const;
    void DestroyAudioClip(uint32_t a_addr) const;
    float GetAudioClipDuration(uint32_t a_addr) const;
    uint32_t GetAudioClipSampleRate(uint32_t a_addr) const;
    uint32_t GetAudioClipChannelCount(uint32_t a_addr) const;

    uint32_t GenerateAudioSource(uint32_t a_transformAddr, uint32_t a_clipAddr) const;
    void DestroyAudioSource(uint32_t a_addr) const;
    void PlayAudioSource(uint32_t a_addr) const;
    void SetLoopAudioSource(uint32_t a_addr, bool a_loop) const;
    AudioSourceBuffer GetAudioSourceBuffer(uint32_t a_addr) const;
    void SetAudioSourceBuffer(uint32_t a_addr, const AudioSourceBuffer& a_buffer) const;
};