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
    }, IOP_UINT32 a_addr)

/// @endcond
