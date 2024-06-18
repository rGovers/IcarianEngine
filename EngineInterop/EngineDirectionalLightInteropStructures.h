#pragma once

#include "InteropTypes.h"

#ifdef CUBE_LANGUAGE_CSHARP
namespace IcarianEngine.Rendering.Lighting {
#endif

/// @file EngineDirectionalLightInteropStructures.h

/// @cond INTERNAL

IOP_PACKED IOP_CSINTERNAL struct DirectionalLightBuffer
{
    IOP_CSPUBLIC IOP_UINT32 TransformAddr;
    IOP_CSPUBLIC IOP_UINT32 RenderLayer;
    IOP_CSPUBLIC IOP_VEC4 Color;
    IOP_CSPUBLIC IOP_VEC2 ShadowBias;
    IOP_CSPUBLIC float Intensity;
    IOP_POINTER(void*) Data;
};

/// @endcond

#ifdef CUBE_LANGUAGE_CSHARP
}
#endif