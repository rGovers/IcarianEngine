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