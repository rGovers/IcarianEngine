// Icarian Engine - C# Game Engine
//
// License at end of file.

#pragma once

#include "InteropTypes.h"

/// @file EngineTextUIElementInterop.h

/// @cond INTERNAL

#define ENGINE_TEXTUIELEMENT_EXPORT_TABLE(F) \
    F(IOP_UINT32, IcarianEngine.Rendering.UI, TextUIElementInterop, CreateTextElement, \
    { \
        return Instance->CreateTextElement(); \
    }) \
    F(float, IcarianEngine.Rendering.UI, TextUIElementInterop, GetFontSize, \
    { \
        return Instance->GetTextElementFontSize(a_addr); \
    }, IOP_UINT32 a_addr) \
    F(void, IcarianEngine.Rendering.UI, TextUIElementInterop, SetFontSize, \
    { \
        Instance->SetTextElementFontSize(a_addr, a_size); \
    }, IOP_UINT32 a_addr, float a_size) \
    F(IOP_UINT32, IcarianEngine.Rendering.UI, TextUIElementInterop, GetFont, \
    { \
        return Instance->GetTextElementFont(a_addr); \
    }, IOP_UINT32 a_addr) \
    F(void, IcarianEngine.Rendering.UI, TextUIElementInterop, SetFont, \
    { \
        Instance->SetTextElementFont(a_addr, a_fontAddr); \
    }, IOP_UINT32 a_addr, IOP_UINT32 a_fontAddr) \
    F(IOP_STRING, IcarianEngine.Rendering.UI, TextUIElementInterop, GetText, \
    { \
        const IcarianCore::COWU32String text = Instance->GetTextElementText(a_addr); \
        MonoDomain* domain = mono_domain_get(); \
        return mono_string_new_utf32(domain, (mono_unichar4*)text.Data(), (int32_t)text.Length()); \
    }, IOP_UINT32 a_addr) \
    F(void, IcarianEngine.Rendering.UI, TextUIElementInterop, SetText, \
    { \
        mono_unichar4* str = mono_string_to_utf32(a_str); \
        IDEFER(mono_free(str)); \
        Instance->SetTextElementText(a_addr, (IcarianCore::CharU32*)str); \
    }, IOP_UINT32 a_addr, IOP_STRING a_str) \

/// @endcond

// MIT License
//
// Copyright (c) 2026 River Govers
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
