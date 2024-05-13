#pragma once

#include "InteropTypes.h"

/// @cond INTERNAL

#define ENGINE_NETWORKMANAGER_EXPORT_TABLE(F) \
    F(IOP_UINT32, IcarianEngine.Networking, NetworkManagerInterop, IsInitialized, \
    { \
        return (uint32_t)Instance->IsInitialized(); \
    }) \
    
/// @endcond
