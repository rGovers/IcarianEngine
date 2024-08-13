// Icarian Engine - C# Game Engine
// 
// License at end of file.

#pragma once

#include "InteropTypes.h"

/// @file EnginePhysicsInterop.h

/// @cond INTERNAL

#define ENGINE_PHYSICS_EXPORT_TABLE(F) \
    F(IOP_VEC3, IcarianEngine.Physics, PhysicsInterop, GetGravity, \
    { \
        return Instance->GetGravity(); \
    }) \
    F(void, IcarianEngine.Physics, PhysicsInterop, SetGravity, \
    { \
        Instance->SetGravity(a_gravity); \
    }, IOP_VEC3 a_gravity) \
    F(IOP_UINT32, IcarianEngine.Physics, PhysicsInterop, GetObjectLayerCollision, \
    { \
        return (uint32_t)Instance->GetObjectLayerCollision(a_layerA, a_layerB); \
    }, IOP_UINT32 a_layerA, IOP_UINT32 a_layerB) \
    F(void, IcarianEngine.Physics, PhysicsInterop, SetObjectLayerCollision, \
    { \
        Instance->SetObjectLayerCollision(a_layerA, a_layerB, (bool)a_state); \
    }, IOP_UINT32 a_layerA, IOP_UINT32 a_layerB, IOP_UINT32 a_state) \
    F(IOP_ARRAY(RaycastResultBuffer[]), IcarianEngine.Physics, PhysicsInterop, Raycast, \
    { \
        uint32_t resultCount; \
        const RaycastResultBuffer* results = Instance->Raycast(a_pos, a_dir, &resultCount); \
        if (results != nullptr) \
        { \
            IDEFER(delete[] results); \
            MonoClass* klass = RuntimeManager::GetClass("IcarianEngine.Physics", "RaycastResultBuffer"); \
            MonoArray* arr = mono_array_new(RuntimeManager::GetDomain(), klass, (uintptr_t)resultCount); \
            for (uint32_t i = 0; i < resultCount; ++i) \
            { \
                mono_array_set(arr, RaycastResultBuffer, i, results[i]); \
            } \
            return arr; \
        } \
        return NULL; \
    }, IOP_VEC3 a_pos, IOP_VEC3 a_dir) \
    F(IOP_ARRAY(uint[]), IcarianEngine.Physics, PhysicsInterop, SphereCollision, \
    { \
        uint32_t resultCount; \
        const uint32_t* data = Instance->SphereCollision(a_pos, a_radius, &resultCount); \
        if (data != nullptr) \
        { \
            IDEFER(delete[] data); \
            MonoArray* arr = mono_array_new(RuntimeManager::GetDomain(), mono_get_uint32_class(), (uintptr_t)resultCount); \
            for (uint32_t i = 0; i < resultCount; ++i) \
            { \
                mono_array_set(arr, uint32_t, i, data[i]); \
            } \
            return arr; \
        } \
        return NULL; \
    }, IOP_VEC3 a_pos, float a_radius) \
    F(IOP_ARRAY(uint[]), IcarianEngine.Physics, PhysicsInterop, BoxCollision, \
    { \
        glm::mat4 t; \
        float* tDat = (float*)&t; \
        for (int i = 0; i < 16; ++i) \
        { \
            tDat[i] = mono_array_get(a_tranform, float, i); \
        } \
        uint32_t resultCount; \
        const uint32_t* data = Instance->BoxCollision(t, a_extents, &resultCount); \
        if (data != nullptr) \
        { \
            IDEFER(delete[] data); \
            MonoArray* arr = mono_array_new(RuntimeManager::GetDomain(), mono_get_uint32_class(), (uintptr_t)resultCount); \
            for (uint32_t i = 0; i < resultCount; ++i) \
            { \
                mono_array_set(arr, uint32_t, i, data[i]); \
            } \
            return arr; \
        } \
        return NULL; \
    }, IOP_ARRAY(float[]) a_tranform, IOP_VEC3 a_extents) \
    F(IOP_ARRAY(uint[]), IcarianEngine.Physics, PhysicsInterop, AABBCollision, \
    { \
        uint32_t resultCount; \
        const uint32_t* data = Instance->AABBCollision(a_min, a_max, &resultCount); \
        if (data != nullptr) \
        { \
            IDEFER(delete[] data); \
            MonoArray* arr = mono_array_new(RuntimeManager::GetDomain(), mono_get_uint32_class(), (uintptr_t)resultCount); \
            for (uint32_t i = 0; i < resultCount; ++i) \
            { \
                mono_array_set(arr, uint32_t, i, data[i]); \
            } \
            return arr; \
        } \
        return NULL; \
    }, IOP_VEC3 a_min, IOP_VEC3 a_max) \


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