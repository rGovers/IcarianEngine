// Icarian Engine - C# Game Engine
// 
// License at end of file.

#pragma once

#include "InteropTypes.h"

#ifdef CUBE_LANGUAGE_CSHARP
namespace IcarianEngine.Rendering {
#endif

IOP_CSPUBLIC enum IOP_ENUM_NAME(TextureFilter) : IOP_UINT16
{
    IOP_ENUM_VALUE(TextureFilter, Nearest) = 0,
    IOP_ENUM_VALUE(TextureFilter, Linear) = 1
};

IOP_CSPUBLIC enum IOP_ENUM_NAME(TextureAddress) : IOP_UINT16
{
    IOP_ENUM_VALUE(TextureAddress, Repeat) = 0,
    IOP_ENUM_VALUE(TextureAddress, MirroredRepeat) = 1,
    IOP_ENUM_VALUE(TextureAddress, ClampToEdge) = 2
};

/// @cond INTERNAL

IOP_CSINTERNAL enum IOP_ENUM_NAME(TextureMode) : IOP_UINT16
{
    IOP_ENUM_VALUE(TextureMode, Null) = IOP_UINT16_MAX,
    IOP_ENUM_VALUE(TextureMode, Texture) = 0,
    IOP_ENUM_VALUE(TextureMode, RenderTexture) = 1,
    IOP_ENUM_VALUE(TextureMode, RenderTextureDepth) = 2,
    IOP_ENUM_VALUE(TextureMode, DepthRenderTexture) = 3,
    IOP_ENUM_VALUE(TextureMode, DepthCubeRenderTexture) = 4
};

IOP_PACKED IOP_CSINTERNAL struct TextureSamplerBuffer
{
    IOP_UINT32 Addr;
    IOP_UINT32 Slot;
    IOP_ENUM_NAME(TextureMode) TextureMode;
    IOP_ENUM_NAME(TextureFilter) FilterMode;
    IOP_ENUM_NAME(TextureAddress) AddressMode;
    IOP_POINTER(void*) Data;
};

/// @endcond

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