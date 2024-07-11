#pragma once

#include "InteropTypes.h"

#ifdef CUBE_LANGUAGE_CSHARP
namespace IcarianEngine {
#endif

/// @file EngineApplicationInteropStructures.h

IOP_PACKED IOP_CSPUBLIC struct Monitor
{
    IOP_CSPUBLIC IOP_UINT32 Index;
    IOP_CSPUBLIC IOP_STRING Name;
    IOP_CSPUBLIC IOP_UINT32 Width;
    IOP_CSPUBLIC IOP_UINT32 Height;
    IOP_POINTER(void*) Handle;
};

#ifdef CUBE_LANGUAGE_CSHARP
}
#endif