#include "Physics/IcObjectLayerPairFilter.h"

#include "Flare/IcarianAssert.h"
#include "Physics/PhysicsEngine.h"

IcObjectLayerPairFilter::IcObjectLayerPairFilter()
{

}
IcObjectLayerPairFilter::~IcObjectLayerPairFilter()
{

}

bool IcObjectLayerPairFilter::ShouldCollide(JPH::ObjectLayer a_lhs, JPH::ObjectLayer a_rhs) const
{
    switch (a_lhs)
    {
    case PhysicsEngine::LayerNonMoving:
    {
        return a_rhs == PhysicsEngine::LayerMoving;
    }
    case PhysicsEngine::LayerMoving:
    {
        return true;
    }
    case PhysicsEngine::LayerTrigger:
    {
        return a_rhs == PhysicsEngine::LayerMoving;
    }
    }

    ICARIAN_ASSERT(0);

    return false;
}