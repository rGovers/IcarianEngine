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

IOP_PACKED IOP_CSPUBLIC struct LightShadowSplit
{
    /// <summary>
    /// The light view projection matrix for the split
    /// </summary>
    IOP_CSPUBLIC IOP_MAT4 LVP;
    /// <summary>
    /// The far plane for the split
    /// </summary>
    IOP_CSPUBLIC float Split;
};

#ifdef CUBE_LANGUAGE_CSHARP
}
#endif