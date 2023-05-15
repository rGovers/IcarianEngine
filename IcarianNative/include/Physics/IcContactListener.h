#pragma once

#include <Jolt/Jolt.h>

#include <Jolt/Physics/Collision/ContactListener.h>

class IcContactListener : public JPH::ContactListener
{
private:

protected:

public:
    IcContactListener();
    virtual ~IcContactListener();

    virtual JPH::ValidateResult OnContactValidate(const JPH::Body& a_lhs, const JPH::Body& a_rhs, JPH::RVec3Arg a_baseOffset, const JPH::CollideShapeResult& a_collisionResult);
    virtual void OnContactAdded(const JPH::Body& a_lhs, const JPH::Body& a_rhs, const JPH::ContactManifold& a_manifold, JPH::ContactSettings& a_ioSettings);
    virtual void OnContactPersisted(const JPH::Body& a_lhs, const JPH::Body& a_rhs, const JPH::ContactManifold& a_manifold, JPH::ContactSettings& a_ioSettings);
    virtual void OnContactRemoved(const JPH::SubShapeIDPair& a_shapePair);
};