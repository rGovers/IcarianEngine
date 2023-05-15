#pragma once

#include <Jolt/Jolt.h>

#include <Jolt/Physics/Collision/BroadPhase/BroadPhaseLayer.h>

class IcObjectVsBroadPhaseLayerFilter : public JPH::ObjectVsBroadPhaseLayerFilter
{
private:

protected:

public:
    IcObjectVsBroadPhaseLayerFilter();
    virtual ~IcObjectVsBroadPhaseLayerFilter();

    virtual bool ShouldCollide(JPH::ObjectLayer a_lhs, JPH::BroadPhaseLayer a_rhs) const;
};