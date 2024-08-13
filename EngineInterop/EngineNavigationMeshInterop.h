// Icarian Engine - C# Game Engine
// 
// License at end of file.

#pragma once

#include "InteropTypes.h"

/// @file EngineNavigationMeshInterop.h

#ifdef CUBE_LANGUAGE_CPP
#include "DeletionQueue.h"
#endif

/// @cond INTERNAL

#define ENGINE_NAVIGATIONMESH_EXPORT_TABLE(F) \
    F(IOP_UINT32, IcarianEngine.AI, NavigationMeshInterop, GenerateMesh, \
    { \
        char* str = mono_string_to_utf8(a_path); \
        IDEFER(mono_free(str)); \
        return Instance->CreateNavMesh(str); \
    }, IOP_STRING a_path) \
    F(void, IcarianEngine.AI, NavigationMeshInterop, DestroyMesh, \
    { \
        IPUSHDELETIONFUNC( \
        { \
            Instance->DestroyNavMesh(a_addr); \
        }, DeletionIndex_Update); \
    }, IOP_UINT32 a_addr) \
    F(IOP_ARRAY(IOP_VEC3[]), IcarianEngine.AI, NavigationMeshInterop, GetPath, \
    { \
        const Array<glm::vec3> path = Instance->GetNavMeshPath(a_addr, a_startPoint, a_endPoint, a_agentRadius); \
        const uint32_t count = path.Size(); \
        MonoClass* klass = RuntimeManager::GetClass("IcarianEngine.Maths", "Vector3"); \
        MonoArray* arr = mono_array_new(mono_domain_get(), klass, count); \
        for (uint32_t i = 0; i < count; ++i) \
        { \
            mono_array_set(arr, IOP_VEC3, i, path[i]); \
        } \
        return arr; \
    }, IOP_UINT32 a_addr, IOP_VEC3 a_startPoint, IOP_VEC3 a_endPoint, float a_agentRadius) \

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