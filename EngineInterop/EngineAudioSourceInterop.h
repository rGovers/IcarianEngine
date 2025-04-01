// Icarian Engine - C# Game Engine
// 
// License at end of file.

#pragma once

#include "InteropTypes.h"

/// @file EngineAudioSourceInterop.h

/// @cond INTERNAL

#ifdef CUBE_LANGUAGE_CPP
#include "DeletionQueue.h"
#endif

#define ENGINE_AUDIOSOURCE_EXPORT_TABLE(F) \
    F(IOP_UINT32, IcarianEngine.Audio, AudioSourceInterop, GenerateAudioSource, \
    { \
        return Instance->GenerateAudioSource(a_transformAddr, a_clipAddr); \
    }, IOP_UINT32 a_transformAddr, IOP_UINT32 a_clipAddr) \
    F(void, IcarianEngine.Audio, AudioSourceInterop, DestroyAudioSource, \
    { \
        IPUSHDELETIONFUNC( \
        { \
            Instance->DestroyAudioSource(a_addr); \
        }, DeletionIndex_Update); \
    }, IOP_UINT32 a_addr) \
    F(void, IcarianEngine.Audio, AudioSourceInterop, PlayAudioSource, \
    { \
        Instance->PlayAudioSource(a_addr); \
    }, IOP_UINT32 a_addr) \
    F(AudioSourceBuffer, IcarianEngine.Audio, AudioSourceInterop, GetAudioSourceBuffer, \
    { \
        return Instance->GetAudioSourceBuffer(a_addr); \
    }, IOP_UINT32 a_addr) \
    F(void, IcarianEngine.Audio, AudioSourceInterop, SetAudioSourceBuffer, \
    { \
        Instance->SetAudioSourceBuffer(a_addr, a_buffer); \
    }, IOP_UINT32 a_addr, AudioSourceBuffer a_buffer) \

/// @endcond

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