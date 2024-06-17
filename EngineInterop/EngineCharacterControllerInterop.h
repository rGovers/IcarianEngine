#pragma once

#include "InteropTypes.h"

/// @file EngineCharacterControllerInterop.h

/// @cond INTERNAL

#ifdef CUBE_LANGUAGE_CPP
#include "DeletionQueue.h"
#endif

#define ENGINE_CHARACTERCONTROLLER_EXPORT_TABLE(F) \
    F(IOP_UINT32, IcarianEngine.Physics, CharacterControllerInterop, CreateCharacter, \
    { \
        return Instance->CreateCharacterController(a_transformAddr, a_colliderAddr, a_up, a_slopeAngle, a_mass); \
    }, IOP_UINT32 a_transformAddr, IOP_UINT32 a_colliderAddr, IOP_VEC3 a_up, float a_slopeAngle, float a_mass) \
    F(void, IcarianEngine.Physics, CharacterControllerInterop, DestroyCharacter, \
    { \
        IPUSHDELETIONFUNC( \
        { \
            Instance->DestroyCharacterController(a_addr); \
        }, DeletionIndex_Update); \
    }, IOP_UINT32 a_addr) \
    F(IOP_VEC3, IcarianEngine.Physics, CharacterControllerInterop, GetVelocity, \
    { \
        return Instance->GetCharacterControllerVelocity(a_addr); \
    }, IOP_UINT32 a_addr) \
    F(void, IcarianEngine.Physics, CharacterControllerInterop, SetVelocity, \
    { \
        Instance->SetCharacterControllerVelocity(a_addr, a_velocity); \
    }, IOP_UINT32 a_addr, IOP_VEC3 a_velocity)\
    

/// @endcond