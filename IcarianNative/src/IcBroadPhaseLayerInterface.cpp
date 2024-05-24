#include "Physics/IcBroadPhaseLayerInterface.h"

#include "Core/IcarianAssert.h"
#include "IcarianError.h"
#include "Physics/PhysicsEngine.h"

IcBroadPhaseLayerInterface::IcBroadPhaseLayerInterface()
{

}
IcBroadPhaseLayerInterface::~IcBroadPhaseLayerInterface()
{

}

JPH::uint IcBroadPhaseLayerInterface::GetNumBroadPhaseLayers() const
{
    return 3;
}
JPH::BroadPhaseLayer IcBroadPhaseLayerInterface::GetBroadPhaseLayer(JPH::ObjectLayer a_layer) const
{
    IVERIFY(a_layer < 8);
    
    switch (a_layer)
    {
    case PhysicsEngine::LayerNonMoving:
    {
        return PhysicsEngine::BroadphaseNonMoving;
    }
    case PhysicsEngine::LayerTrigger:
    {
        return PhysicsEngine::BroadphaseTrigger;
    }
    }

    return PhysicsEngine::BroadphaseMoving;
}

#if defined(JPH_EXTERNAL_PROFILE) || defined(JPH_PROFILE_ENABLED)
const char* IcBroadPhaseLayerInterface::GetBroadPhaseLayerName(JPH::BroadPhaseLayer a_layer) const
{
    switch ((JPH::BroadPhaseLayer::Type)a_layer)
    {
    case (JPH::BroadPhaseLayer::Type)PhysicsEngine::BroadphaseNonMoving:
    {
        return "NON_MOVING";
    }
    case (JPH::BroadPhaseLayer::Type)PhysicsEngine::BroadphaseMoving:
    {
        return "MOVING";
    }
    case (JPH::BroadPhaseLayer::Type)PhysicsEngine::BroadphaseTrigger:
    {
        return "TRIGGER";
    }
    }

    ICARIAN_ASSERT(0);
}
#endif 