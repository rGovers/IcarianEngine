#include "Physics/IcObjectVsBroadPhaseLayerFilter.h"

#include "IcarianError.h"
#include "Physics/PhysicsEngine.h"

IcObjectVsBroadPhaseLayerFilter::IcObjectVsBroadPhaseLayerFilter()
{

}
IcObjectVsBroadPhaseLayerFilter::~IcObjectVsBroadPhaseLayerFilter()
{

}

bool IcObjectVsBroadPhaseLayerFilter::ShouldCollide(JPH::ObjectLayer a_lhs, JPH::BroadPhaseLayer a_rhs) const
{
    IVERIFY(a_lhs < 8);

    switch (a_lhs)
    {
    case PhysicsEngine::LayerNonMoving:
    {
        return a_rhs == PhysicsEngine::BroadphaseMoving;
    }
    case PhysicsEngine::LayerTrigger:
    {
        return a_rhs == PhysicsEngine::BroadphaseMoving;
    }
    default:
    {
        return true;
    }
    }

    return false;
}