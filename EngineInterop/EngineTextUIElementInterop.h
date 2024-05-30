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
        const std::u32string text = Instance->GetTextElementText(a_addr); \
        return mono_string_new_utf32(mono_domain_get(), (mono_unichar4*)text.c_str(), (int32_t)text.size()); \
    }, IOP_UINT32 a_addr) \
    F(void, IcarianEngine.Rendering.UI, TextUIElementInterop, SetText, \
    { \
        mono_unichar4* str = mono_string_to_utf32(a_str); \
        IDEFER(mono_free(str)); \
        Instance->SetTextElementText(a_addr, (char32_t*)str); \
    }, IOP_UINT32 a_addr, IOP_STRING a_str) \

/// @endcond
