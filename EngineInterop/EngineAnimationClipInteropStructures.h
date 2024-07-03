#pragma once

#include "InteropTypes.h"

#ifdef CUBE_LANGUAGE_CSHARP
namespace IcarianEngine.Rendering.Animation {
#endif

/// @file EngineAnimationClipInteropStructures.h

/// @cond INTERNAL

IOP_PACKED IOP_CSINTERNAL struct AnimationFrameExternal
{
    IOP_CSPUBLIC float Time;
    IOP_CSPUBLIC IOP_VEC4 Data;
};

IOP_PACKED IOP_CSINTERNAL struct AnimationDataExternal
{
    IOP_CSPUBLIC IOP_STRING Name;
    IOP_CSPUBLIC IOP_STRING Target;
    IOP_CSPUBLIC IOP_ARRAY(AnimationFrameExternal[]) Frames;
};

/// @endcond

#ifdef CUBE_LANGUAGE_CSHARP
}
#endif