#pragma once

#include "InteropTypes.h"

#ifdef CUBE_LANGUAGE_CSHARP
namespace IcarianEngine.Rendering.Lighting {
#endif

/// @file EngineLightInteropStructures.h

/// <summary>
/// Light type enumeration.
/// </summary>
IOP_CSPUBLIC enum IOP_ENUM_NAME(LightType) : IOP_UINT16
{
    IOP_ENUM_VALUE(LightType, Directional) = 0,
    IOP_ENUM_VALUE(LightType, Point) = 1,
    IOP_ENUM_VALUE(LightType, Spot) = 2,
    IOP_ENUM_VALUE(LightType, Ambient) = 3,
    IOP_ENUM_VALUE(LightType, End) = 4,
    IOP_ENUM_VALUE(LightType, ShadowEnd) = IOP_ENUM_VALUE(LightType, Ambient)
};

#ifdef CUBE_LANGUAGE_CSHARP
}
#endif