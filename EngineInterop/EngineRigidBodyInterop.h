// Icarian Engine - C# Game Engine
// 
// License at end of file.

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

// MIT License
// 
// Copyright (c) 2024 River Govers
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.