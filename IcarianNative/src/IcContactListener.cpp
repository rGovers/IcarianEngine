#include "Physics/IcContactListener.h"

#include "Physics/PhysicsEngine.h"
#include "Runtime/RuntimeFunction.h"
#include "Runtime/RuntimeManager.h"

IcContactListener::IcContactListener(PhysicsEngine* a_engine, RuntimeManager* a_runtime)
{
    m_engine = a_engine;

    m_runtime = a_runtime;

    m_onCollisionEnterFunc = a_runtime->GetFunction("IcarianEngine.Physics", "PhysicsBody", ":OnCollisionEnter");
    m_onCollisionStayFunc = a_runtime->GetFunction("IcarianEngine.Physics", "PhysicsBody", ":OnCollisionStay");
    m_onCollisionExitFunc = a_runtime->GetFunction("IcarianEngine.Physics", "PhysicsBody", ":OnCollisionExit");
}
IcContactListener::~IcContactListener()
{
    delete m_onCollisionEnterFunc;
    delete m_onCollisionStayFunc;
    delete m_onCollisionExitFunc;
}

JPH::ValidateResult IcContactListener::OnContactValidate(const JPH::Body &a_lhs, const JPH::Body &a_rhs, JPH::RVec3Arg a_baseOffset, const JPH::CollideShapeResult &a_collisionResult)
{
    return JPH::ValidateResult::AcceptAllContactsForThisBodyPair;
}

void IcContactListener::OnContactAdded(const JPH::Body& a_lhs, const JPH::Body& a_rhs, const JPH::ContactManifold& a_manifold, JPH::ContactSettings& a_ioSettings)
{
    m_runtime->AttachThread();

    DispatchCollisionData data;

    data.IsTrigger = a_ioSettings.mIsSensor;

    data.BodyAddrA = m_engine->GetBodyAddr(a_lhs.GetID().GetIndex());
    data.BodyAddrB = m_engine->GetBodyAddr(a_rhs.GetID().GetIndex());

    const JPH::RVec3 normalV = a_manifold.mWorldSpaceNormal;

    data.Normal = glm::vec3(normalV.GetX(), normalV.GetY(), normalV.GetZ());
    data.Depth = a_manifold.mPenetrationDepth;

    void* args[] =
    {
        &data
    };

    m_onCollisionEnterFunc->Exec(args);
}
void IcContactListener::OnContactPersisted(const JPH::Body& a_lhs, const JPH::Body& a_rhs, const JPH::ContactManifold& a_manifold, JPH::ContactSettings& a_ioSettings)
{
    m_runtime->AttachThread();

    DispatchCollisionData data;

    data.IsTrigger = a_ioSettings.mIsSensor;

    data.BodyAddrA = m_engine->GetBodyAddr(a_lhs.GetID().GetIndex());
    data.BodyAddrB = m_engine->GetBodyAddr(a_rhs.GetID().GetIndex());

    const JPH::RVec3 normalV = a_manifold.mWorldSpaceNormal;

    data.Normal = glm::vec3(normalV.GetX(), normalV.GetY(), normalV.GetZ());
    data.Depth = a_manifold.mPenetrationDepth;

    void* args[] =
    {
        &data
    };

    m_onCollisionStayFunc->Exec(args);
}
void IcContactListener::OnContactRemoved(const JPH::SubShapeIDPair& a_shapePair)
{
    m_runtime->AttachThread();

    DispatchCollisionData data;

    data.BodyAddrA = m_engine->GetBodyAddr(a_shapePair.GetBody1ID().GetIndex());
    data.BodyAddrB = m_engine->GetBodyAddr(a_shapePair.GetBody2ID().GetIndex());

    void* args[] =
    {
        &data
    };

    m_onCollisionExitFunc->Exec(args);
}
