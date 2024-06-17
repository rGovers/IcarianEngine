#pragma once

#include "InteropTypes.h"

/// @file EnginePhysicsBodyInterop.h

/// @cond INTERNAL

#ifdef CUBE_LANGUAGE_CPP
#include "DeletionQueue.h"
#endif

#define ENGINE_PHYSICSBODY_EXPORT_TABLE(F) \
    F(IOP_UINT32, IcarianEngine.Physics, PhysicsBodyInterop, CreatePhysicsBody, \
    { \
        return Instance->CreatePhysicsBody(a_transformAddr, a_colliderAddr); \
    }, IOP_UINT32 a_transformAddr, IOP_UINT32 a_colliderAddr) \
    F(void, IcarianEngine.Physics, PhysicsBodyInterop, DestroyPhysicsBody, \
    { \
        IPUSHDELETIONFUNC( \
        { \
            Instance->DestroyPhysicsBody(a_addr); \
        }, DeletionIndex_Update); \
    }, IOP_UINT32 a_addr) \
    F(void, IcarianEngine.Physics, PhysicsBodyInterop, SetPosition, \
    { \
        Instance->SetPhysicsBodyPosition(a_addr, a_pos); \
    }, IOP_UINT32 a_addr, IOP_VEC3 a_pos) \
    F(IOP_VEC3, IcarianEngine.Physics, PhysicsBodyInterop, GetPosition, \
    { \
        return Instance->GetPhysicsBodyPosition(a_addr); \
    }, IOP_UINT32 a_addr) \
    F(void, IcarianEngine.Physics, PhysicsBodyInterop, SetRotation, \
    { \
        Instance->SetPhysicsBodyRotation(a_addr, a_rot); \
    }, IOP_UINT32 a_addr, IOP_QUAT a_rot) \
    F(IOP_QUAT, IcarianEngine.Physics, PhysicsBodyInterop, GetRotation, \
    { \
        return Instance->GetPhysicsBodyRotation(a_addr); \
    }, IOP_UINT32 a_addr) \


/// @endcond