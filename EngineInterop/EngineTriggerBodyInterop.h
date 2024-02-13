#pragma once

#include "InteropTypes.h"

/// @file EngineTriggerBodyInterop.h

/// @cond INTERNAL

#define ENGINE_TRIGGERBODY_EXPORT_TABLE(F) \
    F(IOP_UINT32, IcarianEngine.Physics, TriggerBodyInterop, CreateTriggerBody, \
    { \
        return Instance->CreateTriggerBody(a_transformAddr, a_colliderAddr); \
    }, IOP_UINT32 a_transformAddr, IOP_UINT32 a_colliderAddr) \


/// @endcond