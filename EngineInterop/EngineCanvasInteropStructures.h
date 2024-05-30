#pragma once

#include "InteropTypes.h"

/// @file EngineCanvasInteropStructures.h

#ifdef CUBE_LANGUAGE_CSHARP
namespace IcarianEngine.Rendering.UI {
#endif

/// @cond INTERNAL

IOP_PACKED IOP_CSINTERNAL struct CanvasBuffer
{
    IOP_CSPUBLIC static IOP_CONSTEXPR IOP_UINT32 CaptureInputBit = 0;

    IOP_CSPUBLIC IOP_VEC2 ReferenceResolution;
    IOP_UINT32 ChildCount;
    IOP_POINTER(IOP_UINT32*) ChildElements;
    IOP_CSPUBLIC IOP_UINT8 Flags;
};

/// @endcond

#ifdef CUBE_LANGUAGE_CSHARP
}
#endif