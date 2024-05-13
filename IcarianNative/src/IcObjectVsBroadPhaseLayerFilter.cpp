#include "Physics/IcObjectVsBroadPhaseLayerFilter.h"

#include "Core/IcarianAssert.h"
#include "Physics/PhysicsEngine.h"

IcObjectVsBroadPhaseLayerFilter::IcObjectVsBroadPhaseLayerFilter()
{

}
IcObjectVsBroadPhaseLayerFilter::~IcObjectVsBroadPhaseLayerFilter()
{

}

bool IcObjectVsBroadPhaseLayerFilter::ShouldCollide(JPH::ObjectLayer a_lhs, JPH::BroadPhaseLayer a_rhs) const
{
    switch (a_lhs)
    {
    case PhysicsEngine::LayerNonMoving:
    {
        return a_rhs == PhysicsEngine::BroadphaseMoving;
    }
    case PhysicsEngine::LayerMoving:
    {
        return true;
    }
    case PhysicsEngine::LayerTrigger:
    {
        return a_rhs == PhysicsEngine::BroadphaseMoving;
    }
    }

    ICARIAN_ASSERT(0);

    return false;
}