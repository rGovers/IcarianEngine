#pragma once

#include "InteropTypes.h"

/// @file EngineRigidBodyInterop.h

/// @cond INTERNAL

#define ENGINE_RIGIDBODY_EXPORT_TABLE(F) \
    F(IOP_UINT32, IcarianEngine.Physics, RigidBodyInterop, CreateRigidBody, \
    { \
        return Instance->CreateRigidBody(a_transformAddr, a_colliderAddr, a_layer, a_mass); \
    }, IOP_UINT32 a_transformAddr, IOP_UINT32 a_colliderAddr, IOP_UINT32 a_layer, float a_mass) \
    F(void, IcarianEngine.Physics, RigidBodyInterop, SetGravityFactor, \
    { \
        Instance->SetRigidBodyGravityFactor(a_addr, a_factor); \
    }, IOP_UINT32 a_addr, float a_factor) \
    F(float, IcarianEngine.Physics, RigidBodyInterop, GetGravityFactor, \
    { \
        return Instance->GetRigidBodyGravityFactor(a_addr); \
    }, IOP_UINT32 a_addr) \
    F(IOP_VEC3, IcarianEngine.Physics, RigidBodyInterop, GetVelocity, \
    { \
        return Instance->GetRigidBodyVelocity(a_addr); \
    }, IOP_UINT32 a_addr) \
    F(void, IcarianEngine.Physics, RigidBodyInterop, SetVelocity, \
    { \
        Instance->SetRigidBodyVelocity(a_addr, a_velocity); \
    }, IOP_UINT32 a_addr, IOP_VEC3 a_velocity) \
    F(IOP_VEC3, IcarianEngine.Physics, RigidBodyInterop, GetAngularVelocity, \
    { \
        return Instance->GetRigidBodyAngularVelocity(a_addr); \
    }, IOP_UINT32 a_addr) \
    F(void, IcarianEngine.Physics, RigidBodyInterop, SetAngularVelocity, \
    { \
        Instance->SetRigidBodyAngularVelocity(a_addr, a_velocity); \
    }, IOP_UINT32 a_addr, IOP_VEC3 a_velocity) \
    F(void, IcarianEngine.Physics, RigidBodyInterop, AddForce, \
    { \
        Instance->RigidBodyAddForce(a_addr, a_force, (e_ForceMode)a_mode); \
    }, IOP_UINT32 a_addr, IOP_VEC3 a_force, IOP_UINT32 a_mode) \
    F(void, IcarianEngine.Physics, RigidBodyInterop, AddTorque, \
    { \
        Instance->RigidBodyAddTorque(a_addr, a_force, (e_ForceMode)a_mode); \
    }, IOP_UINT32 a_addr, IOP_VEC3 a_force, IOP_UINT32 a_mode) \

/// @endcond