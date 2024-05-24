#pragma once

#include <Jolt/Jolt.h>

#include <Jolt/Physics/Collision/ObjectLayer.h>

class PhysicsEngine;

class IcObjectLayerPairFilter : public JPH::ObjectLayerPairFilter
{
private:
    PhysicsEngine* m_engine;

protected:

public:
    IcObjectLayerPairFilter(PhysicsEngine* a_engine);
    virtual ~IcObjectLayerPairFilter();

    virtual bool ShouldCollide(JPH::ObjectLayer a_lhs, JPH::ObjectLayer a_rhs) const;
};