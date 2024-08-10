#pragma once

#include "InteropTypes.h"

#ifdef CUBE_LANGUAGE_CSHARP
namespace IcarianEngine.Physics {
#endif

/// @file EnginePhysicsBodyInteropStructures.h

/// @cond INTERNAL

IOP_PACKED IOP_CSINTERNAL struct CollisionDataBuffer
{
    IOP_CSPUBLIC IOP_UINT32 IsTrigger;
    IOP_CSPUBLIC IOP_UINT32 BodyAddrA;
    IOP_CSPUBLIC IOP_UINT32 BodyAddrB;
    IOP_CSPUBLIC IOP_VEC3 Position;
    IOP_CSPUBLIC IOP_VEC3 Normal;
    IOP_CSPUBLIC float Depth;
};

/// @endcond

#ifdef CUBE_LANGUAGE_CSHARP
}
#endif