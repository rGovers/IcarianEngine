#pragma once

#include "InteropTypes.h"

/// @file EngineIcarianAssemblyInterop.h

/// @cond INTERNAL

#define ENGINE_ICARIANASSEMBLY_EXPORT_TABLE(F) \
    F(void, IcarianEngine.Mod, IcarianAssemblyInterop, LoadNativeAssembly, \
    { \
        char* str = mono_string_to_utf8(a_path); \
        IDEFER(mono_free(str)); \
        RuntimeManager::PushDLLPath(str); \
    }, IOP_STRING a_path) \

/// @endcond