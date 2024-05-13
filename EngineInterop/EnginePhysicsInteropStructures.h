#pragma once

#include "InteropTypes.h"

#ifdef CUBE_LANGUAGE_CSHARP
namespace IcarianEngine.Physics {
#endif

/// @file EnginePhysicsInteropStructures.h

/// @cond INTERNAL

IOP_PACKED IOP_CSINTERNAL struct RaycastResultBuffer
{
    IOP_CSPUBLIC IOP_VEC3 Position;
    IOP_CSPUBLIC IOP_UINT32 BodyAddr;
};

/// @endcond

#ifdef CUBE_LANGUAGE_CSHARP
}
#endif