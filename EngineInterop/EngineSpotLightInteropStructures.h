#pragma once

#include "InteropTypes.h"

#ifdef CUBE_LANGUAGE_CSHARP
namespace IcarianEngine.Rendering.Lighting {
#endif

/// @file EngineSpotLightInteropStructures.h

/// @cond INTERNAL

IOP_PACKED IOP_CSINTERNAL struct SpotLightBuffer
{
    IOP_CSPUBLIC IOP_UINT32 TransformAddr;
    IOP_CSPUBLIC IOP_UINT32 RenderLayer;
    IOP_CSPUBLIC IOP_VEC4 Color;
    IOP_CSPUBLIC float Intensity;
    IOP_CSPUBLIC IOP_VEC2 CutoffAngle;
    IOP_CSPUBLIC float Radius;
    IOP_POINTER(void*) Data;
};

/// @endcond

#ifdef CUBE_LANGUAGE_CSHARP
}
#endif