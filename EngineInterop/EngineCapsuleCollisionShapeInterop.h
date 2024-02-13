#pragma once

#include "InteropTypes.h"

/// @file EngineCapsuleCollisionShapeInterop.h

/// @cond INTERNAL

#define ENGINE_CAPSULECOLLISIONSHAPE_EXPORT_TABLE(F) \
    F(IOP_UINT32, IcarianEngine.Physics.Shapes, CapsuleCollisionShapeInterop, CreateCapsule, \
    { \
        return Instance->CreateCapsuleShape(a_height, a_radius); \
    }, float a_height, float a_radius) \
    F(float, IcarianEngine.Physics.Shapes, CapsuleCollisionShapeInterop, GetHeight, \
    { \
        return Instance->GetCapsuleShapeHeight(a_addr); \
    }, IOP_UINT32 a_addr) \
    F(float, IcarianEngine.Physics.Shapes, CapsuleCollisionShapeInterop, GetRadius, \
    { \
        return Instance->GetCasuleShapeRadius(a_addr); \
    }, IOP_UINT32 a_addr) \


/// @endcond