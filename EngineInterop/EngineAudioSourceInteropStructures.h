// Icarian Engine - C# Game Engine
// 
// License at end of file.

#pragma once

#include "InteropTypes.h"

#ifdef CUBE_LANGUAGE_CSHARP
namespace IcarianEngine.Audio {
#endif

/// @file EngineAudioSourceInteropStructures.h

/// @cond INTERNAL

IOP_PACKED struct AudioSourceBuffer
{
    IOP_CSPUBLIC static IOP_CONSTEXPR IOP_UINT32 PlayBitOffset = 0;
    IOP_CSPUBLIC static IOP_CONSTEXPR IOP_UINT32 LoopBitOffset = 1;
    IOP_CSPUBLIC static IOP_CONSTEXPR IOP_UINT32 PlayingBitOffset = 2;
    IOP_CSPUBLIC static IOP_CONSTEXPR IOP_UINT32 SpatialBitOffset = 3;

    IOP_CSPUBLIC IOP_UINT32 TransformAddr;
    IOP_CSPUBLIC IOP_UINT32 AudioClipAddr;
    IOP_CSPUBLIC IOP_UINT32 AudioMixerAddr;
    IOP_CSPUBLIC IOP_UINT32 AudioStream;
    IOP_CSPUBLIC IOP_UINT64 SampleOffset;
    IOP_CSPUBLIC IOP_UINT32 Flags;
};

/// @endcond

#ifdef CUBE_LANGUAGE_CSHARP
}
#endif

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