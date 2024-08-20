// Icarian Engine - C# Game Engine
// 
// License at end of file.

#pragma once

#include <cstdint>

#include "Audio/AudioListenerBuffer.h"
#include "DataTypes/RingAllocator.h"
#include "DataTypes/TNCArray.h"

class AudioClip;
class AudioEngineBindings;

#include "EngineAudioSourceInteropStructures.h"
#include "EngineAudioMixerInteropStructures.h"

#include <miniaudio.h>

class AudioEngine
{
private:
    friend class AudioEngineBindings;

    constexpr static uint32_t SampleRate = 48000;

    RingAllocator                 m_allocator;

    bool                          m_init;
    AudioEngineBindings*          m_bindings;

    ma_device                     m_device;

    TNCArray<AudioClip*>          m_audioClips;
    TNCArray<AudioSourceBuffer>   m_audioSources;
    TNCArray<AudioListenerBuffer> m_audioListeners;
    TNCArray<AudioMixerBuffer>    m_audioMixers;

protected:

public:
    AudioEngine();
    ~AudioEngine();

    void AudioOutCallback(ma_device* a_device, void* a_output, ma_uint32 a_frameCount);

    void Update();
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
