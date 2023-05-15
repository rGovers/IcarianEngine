#include "Physics/IcBroadPhaseLayerInterface.h"

#include "Flare/IcarianAssert.h"
#include "Physics/PhysicsEngine.h"

IcBroadPhaseLayerInterface::IcBroadPhaseLayerInterface()
{

}
IcBroadPhaseLayerInterface::~IcBroadPhaseLayerInterface()
{

}

uint IcBroadPhaseLayerInterface::GetNumBroadPhaseLayers() const
{
    return 2;
}
JPH::BroadPhaseLayer IcBroadPhaseLayerInterface::GetBroadPhaseLayer(JPH::ObjectLayer a_layer) const
{
    ICARIAN_ASSERT(a_layer < 2);
    
    return m_layers[a_layer];
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
        }

        ICARIAN_ASSERT(0);
    }
#endif 