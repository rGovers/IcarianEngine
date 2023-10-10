#pragma once

#include <AL/al.h>
#include <AL/alc.h>
#include <cstdint>

#include "Audio/AudioListenerBuffer.h"
#include "Audio/AudioSourceBuffer.h"
#include "DataTypes/TNCArray.h"

class AudioClip;
class AudioEngineBindings;

class AudioEngine
{
private:
    friend class AudioEngineBindings;

    constexpr static uint32_t AudioBufferSampleSize = 4096;

    AudioEngineBindings*          m_bindings;

    ALCdevice*                    m_device;
    ALCcontext*                   m_context;

    TNCArray<AudioClip*>          m_audioClips;
    TNCArray<AudioSourceBuffer>   m_audioSources;
    TNCArray<AudioListenerBuffer> m_audioListeners;

protected:

public:
    AudioEngine();
    ~AudioEngine();

    void Update();
};