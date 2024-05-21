#pragma once

#include "InteropTypes.h"

#ifdef CUBE_LANGUAGE_CSHARP
namespace IcarianEngine.Rendering {
#endif

/// @file EngineRenderCommandInteropStructures.h

/// <summary>
/// RenderTexutreBindMode enumeration
/// </summary>
IOP_CSPUBLIC enum IOP_ENUM_NAME(RenderTextureBindMode) : IOP_UINT8
{
    IOP_ENUM_VALUE(RenderTextureBindMode, Clear) = 0,
    IOP_ENUM_VALUE(RenderTextureBindMode, ClearColor) = 1,
    IOP_ENUM_VALUE(RenderTextureBindMode, NoClear) = 2
};

#ifdef CUBE_LANGUAGE_CSHARP
}
#endif