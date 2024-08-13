// Icarian Engine - C# Game Engine
// 
// License at end of file.

#pragma once

#include "InteropTypes.h"

#ifdef CUBE_LANGUAGE_CSHARP
namespace IcarianEngine {
#endif

/// @file EngineApplicationInteropStructures.h

IOP_PACKED IOP_CSPUBLIC struct Monitor
{
    /// <summary>
    /// The index of the Monitor
    /// </summary>
    IOP_CSPUBLIC IOP_UINT32 Index;
    /// <summary>
    /// The name of the Monitor
    /// </summary>
    IOP_CSPUBLIC IOP_STRING Name;
    /// <summary>
    /// The width of the Monitor in pixels
    /// </summary>
    IOP_CSPUBLIC IOP_UINT32 Width;
    /// <summary>
    /// The height of the Monitor in pixels
    /// </summary>
    IOP_CSPUBLIC IOP_UINT32 Height;
    IOP_POINTER(void*) Handle;
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