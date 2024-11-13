// Icarian Engine - C# Game Engine
// 
// License at end of file.

#include "Physics/IcContactListener.h"

#include "Physics/PhysicsEngine.h"
#include "Runtime/RuntimeFunction.h"
#include "Runtime/RuntimeManager.h"

class ContactThreadJob : public ThreadJob
{
private:
    RuntimeFunction*    m_function;
    CollisionDataBuffer m_buffer;

protected:

public:
    ContactThreadJob(const CollisionDataBuffer& a_buffer, RuntimeFunction* a_function) : ThreadJob(JobPriority_RuntimeHigh)
    {
        m_function = a_function;
        m_buffer = a_buffer;
    }
    virtual ~ContactThreadJob()
    {

    }

    virtual void Execute() 
    {
        void* args[] =
        {
            &m_buffer
        };

        m_function->Exec(args);
    }
};

IcContactListener::IcContactListener(PhysicsEngine* a_engine)
{
    m_engine = a_engine;

    m_onCollisionEnterFunc = RuntimeManager::GetFunction("IcarianEngine.Physics", "PhysicsBody", ":OnCollisionEnter");
    m_onCollisionStayFunc = RuntimeManager::GetFunction("IcarianEngine.Physics", "PhysicsBody", ":OnCollisionStay");
    m_onCollisionExitFunc = RuntimeManager::GetFunction("IcarianEngine.Physics", "PhysicsBody", ":OnCollisionExit");
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
    // Not the correct way but should work
    const JPH::RVec3 pos = a_manifold.mBaseOffset;
    const JPH::RVec3 normalV = a_manifold.mWorldSpaceNormal;

    const JPH::BodyID bodyAID = a_lhs.GetID();
    const JPH::BodyID bodyBID = a_rhs.GetID();

    const JPH::uint32 bodyAIndex = bodyAID.GetIndex();
    const JPH::uint32 bodyBIndex = bodyBID.GetIndex();

    const CollisionDataBuffer data = 
    {
        .IsTrigger = (uint32_t)a_ioSettings.mIsSensor,
        .BodyAddrA = (uint32_t)m_engine->GetBodyAddr(bodyAIndex),
        .BodyAddrB = (uint32_t)m_engine->GetBodyAddr(bodyBIndex),
        .Position = glm::vec3(pos.GetX(), pos.GetY(), pos.GetZ()),
        .Normal = glm::vec3(normalV.GetX(), normalV.GetY(), normalV.GetZ()),
        .Depth = (float)a_manifold.mPenetrationDepth
    };

    ThreadPool::PushJob(new ContactThreadJob(data, m_onCollisionEnterFunc));
}
void IcContactListener::OnContactPersisted(const JPH::Body& a_lhs, const JPH::Body& a_rhs, const JPH::ContactManifold& a_manifold, JPH::ContactSettings& a_ioSettings)
{
    const JPH::RVec3 pos = a_manifold.mBaseOffset;
    const JPH::RVec3 normalV = a_manifold.mWorldSpaceNormal;

    const JPH::BodyID bodyAID = a_lhs.GetID();
    const JPH::BodyID bodyBID = a_rhs.GetID();

    const JPH::uint32 bodyAIndex = bodyAID.GetIndex();
    const JPH::uint32 bodyBIndex = bodyBID.GetIndex();

    const CollisionDataBuffer data = 
    {
        .IsTrigger = (uint32_t)a_ioSettings.mIsSensor,
        .BodyAddrA = (uint32_t)m_engine->GetBodyAddr(bodyAIndex),
        .BodyAddrB = (uint32_t)m_engine->GetBodyAddr(bodyBIndex),
        .Position = glm::vec3(pos.GetX(), pos.GetY(), pos.GetZ()),
        .Normal = glm::vec3(normalV.GetX(), normalV.GetY(), normalV.GetZ()),
        .Depth = (float)a_manifold.mPenetrationDepth
    };

    ThreadPool::PushJob(new ContactThreadJob(data, m_onCollisionStayFunc));
}
void IcContactListener::OnContactRemoved(const JPH::SubShapeIDPair& a_shapePair)
{
    const JPH::BodyID bodyAID = a_shapePair.GetBody1ID();
    const JPH::BodyID bodyBID = a_shapePair.GetBody2ID();

    const JPH::uint32 bodyAIndex = bodyAID.GetIndex();
    const JPH::uint32 bodyBIndex = bodyBID.GetIndex();

    const CollisionDataBuffer data =
    {
        .BodyAddrA = m_engine->GetBodyAddr(bodyAIndex),
        .BodyAddrB = m_engine->GetBodyAddr(bodyBIndex)
    };

    ThreadPool::PushJob(new ContactThreadJob(data, m_onCollisionExitFunc));
}

// MIT License
// 
// Copyright (c) 2024 River Govers
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.