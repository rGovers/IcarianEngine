// Icarian Engine - C# Game Engine
// 
// License at end of file.

#pragma once

#include "InteropTypes.h"

/// @file EngineImageUIElementInterop.h

/// @cond INTERNAL

#define ENGINE_IMAGEUIELEMENT_EXPORT_TABLE(F) \
    F(IOP_UINT32, IcarianEngine.Rendering.UI, ImageUIElementInterop, CreateImageElement, \
    { \
        return Instance->CreateImageElement(); \
    }) \
    F(IOP_UINT32, IcarianEngine.Rendering.UI, ImageUIElementInterop, GetSampler, \
    { \
        return Instance->GetImageElementSampler(a_addr); \
    }, IOP_UINT32 a_addr) \
    F(void, IcarianEngine.Rendering.UI, ImageUIElementInterop, SetSampler, \
    { \
        Instance->SetImageElementSampler(a_addr, a_samplerAddr); \
    }, IOP_UINT32 a_addr, IOP_UINT32 a_samplerAddr)

/// @endcond

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