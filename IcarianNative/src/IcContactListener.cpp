#include "Physics/IcContactListener.h"

IcContactListener::IcContactListener()
{

}
IcContactListener::~IcContactListener()
{

}

JPH::ValidateResult IcContactListener::OnContactValidate(const JPH::Body &a_lhs, const JPH::Body &a_rhs, JPH::RVec3Arg a_baseOffset, const JPH::CollideShapeResult &a_collisionResult)
{
    return JPH::ValidateResult::AcceptAllContactsForThisBodyPair;
}

void IcContactListener::OnContactAdded(const JPH::Body &a_lhs, const JPH::Body &a_rhs, const JPH::ContactManifold &a_manifold, JPH::ContactSettings &a_ioSettings)
{

}
void IcContactListener::OnContactPersisted(const JPH::Body &a_lhs, const JPH::Body &a_rhs, const JPH::ContactManifold &a_manifold, JPH::ContactSettings &a_ioSettings)
{
    
}
void IcContactListener::OnContactRemoved(const JPH::SubShapeIDPair &a_shapePair)
{
    
}
