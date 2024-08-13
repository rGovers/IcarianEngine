// Icarian Engine - C# Game Engine
// 
// License at end of file.

#pragma once

#include "InteropTypes.h"

/// @file EngineTransformInterop.h

/// @cond INTERNAL

#define ENGINE_TRANSFORM_EXPORT_TABLE(F) \
    F(IOP_UINT32, IcarianEngine, TransformInterop, GenerateTransformBuffer, \
    { \
        return Instance->CreateTransformBuffer(); \
    }) \
    F(IOP_ARRAY(uint[]), IcarianEngine, TransformInterop, BatchGenerateTransformBuffer, \
    { \
        uint32_t* addrs = ObjectManager::BatchCreateTransformBuffer(a_count); \
        IDEFER(delete[] addrs); \
        MonoArray* a = mono_array_new(RuntimeManager::GetDomain(), mono_get_uint32_class(), a_count); \
        for (uint32_t i = 0; i < a_count; ++i) \
        { \
            mono_array_set(a, uint32_t, i, addrs[i]); \
        } \
        return a; \
    }, IOP_UINT32 a_count) \
    F(void, IcarianEngine, TransformInterop, DestroyTransformBuffer, \
    { \
        Instance->DestroyTransformBuffer(a_addr); \
    }, IOP_UINT32 a_addr) \
    \
    F(TransformBuffer, IcarianEngine, TransformInterop, GetTransformBuffer, \
    { \
        return Instance->GetTransformBuffer(a_addr); \
    }, IOP_UINT32 a_addr) \
    F(void, IcarianEngine, TransformInterop, SetTransformBuffer, \
    { \
        Instance->SetTransformBuffer(a_addr, a_buffer); \
    }, IOP_UINT32 a_addr, TransformBuffer a_buffer) \
    F(IOP_ARRAY(float[]), IcarianEngine, TransformInterop, GetTransformMatrix, \
    { \
        MonoArray* a = mono_array_new(RuntimeManager::GetDomain(), mono_get_single_class(), 16); \
        const glm::mat4 mat = ObjectManager::GetMatrix(a_addr); \
        for (uint32_t i = 0; i < 16; ++i) \
        { \
            mono_array_set(a, float, i, mat[i / 4][i % 4]); \
        } \
        return a; \
    }, IOP_UINT32 a_addr) \
    F(IOP_ARRAY(float[]), IcarianEngine, TransformInterop, GetGlobalTransformMatrix, \
    { \
        MonoArray* a = mono_array_new(RuntimeManager::GetDomain(), mono_get_single_class(), 16); \
        const glm::mat4 mat = ObjectManager::GetGlobalMatrix(a_addr); \
        for (uint32_t i = 0; i < 16; ++i) \
        { \
            mono_array_set(a, float, i, mat[i / 4][i % 4]); \
        } \
        return a; \
    }, IOP_UINT32 a_addr) \
    F(void, IcarianEngine, TransformInterop, SetTransformMatrix, \
    { \
        glm::mat4 mat; \
        float* dat = (float*)&mat; \
        for (uint32_t i = 0; i < 16; ++i) \
        { \
            dat[i] = mono_array_get(a_mat, float, i); \
        } \
        TransformBuffer buffer = ObjectManager::GetTransformBuffer(a_addr); \
        glm::vec3 skew; \
        glm::vec4 perspective; \
        glm::decompose(mat, buffer.Scale, buffer.Rotation, buffer.Translation, skew, perspective); \
        ObjectManager::SetTransformBuffer(a_addr, buffer); \
    }, IOP_UINT32 a_addr, IOP_ARRAY(float[]) a_mat) \

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