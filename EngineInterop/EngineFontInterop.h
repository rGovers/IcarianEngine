// Icarian Engine - C# Game Engine
// 
// License at end of file.

#pragma once

#include "InteropTypes.h"

/// @file EngineFontInterop.h

#ifdef CUBE_LANGUAGE_CPP
#include "DeletionQueue.h"
#endif

/// @cond INTERNAL

#define ENGINE_FONT_EXPORT_TABLE(F) \
    F(IOP_UINT32, IcarianEngine.Rendering.UI, FontInterop, GenerateFont, \
    { \
        char* str = mono_string_to_utf8(a_path); \
        IDEFER(mono_free(str)); \
        return Instance->GenerateFont(str); \
    }, IOP_STRING a_path) \
    F(void, IcarianEngine.Rendering.UI, FontInterop, DestroyFont, \
    { \
        IDUALDELETIONFUNC( \
        { \
            Instance->DestroyFont(a_addr); \
        }); \
    }, IOP_UINT32 a_addr) \
    F(IOP_UINT32, IcarianEngine.Rendering.UI, FontInterop, GenerateModel, \
    { \
        mono_unichar4* str = mono_string_to_utf32(a_str); \
        IDEFER(mono_free(str)); \
        return Instance->GenerateModelFromString(a_addr, std::u32string_view((char32_t*)str), a_fontSize, a_scale, a_depth); \
    }, IOP_UINT32 a_addr, IOP_STRING a_str, float a_fontSize, float a_scale, float a_depth) \


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