// Icarian Engine - C# Game Engine
// 
// License at end of file.

#pragma once

#include "InteropTypes.h"

#ifdef CUBE_LANGUAGE_CSHARP
namespace IcarianEngine.Rendering {
#endif

/// @file EngineParticleSystemInteropStructures.h

/// <summary>
/// Particle system emitter type
/// </summary>
IOP_CSPUBLIC enum IOP_ENUM_NAME(ParticleEmitterType) : IOP_UINT16
{
    IOP_ENUM_VALUE(ParticleEmitterType, Null) = IOP_UINT16_MAX,
    IOP_ENUM_VALUE(ParticleEmitterType, Point) = 0
};

/// @cond INTERNAL

IOP_CSINTERNAL enum IOP_ENUM_NAME(ParticleDisplayMode) : IOP_UINT16
{
    IOP_ENUM_VALUE(ParticleDisplayMode, Quad) = 0,
    IOP_ENUM_VALUE(ParticleDisplayMode, Mesh) = 1
};

IOP_PACKED IOP_CSINTERNAL struct ComputeParticleBuffer
{
    IOP_CSINTERNAL static IOP_CONSTEXPR IOP_UINT32 PlayBit = 0;
    IOP_CSINTERNAL static IOP_CONSTEXPR IOP_UINT32 PlayingBit = 1;
    IOP_CSINTERNAL static IOP_CONSTEXPR IOP_UINT32 DynamicBit = 2;
    IOP_CSINTERNAL static IOP_CONSTEXPR IOP_UINT32 RefreshBit = 3;
    IOP_CSINTERNAL static IOP_CONSTEXPR IOP_UINT32 GraphicsRefreshBit = 4;
    IOP_CSINTERNAL static IOP_CONSTEXPR IOP_UINT32 BurstBit = 5;

    IOP_CSPUBLIC IOP_UINT32 TransformAddr;
    IOP_CSPUBLIC IOP_UINT32 RenderLayer;
    IOP_CSPUBLIC IOP_ENUM_NAME(ParticleEmitterType) EmitterType;
    IOP_CSPUBLIC IOP_ENUM_NAME(ParticleDisplayMode) DisplayMode;
    IOP_CSPUBLIC float EmitterRadius;
    IOP_CSPUBLIC float EmitterRatio;
    IOP_CSPUBLIC IOP_VEC3 EmitterBounds;
    IOP_CSPUBLIC IOP_VEC3 Gravity;
    IOP_CSPUBLIC IOP_VEC4 Colour;
    IOP_CSPUBLIC IOP_UINT32 MaxParticles;
    IOP_CSPUBLIC IOP_UINT8 Flags;
    IOP_POINTER(void*) Data;
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