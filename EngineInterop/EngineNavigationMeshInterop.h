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
