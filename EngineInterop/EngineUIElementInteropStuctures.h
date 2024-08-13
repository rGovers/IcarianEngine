// Icarian Engine - C# Game Engine
// 
// License at end of file.

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

// MIT License
// 
// Copyright (c) 2024 River Govers
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.