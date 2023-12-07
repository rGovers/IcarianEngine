#pragma once

#include "InteropTypes.h"

#ifdef CUBE_LANGUAGE_CSHARP
namespace IcarianEngine.Rendering.Animation {
#endif

/// @cond INTERNAL

IOP_PACKED struct DAERAnimationFrame
{
    IOP_CSPUBLIC float Time;
    IOP_CSPUBLIC IOP_ARRAY(float[]) Transform;
};
IOP_PACKED struct DAERAnimation
{
    IOP_CSPUBLIC IOP_STRING Name;
    IOP_CSPUBLIC IOP_ARRAY(DAERAnimationFrame[]) Frames;
};

IOP_PACKED struct FBXRAnimationFrame
{
    IOP_CSPUBLIC float Time;
    IOP_CSPUBLIC IOP_VEC4 Data;
};
IOP_PACKED struct FBXRAnimation
{
    IOP_CSPUBLIC IOP_STRING Name;
    IOP_CSPUBLIC IOP_STRING Target;
    IOP_CSPUBLIC IOP_ARRAY(FBXRAnimationFrame[]) Frames;
};

IOP_PACKED struct GLTFRAnimationFrame
{
    IOP_CSPUBLIC float Time;
    IOP_CSPUBLIC IOP_VEC4 Data;
};
IOP_PACKED struct GLTFRAnimation
{
    IOP_CSPUBLIC IOP_STRING Name;
    IOP_CSPUBLIC IOP_STRING Target;
    IOP_CSPUBLIC IOP_ARRAY(GLTFRAnimationFrame[]) Frames;
};

/// @endcond

#ifdef CUBE_LANGUAGE_CSHARP
}
#endif