// Icarian Engine - C# Game Engine
// 
// License at end of file.

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