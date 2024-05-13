#include "Physics/IcBroadPhaseLayerInterface.h"

#include "Core/IcarianAssert.h"
#include "Physics/PhysicsEngine.h"

IcBroadPhaseLayerInterface::IcBroadPhaseLayerInterface()
{
    m_layers[0] = PhysicsEngine::BroadphaseNonMoving;
    m_layers[1] = PhysicsEngine::BroadphaseMoving;
    m_layers[2] = PhysicsEngine::BroadphaseTrigger;
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
    ICARIAN_ASSERT(a_layer < 3);
    
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
        case (JPH::BroadPhaseLayer::Type)PhysicsEngine::BroadphaseTrigger:
        {
            return "TRIGGER";
        }
        }

        ICARIAN_ASSERT(0);
    }
#endif 