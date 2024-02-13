#pragma once

#include "InteropTypes.h"

/// @file EngineBoxCollisionShapeInterop.h

/// @cond INTERNAL

#define ENGINE_BOXCOLLISIONSHAPE_EXPORT_TABLE(F) \
    F(IOP_UINT32, IcarianEngine.Physics.Shapes, BoxCollisionShapeInterop, CreateBox, \
    { \
        return Instance->CreateBoxShape(a_extents); \
    }, IOP_VEC3 a_extents) \
    F(IOP_VEC3, IcarianEngine.Physics.Shapes, BoxCollisionShapeInterop, GetExtents, \
    { \
        return Instance->GetBoxShapeExtents(a_addr); \
    }, IOP_UINT32 a_addr) \

/// @endcond