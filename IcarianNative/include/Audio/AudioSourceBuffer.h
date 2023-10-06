#pragma once

#include <AL/al.h>

#include <cstdint>

struct AudioSourceBuffer
{
    static constexpr uint32_t PlayBitOffset = 0;
    static constexpr uint32_t LoopBitOffset = 1;
    static constexpr uint32_t PlayingBitOffset = 2;

    static constexpr uint32_t BufferCount = 3;

    uint32_t TransformAddr;
    uint32_t AudioClipAddr;
    uint64_t SampleOffset;
    uint32_t Flags;
    ALuint Buffers[BufferCount];
    ALuint Source;
};