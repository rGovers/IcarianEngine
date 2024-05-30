#pragma once

#include "InteropTypes.h"

/// @file EngineCanvasInterop.h

/// @cond INTERNAL

#ifdef CUBE_LANGUAGE_CPP
#include "DeletionQueue.h"
#endif

#define ENGINE_CANVAS_EXPORT_TABLE(F) \
    F(IOP_UINT32, IcarianEngine.Rendering.UI, CanvasInterop, CreateCanvas, \
    { \
        return Instance->CreateCanvas(a_refRes); \
    }, IOP_VEC2 a_refRes) \
    F(void, IcarianEngine.Rendering.UI, CanvasInterop, DestroyCanvas, \
    { \
        IDUALDELETIONFUNC( \
        { \
            Instance->DestroyCanvas(a_addr); \
        }); \
    }, IOP_UINT32 a_addr) \
    F(CanvasBuffer, IcarianEngine.Rendering.UI, CanvasInterop, GetBuffer, \
    { \
        return UIControl::GetCanvas(a_addr); \
    }, IOP_UINT32 a_addr) \
    F(void, IcarianEngine.Rendering.UI, CanvasInterop, SetBuffer, \
    { \
        UIControl::SetCanvas(a_addr, a_buffer); \
    }, IOP_UINT32 a_addr, CanvasBuffer a_buffer) \
    F(void, IcarianEngine.Rendering.UI, CanvasInterop, AddChildElement, \
    { \
        Instance->AddCanvasChild(a_addr, a_childAddr); \
    }, IOP_UINT32 a_addr, IOP_UINT32 a_childAddr) \
    F(void, IcarianEngine.Rendering.UI, CanvasInterop, RemoveChildElement, \
    { \
        Instance->RemoveCanvasChild(a_addr, a_childAddr); \
    }, IOP_UINT32 a_addr, IOP_UINT32 a_childAddr) \
    F(IOP_ARRAY(IOP_UINT32[]), IcarianEngine.Rendering.UI, CanvasInterop, GetChildren, \
    { \
        uint32_t count; \
        const uint32_t* children = Instance->GetCanvasChildren(a_addr, &count); \
        MonoArray* arr = mono_array_new(mono_domain_get(), mono_get_uint32_class(), count); \
        for (uint32_t i = 0; i < count; ++i) \
        { \
            mono_array_set(arr, uint32_t, i, children[i]); \
        } \
        return arr; \
    }, IOP_UINT32 a_addr) \

/// @endcond
