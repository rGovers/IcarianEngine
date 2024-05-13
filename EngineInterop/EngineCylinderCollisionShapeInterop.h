#pragma once

#include "InteropTypes.h"

/// @file EngineCylinderCollisionShapeInterop.h

/// @cond INTERNAL

#define ENGINE_CYLINDERCOLLISIONSHAPE_EXPORT_TABLE(F) \
    F(IOP_UINT32, IcarianEngine.Physics.Shapes, CylinderCollisionShapeInterop, CreateCylinder, \
    { \
        return Instance->CreateCylinderShape(a_height, a_radius); \
    }, float a_height, float a_radius) \
    F(float, IcarianEngine.Physics.Shapes, CylinderCollisionShapeInterop, GetHeight, \
    { \
        return Instance->GetCylinderShapeHeight(a_addr); \
    }, IOP_UINT32 a_addr) \
    F(float, IcarianEngine.Physics.Shapes, CylinderCollisionShapeInterop, GetRadius, \
    { \
        return Instance->GetCylinderShapeRadius(a_addr); \
    }, IOP_UINT32 a_addr) \


/// @endcond