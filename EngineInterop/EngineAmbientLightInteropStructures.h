#pragma once

#include "InteropTypes.h"

#ifdef CUBE_LANGUAGE_CSHARP
namespace IcarianEngine.Rendering.Lighting {
#endif

/// @file EngineAmbientLightInteropStructures.h

/// @cond INTERNAL

IOP_PACKED IOP_CSINTERNAL struct AmbientLightBuffer
{
    IOP_CSPUBLIC IOP_UINT32 RenderLayer;
    IOP_CSPUBLIC IOP_VEC4 Color;
    IOP_CSPUBLIC float Intensity;
    IOP_POINTER(void*) Data;
};

/// @endcond

#ifdef CUBE_LANGUAGE_CSHARP
}
#endif