#pragma once

#include "InteropTypes.h"

#ifdef CUBE_LANGUAGE_CSHARP
namespace IcarianEngine.Rendering.Animation {
#endif

/// @cond INTERNAL

IOP_PACKED IOP_CSINTERNAL struct RuntimeImportBoneData
{
    IOP_CSPUBLIC IOP_ARRAY(string[]) Names;
    IOP_CSPUBLIC IOP_ARRAY(uint[]) Parents;
    IOP_CSPUBLIC IOP_ARRAY(float[][]) BindPoses;
};

/// @endcond

#ifdef  CUBE_LANGUAGE_CSHARP
}
#endif