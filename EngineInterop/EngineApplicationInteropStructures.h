#pragma once

#include "InteropTypes.h"

#ifdef CUBE_LANGUAGE_CSHARP
namespace IcarianEngine {
#endif

/// @file EngineApplicationInteropStructures.h

IOP_PACKED IOP_CSPUBLIC struct Monitor
{
    /// <summary>
    /// The index of the Monitor
    /// </summary>
    IOP_CSPUBLIC IOP_UINT32 Index;
    /// <summary>
    /// The name of the Monitor
    /// </summary>
    IOP_CSPUBLIC IOP_STRING Name;
    /// <summary>
    /// The width of the Monitor in pixels
    /// </summary>
    IOP_CSPUBLIC IOP_UINT32 Width;
    /// <summary>
    /// The height of the Monitor in pixels
    /// </summary>
    IOP_CSPUBLIC IOP_UINT32 Height;
    IOP_POINTER(void*) Handle;
};

#ifdef CUBE_LANGUAGE_CSHARP
}
#endif