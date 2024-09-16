// Icarian Engine - C# Game Engine
// 
// License at end of file.

#include "Physics/PhysicsEngine.h"

#include <cstdarg>
#include <glm/ext/matrix_transform.hpp>
#include <glm/fwd.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <Jolt/Core/Factory.h>
#include <Jolt/Math/Vec3.h>
#include <Jolt/Physics/Body/Body.h>
#include <Jolt/Physics/Body/BodyID.h>
#include <Jolt/Physics/Body/BodyInterface.h>
#include <Jolt/RegisterTypes.h>
#include <sstream>

#include "Config.h"
#include "Core/Bitfield.h"
#include "IcarianError.h"
#include "ObjectManager.h"
#include "Physics/InterfaceLock.h"
#include "Physics/PhysicsEngineBindings.h"
#include "Profiler.h"
#include "Runtime/RuntimeFunction.h"
#include "Runtime/RuntimeManager.h"
#include "Trace.h"

static void TraceImpl(const char* inFMT, ...)
{
    va_list list;
    va_start(list, inFMT);
    char buffer[2048];
    vsnprintf(buffer, sizeof(buffer), inFMT, list);
    va_end(list);

    TRACE(buffer);
}

static bool AssertImpl(const char* a_expression, const char* a_message, const char* a_file, JPH::uint a_line)
{
    std::stringstream ss;

    ss << "Jolt Assert: " << a_expression;
    if (a_message != nullptr)
    {
        ss << ": " << a_message;
    }

    ss << "{" << a_file << ":" << std::to_string(a_line) << "}";

    IERROR(ss.str());

    return true;
}

PhysicsEngine::PhysicsEngine(Config* a_config) 
{
    memset(m_objectLayerCollisions, 0, sizeof(m_objectLayerCollisions));
    for (uint32_t i = 0; i < 6; ++i)
    {
        for (uint32_t j = i; j < 6; ++j)
        {
            ISETBIT(m_objectLayerCollisions[i], j);
            ISETBIT(m_objectLayerCollisions[j], i);
        }

        ISETBIT(m_objectLayerCollisions[i], 6);
        ISETBIT(m_objectLayerCollisions[i], 7);

        ISETBIT(m_objectLayerCollisions[6], i);
        ISETBIT(m_objectLayerCollisions[7], i);
    }

    m_fixedUpdateFunction = RuntimeManager::GetFunction("IcarianEngine", "Program", ":FixedUpdate(double,double)");

    m_fixedTimeStep = a_config->GetFixedTimeStep();
    m_fixedTimeTimer = 0.0;
    m_fixedTimePassed = 0.0;

    JPH::RegisterDefaultAllocator();

    JPH::Trace = TraceImpl;
    JPH_IF_ENABLE_ASSERTS(JPH::AssertFailed = AssertImpl;)

    JPH::Factory::sInstance = new JPH::Factory();

    JPH::RegisterTypes();

    m_allocator = new JPH::TempAllocatorImpl(AllocatorSize);

    m_jobSystem = new IcPhysicsJobSystem(JPH::cMaxPhysicsBarriers);

    m_broadPhase = new IcBroadPhaseLayerInterface();
    m_objectBroad = new IcObjectVsBroadPhaseLayerFilter();
    m_pairFilter = new IcObjectLayerPairFilter(this);

    m_physicsSystem = new JPH::PhysicsSystem();
    m_physicsSystem->Init((JPH::uint)MaxBodies, 0, (JPH::uint)MaxBodies, (JPH::uint)MaxContactConstraints, *m_broadPhase, *m_objectBroad, *m_pairFilter);

    m_contactListener = new IcContactListener(this);
    m_activationListener = new IcBodyActivationListener();
    m_characterListener = new IcCharacterListener(this);

    m_physicsSystem->SetContactListener(m_contactListener);
    m_physicsSystem->SetGravity(JPH::Vec3(0.0f, 9.807f, 0.0f));

    m_runtimeBindings = new PhysicsEngineBindings(this);
}
PhysicsEngine::~PhysicsEngine()
{
    delete m_fixedUpdateFunction;

    delete m_physicsSystem;

    delete m_contactListener;
    delete m_activationListener;
    delete m_characterListener;

    delete m_broadPhase;
    delete m_objectBroad;
    delete m_pairFilter;

    delete m_jobSystem;

    delete m_allocator;

    delete m_runtimeBindings;

    JPH::UnregisterTypes();

    delete JPH::Factory::sInstance;
    JPH::Factory::sInstance = nullptr;
}

bool PhysicsEngine::CanObjectLayersCollide(uint32_t a_lhs, uint32_t a_rhs) const
{
    IVERIFY(a_lhs < 8);
    IVERIFY(a_rhs < 8);

    return IISBITSET(m_objectLayerCollisions[a_lhs], a_rhs);
}

uint32_t PhysicsEngine::GetBodyAddr(JPH::uint a_joltIndex)
{
    const SharedThreadGuard g = SharedThreadGuard(m_bodyMapLock);

    const auto iter = m_bodyMap.find(a_joltIndex);
    if (iter != m_bodyMap.end())
    {
        return iter->second;
    }

    return -1;
}

