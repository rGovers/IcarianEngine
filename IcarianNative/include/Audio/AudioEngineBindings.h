// Icarian Engine - C# Game Engine
// 
// License at end of file.

#pragma once

#include <cstdint>
#include <filesystem>

class AudioEngine;

#include "Audio/AudioSourceBuffer.h"
#include "Audio/AudioMixerBuffer.h"

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
    bool GetAudioSourcePlayingState(uint32_t a_addr) const;
    AudioSourceBuffer GetAudioSourceBuffer(uint32_t a_addr) const;
    void SetAudioSourceBuffer(uint32_t a_addr, const AudioSourceBuffer& a_buffer) const;

    uint32_t GenerateAudioMixer() const;
    void DestroyAudioMixer(uint32_t a_addr) const;
    AudioMixerBuffer GetAudioMixerBuffer(uint32_t a_addr) const;
    void SetAudioMixerBuffer(uint32_t a_addr, const AudioMixerBuffer& a_buffer) const;

    uint32_t GenerateAudioListener(uint32_t a_transformAddr) const;
    void DestroyAudioListener(uint32_t a_addr) const;
};

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