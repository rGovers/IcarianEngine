#pragma once

#include "InteropTypes.h"

#ifdef CUBE_LANGUAGE_CSHARP
namespace IcarianEngine.Physics {
#endif

/// @file EngineRigidBodyInteropStructures.h

/// <summary>
/// Force mode enumeration
/// </summary>
IOP_CSPUBLIC enum IOP_ENUM_NAME(ForceMode) : IOP_UINT8
{
    IOP_ENUM_VALUE(ForceMode, Force) = 0,
    IOP_ENUM_VALUE(ForceMode, Acceleration) = 1,
    IOP_ENUM_VALUE(ForceMode, Impulse) = 2
};

#ifdef CUBE_LANGUAGE_CSHARP
}
#endif