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
