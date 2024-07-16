#pragma once

#include "InteropTypes.h"

/// @file EngineMeshCollisionShapeInterop.h

/// @cond INTERNAL

#define ENGINE_MESHCOLLISIONSHAPE_EXPORT_TABLE(F) \
    F(IOP_UINT32, IcarianEngine.Physics.Shapes, MeshCollisionShapeInterop, CreateMesh, \
    { \
        char* str = mono_string_to_utf8(a_path); \
        IDEFER(mono_free(str)); \
        return Instance->CreateMeshShape(str); \
    }, IOP_STRING a_path) \

/// @endcond