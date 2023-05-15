#pragma once

#include <Jolt/Jolt.h>

#include <Jolt/Physics/Collision/ObjectLayer.h>

class IcObjectLayerPairFilter : public JPH::ObjectLayerPairFilter
{
private:

protected:

public:
    IcObjectLayerPairFilter();
    virtual ~IcObjectLayerPairFilter();

    virtual bool ShouldCollide(JPH::ObjectLayer a_lhs, JPH::ObjectLayer a_rhs) const;
};