static void TransformObject(uint32_t a_transformAddr, const glm::vec3& a_translation, const glm::quat& a_rotation)
{
    TransformBuffer buffer = ObjectManager::GetTransformBuffer(a_transformAddr);

    glm::vec3 iTranslation = glm::vec3(0.0f);
    glm::quat iRotation = glm::identity<glm::quat>();

    if (buffer.ParentAddr != -1)
    {
        glm::vec3 s;
        glm::vec3 sk;
        glm::vec4 p;

        const glm::mat4 pMat = ObjectManager::GetGlobalMatrix(buffer.ParentAddr);
        const glm::mat4 pInv = glm::inverse(pMat);

        glm::decompose(pInv, iTranslation, iRotation, s, sk, p);
    }

    const glm::vec4 diff = glm::vec4(a_translation, 1.0f) + glm::vec4(iTranslation, 0.0f);

    buffer.Translation = (iRotation * diff).xyz();
    buffer.Rotation = a_rotation * iRotation;

    ObjectManager::SetTransformBuffer(a_transformAddr, buffer);
}

void PhysicsEngine::Update(double a_delta)
{
    {
        PROFILESTACK("Physics Sim");

        m_fixedTimeTimer += a_delta;

        // Done some digging and found a note about stability above 60hz needing to be done in steps
        constexpr double JoltStepMagicNumber = 1.0 / 60.0;

        const int steps = (int)(m_fixedTimeStep / JoltStepMagicNumber + 1);

        while (m_fixedTimeTimer >= m_fixedTimeStep)
        {
            m_fixedTimeTimer -= m_fixedTimeStep;
            m_fixedTimePassed += m_fixedTimeStep;

            void* args[] = 
            { 
                &m_fixedTimeStep, 
                &m_fixedTimePassed
            };

            m_fixedUpdateFunction->Exec(args);

            const JPH::Vec3 gravity = m_physicsSystem->GetGravity();

            const JPH::DefaultBroadPhaseLayerFilter broadFilter = m_physicsSystem->GetDefaultBroadPhaseLayerFilter(0);
            const JPH::DefaultObjectLayerFilter objectFilter = m_physicsSystem->GetDefaultLayerFilter(0);

            const Array<JPH::CharacterVirtual*> characters = m_characters.ToActiveArray();
            for (JPH::CharacterVirtual* c : characters)
            {
                const JPH::Vec3 up = c->GetUp();

                const JPH::CharacterVirtual::ExtendedUpdateSettings updateSettings = 
                {
                    .mStickToFloorStepDown = -up * 0.2f,
                    .mWalkStairsStepUp = up * 0.2f
                };

                c->ExtendedUpdate((float)m_fixedTimeStep, gravity, updateSettings, broadFilter, objectFilter, { }, { }, *m_allocator);
            }

            m_physicsSystem->Update((float)m_fixedTimeStep, steps, m_allocator, m_jobSystem);
        }
    }

    {
        PROFILESTACK("Physics Sync");

        {
            PROFILESTACK("Physics Bodies");

            const Array<JPH::BodyID> bodies = m_activationListener->ToBodies();
            const SharedThreadGuard g = SharedThreadGuard(m_bodyMapLock);

            // Should not need but doing just incase for good practice as it multithreaded app
            const JPH::BodyLockInterfaceLocking& interface = m_physicsSystem->GetBodyLockInterface();

            // Need to sync the physics transform to the transform
            for (const JPH::BodyID id : bodies)
            {
                const PhysicsInterfaceReadLock lock = PhysicsInterfaceReadLock(id, interface);

                const JPH::Body* body = interface.TryGetBody(id);
                if (body == nullptr)
                {
                    continue;
                }

                constexpr float Min = std::numeric_limits<float>::min();

                // TODO: This is a hack should probably improve this
                // Needed when regenerating Rigidbodies
                const JPH::RVec3 jPos = body->GetPosition();
                if (jPos == JPH::RVec3(Min, Min, Min))
                {
                    continue;
                }

                const auto iter = m_bodyMap.find(id.GetIndex());
                if (iter == m_bodyMap.end())
                {
                    continue;
                }

                const BodyBinding binding = m_bodyBindings[iter->second];

                const bool valid = binding.TransformAddr != -1;
                if (!valid)
                {
                    continue;
                }

                const JPH::RVec3 jTranslation = jPos + body->GetLinearVelocity() * m_fixedTimeTimer;
                const JPH::Quat jRotation = body->GetRotation();

                const glm::vec3 translation = glm::vec3(jTranslation.GetX(), jTranslation.GetY(), jTranslation.GetZ());
                const glm::quat rotation = glm::quat(jRotation.GetX(), jRotation.GetY(), jRotation.GetZ(), jRotation.GetW());

                TransformObject(binding.TransformAddr, translation, rotation);
            }
        }
        
        {
            PROFILESTACK("Characters");

            const Array<JPH::CharacterVirtual*> characters = m_characters.ToActiveArray();
            for (const JPH::CharacterVirtual* c : characters)
            {
                const uint32_t transformAddr = (uint32_t)(c->GetUserData() & 0xFFFFFFFF);

                const JPH::Vec3 jTranslation = c->GetPosition();
                const JPH::Quat jRotation = c->GetRotation();

                const glm::vec3 translation = glm::vec3(jTranslation.GetX(), jTranslation.GetY(), jTranslation.GetZ());
                const glm::quat rotation = glm::quat(jRotation.GetX(), jRotation.GetY(), jRotation.GetZ(), jRotation.GetW());

                TransformObject(transformAddr, translation, rotation);
            }
        }
    }
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