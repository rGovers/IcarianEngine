#pragma once

#include "InteropTypes.h"

/// @file EngineUIElementInteropStuctures.h

#ifdef CUBE_LANGUAGE_CSHARP
namespace IcarianEngine.Rendering.UI {
#endif

/// <summary>
/// ElementState enumeration
/// </summary>
IOP_CSPUBLIC enum IOP_ENUM_NAME(ElementState) : IOP_UINT16
{
    IOP_ENUM_VALUE(ElementState, Normal) = 0,
    IOP_ENUM_VALUE(ElementState, Hovered) = 1,
    IOP_ENUM_VALUE(ElementState, Pressed) = 2,
    IOP_ENUM_VALUE(ElementState, Released) = 3
};

/// <summary>
// UIXAnchor enumeration
/// </summary>
IOP_CSPUBLIC enum IOP_ENUM_NAME(UIXAnchor) : IOP_UINT16
{
    IOP_ENUM_VALUE(UIXAnchor, Left) = 0,
    IOP_ENUM_VALUE(UIXAnchor, Middle) = 1,
    IOP_ENUM_VALUE(UIXAnchor, Right) = 2,
    IOP_ENUM_VALUE(UIXAnchor, Stretch) = 3
};

/// <summary>
// UIYAnchor enumeration
/// </summary>
IOP_CSPUBLIC enum IOP_ENUM_NAME(UIYAnchor) : IOP_UINT16
{
    IOP_ENUM_VALUE(UIYAnchor, Top) = 0,
    IOP_ENUM_VALUE(UIYAnchor, Middle) = 1,
    IOP_ENUM_VALUE(UIYAnchor, Bottom) = 2,
    IOP_ENUM_VALUE(UIYAnchor, Stretch) = 3
};

#ifdef  CUBE_LANGUAGE_CSHARP
}
#endif