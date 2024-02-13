#pragma once

#include "InteropTypes.h"

/// @file EngineSphereCollisionShapeInterop.h

/// @cond INTERNAL

#define ENGINE_SPHERECOLLISIONSHAPE_EXPORT_TABLE(F) \
    F(IOP_UINT32, IcarianEngine.Physics.Shapes, SphereCollisionShapeInterop, CreateSphere, \
    { \
        return Instance->CreateSphereShape(a_radius); \
    }, float a_radius) \
    F(float, IcarianEngine.Physics.Shapes, SphereCollisionShapeInterop, GetRadius, \
    { \
        return Instance->GetSphereShapeRadius(a_addr); \
    }, IOP_UINT32 a_addr) \

/// @endcond