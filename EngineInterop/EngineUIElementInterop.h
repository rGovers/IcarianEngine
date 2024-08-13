// Icarian Engine - C# Game Engine
// 
// License at end of file.

#pragma once

#include "InteropTypes.h"

/// @file EngineUIElementInterop.h

#ifdef CUBE_LANGUAGE_CPP
#include "DeletionQueue.h"
#endif

/// @cond INTERNAL

#define ENGINE_UIELEMENT_EXPORT_TABLE(F) \
    F(IOP_UINT32, IcarianEngine.Rendering.UI, UIElementInterop, CreateUIElement, \
    { \
        return Instance->CreateUIElement(); \
    }) \
    F(void, IcarianEngine.Rendering.UI, UIElementInterop, DestroyUIElement, \
    { \
        IDUALDELETIONFUNC( \
        { \
            Instance->DestroyUIElement(a_addr); \
        }); \
    }, IOP_UINT32 a_addr) \
    F(void, IcarianEngine.Rendering.UI, UIElementInterop, AddChildElement, \
    { \
        Instance->AddElementChild(a_addr, a_childAddr); \
    }, IOP_UINT32 a_addr, IOP_UINT32 a_childAddr) \
    F(void, IcarianEngine.Rendering.UI, UIElementInterop, RemoveChildElement, \
    { \
        Instance->RemoveElementChild(a_addr, a_childAddr); \
    }, IOP_UINT32 a_addr, IOP_UINT32 a_childAddr) \
    F(void, IcarianEngine.Rendering.UI, UIElementInterop, SetPosition, \
    { \
        Instance->SetElementPosition(a_addr, a_pos); \
    }, IOP_UINT32 a_addr, IOP_VEC2 a_pos) \
    F(void, IcarianEngine.Rendering.UI, UIElementInterop, SetSize, \
    { \
        Instance->SetElementSize(a_addr, a_size); \
    }, IOP_UINT32 a_addr, IOP_VEC2 a_size) \
    F(IOP_VEC4, IcarianEngine.Rendering.UI, UIElementInterop, GetColor, \
    { \
        return Instance->GetElementColor(a_addr); \
    }, IOP_UINT32 a_addr) \
    F(void, IcarianEngine.Rendering.UI, UIElementInterop, SetColor, \
    { \
        Instance->SetElementColor(a_addr, a_color); \
    }, IOP_UINT32 a_addr, IOP_VEC4 a_color) \
    F(IOP_UINT32, IcarianEngine.Rendering.UI, UIElementInterop, GetElementXAnchor, \
    { \
        return (uint32_t)Instance->GetElementXAnchor(a_addr); \
    }, IOP_UINT32 a_addr) \
    F(void, IcarianEngine.Rendering.UI, UIElementInterop, SetElementXAnchor, \
    { \
        Instance->SetElementXAnchor(a_addr, (e_UIXAnchor)a_anchor); \
    }, IOP_UINT32 a_addr, IOP_UINT32 a_anchor) \
    F(IOP_UINT32, IcarianEngine.Rendering.UI, UIElementInterop, GetElementYAnchor, \
    { \
        return (uint32_t)Instance->GetElementYAnchor(a_addr); \
    }, IOP_UINT32 a_addr) \
    F(void, IcarianEngine.Rendering.UI, UIElementInterop, SetElementYAnchor, \
    { \
        Instance->SetElementYAnchor(a_addr, (e_UIYAnchor)a_anchor); \
    }, IOP_UINT32 a_addr, IOP_UINT32 a_anchor) \
    F(IOP_UINT32, IcarianEngine.Rendering.UI, UIElementInterop, GetElementState, \
    { \
        return (uint32_t)Instance->GetElementState(a_addr); \
    }, IOP_UINT32 a_addr) \
    F(IOP_ARRAY(IOP_UINT32[]), IcarianEngine.Rendering.UI, UIElementInterop, GetChildren, \
    { \
        uint32_t count; \
        const uint32_t* children = Instance->GetElementChildren(a_addr, &count); \
        MonoArray* arr = mono_array_new(mono_domain_get(), mono_get_uint32_class(), count); \
        for (uint32_t i = 0; i < count; ++i) \
        { \
            mono_array_set(arr, uint32_t, i, children[i]); \
        } \
        return arr; \
    }, IOP_UINT32 a_addr) \
    \
    \
    F(void, IcarianEngine.Rendering.UI, UIElementInterop, GetPosition, \
    { \
        *a_ptr = Instance->GetElementPosition(a_addr); \
    }, IOP_UINT32 a_addr, IOP_POINTER(IOP_VEC2*) a_ptr) \
    F(void, IcarianEngine.Rendering.UI, UIElementInterop, GetSize, \
    { \
        *a_ptr = Instance->GetElementSize(a_addr); \
    }, IOP_UINT32 a_addr, IOP_POINTER(IOP_VEC2*) a_ptr) \

// For some reason GetPosition and GetSize require usage of a pointer instead of a return value on Windows 10
// Stack gets corrupted if I do not
// This is very bad as it manages to somehow avoid exceptions and faults somehow and continue running instead of just crashing
// Seems to be OS version specific for some reason which is very odd
// Not gonna question it, probably me fucking something up
// If the reason can be nailed down will change it back to a return value
// Apparently just a thing C# likes to do with Interop according to people more knowledgable on the subject. Just have to massage the code until it works
// Ah blackboxes gotta love em
// NOTE: That it is intermittent so need to run several 100 times to trigger it if you want to debug it

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