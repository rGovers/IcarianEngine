#pragma once

#include <Jolt/Jolt.h>

#include <Jolt/Physics/Collision/BroadPhase/BroadPhaseLayer.h>

class IcBroadPhaseLayerInterface : public JPH::BroadPhaseLayerInterface
{
private:
    JPH::BroadPhaseLayer m_layers[2];

protected:

public:
    IcBroadPhaseLayerInterface();
    virtual ~IcBroadPhaseLayerInterface();

    virtual JPH::uint GetNumBroadPhaseLayers() const;
    virtual JPH::BroadPhaseLayer GetBroadPhaseLayer(JPH::ObjectLayer a_layer) const;

#if defined(JPH_EXTERNAL_PROFILE) || defined(JPH_PROFILE_ENABLED)
	virtual const char* GetBroadPhaseLayerName(JPH::BroadPhaseLayer a_layer) const;
#endif 
};