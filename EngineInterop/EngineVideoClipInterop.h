#pragma once

#include "InteropTypes.h"

/// @file EngineVideoClipInterop.h

#ifdef CUBE_LANGUAGE_CPP
#include "DeletionQueue.h"
#endif

/// @cond INTERNAL

#define ENGINE_VIDEOCLIP_EXPORT_TABLE(F) \
    F(IOP_UINT32, IcarianEngine.Rendering.Video, VideoClipInterop, GenerateFromFile, \
    { \
        char* str = mono_string_to_utf8(a_path); \
        IDEFER(mono_free(str)); \
        return Instance->GenerateVideoClipFromFile(str); \
    }, IOP_STRING a_path) \
    F(void, IcarianEngine.Rendering.Video, VideoClipInterop, DestroyClip, \
    { \
        IDUALDELETIONFUNC( \
        { \
            Instance->DestroyVideoClip(a_addr); \
        }); \
    }, IOP_UINT32 a_addr) \

/// @endcond
