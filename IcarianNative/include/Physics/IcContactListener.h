#pragma once

#define GLM_FORCE_SWIZZLE 
#include <glm/glm.hpp>

#include <Jolt/Jolt.h>

#include <Jolt/Physics/Collision/ContactListener.h>

class PhysicsEngine;
class RuntimeFunction;

#include "EnginePhysicsBodyInteropStructures.h"

class IcContactListener : public JPH::ContactListener
{
private:
    PhysicsEngine*   m_engine;
    
    RuntimeFunction* m_onCollisionEnterFunc;
    RuntimeFunction* m_onCollisionStayFunc;
    RuntimeFunction* m_onCollisionExitFunc;

protected:

public:
    IcContactListener(PhysicsEngine* a_engine);
    virtual ~IcContactListener();

    virtual JPH::ValidateResult OnContactValidate(const JPH::Body& a_lhs, const JPH::Body& a_rhs, JPH::RVec3Arg a_baseOffset, const JPH::CollideShapeResult& a_collisionResult);
    virtual void OnContactAdded(const JPH::Body& a_lhs, const JPH::Body& a_rhs, const JPH::ContactManifold& a_manifold, JPH::ContactSettings& a_ioSettings);
    virtual void OnContactPersisted(const JPH::Body& a_lhs, const JPH::Body& a_rhs, const JPH::ContactManifold& a_manifold, JPH::ContactSettings& a_ioSettings);
    virtual void OnContactRemoved(const JPH::SubShapeIDPair& a_shapePair);
};