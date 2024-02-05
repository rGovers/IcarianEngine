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
    IOP_ENUM_VALUE(ParticleEmitterType, Point) = 0,
    IOP_ENUM_VALUE(ParticleEmitterType, Sphere) = 1
};

/// @cond INTERNAL

IOP_CSINTERNAL enum IOP_ENUM_NAME(ParticleDisplayMode) : IOP_UINT16
{
    IOP_ENUM_VALUE(ParticleDisplayMode, Point) = 0,
    IOP_ENUM_VALUE(ParticleDisplayMode, Quad) = 1,
    IOP_ENUM_VALUE(ParticleDisplayMode, Mesh) = 2
};

IOP_PACKED IOP_CSINTERNAL struct ComputeParticleBuffer
{
    IOP_CSINTERNAL static IOP_CONSTEXPR IOP_UINT32 PlayBit = 0;
    IOP_CSINTERNAL static IOP_CONSTEXPR IOP_UINT32 PlayingBit = 1;
    IOP_CSINTERNAL static IOP_CONSTEXPR IOP_UINT32 DynamicBit = 2;
    IOP_CSINTERNAL static IOP_CONSTEXPR IOP_UINT32 RefreshBit = 3;

    IOP_CSPUBLIC IOP_ENUM_NAME(ParticleEmitterType) EmitterType;
    IOP_CSPUBLIC IOP_ENUM_NAME(ParticleDisplayMode) DisplayMode;
    IOP_CSPUBLIC float EmitterRadius;
    IOP_CSPUBLIC float EmitterRatio;
    IOP_CSPUBLIC IOP_VEC3 EmitterBounds;
    IOP_CSPUBLIC IOP_UINT32 MaxParticles;
    IOP_CSPUBLIC IOP_UINT8 Flags;
};

/// @endcond

#ifdef CUBE_LANGUAGE_CSHARP
}
#endif