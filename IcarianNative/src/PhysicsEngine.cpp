#include "Physics/PhysicsEngine.h"

#include <cstdarg>
#include <glm/ext/matrix_transform.hpp>
#include <glm/fwd.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <Jolt/Core/Factory.h>
#include <Jolt/Math/Vec3.h>
#include <Jolt/Physics/Body/BodyID.h>
#include <Jolt/RegisterTypes.h>
#include <shared_mutex>
#include <sstream>
#include <vector>

#include "Config.h"
#include "Flare/IcarianAssert.h"
#include "Flare/IcarianDefer.h"
#include "Jolt/Physics/Body/Body.h"
#include "Jolt/Physics/Body/BodyInterface.h"
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

    ICARIAN_ASSERT_MSG_R(0, ss.str());

    return true;
}

PhysicsEngine::PhysicsEngine(Config* a_config) 
{
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
    m_pairFilter = new IcObjectLayerPairFilter();

    m_physicsSystem = new JPH::PhysicsSystem();
    m_physicsSystem->Init((JPH::uint)MaxBodies, 0, (JPH::uint)MaxBodies, (JPH::uint)MaxContactConstraints, *m_broadPhase, *m_objectBroad, *m_pairFilter);

    m_contactListener = new IcContactListener(this);
    m_activationListener = new IcBodyActivationListener();

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

uint32_t PhysicsEngine::GetBodyAddr(JPH::uint a_joltIndex)
{
    const std::shared_lock g = std::shared_lock(m_mapMutex);

    const auto iter = m_bodyMap.find(a_joltIndex);
    if (iter != m_bodyMap.end())
    {
        return iter->second;
    }

    return -1;
}

void PhysicsEngine::Update(double a_delta)
{
    // TODO: Need to figure out why Jolt crashes with less then 3 threads
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

            m_physicsSystem->Update((float)m_fixedTimeStep, steps, 2, m_allocator, m_jobSystem);

            void* args[] = 
            { 
                &m_fixedTimeStep, 
                &m_fixedTimePassed
            };

            m_fixedUpdateFunction->Exec(args);
        }
    }

    {
        PROFILESTACK("Physics Sync");

        const std::vector<JPH::BodyID> bodies = m_activationListener->ToBodies();
        const std::shared_lock g = std::shared_lock(m_mapMutex);

        // Should not need but doing just incase for good practice as it multithreaded app
        const JPH::BodyLockInterfaceLocking& interface = m_physicsSystem->GetBodyLockInterface();

        // Need to sync the physics transform to the transform
        for (const JPH::BodyID id : bodies)
        {
            const auto iter = m_bodyMap.find(id.GetIndex());
            if (iter != m_bodyMap.end())
            {
                const BodyBinding binding = m_bodyBindings[iter->second];
                if (binding.TransformAddr == -1)
                {
                    continue;
                }

                const JPH::Body* body = interface.TryGetBody(id);
                if (body == nullptr)
                {
                    continue;
                }

                TransformBuffer buffer = ObjectManager::GetTransformBuffer(binding.TransformAddr);

                glm::vec3 iTranslation = glm::vec3(0.0f);
                glm::quat iRotation = glm::identity<glm::quat>();

                if (buffer.ParentAddr != -1)
                {
                    glm::vec3 iScale;
                    glm::vec3 iSkew;
                    glm::vec4 iPerspectice;

                    const glm::mat4 pMat = ObjectManager::GetGlobalMatrix(buffer.ParentAddr);
                    const glm::mat4 pInv = glm::inverse(pMat);

                    glm::decompose(pInv, iTranslation, iRotation, iScale, iSkew, iPerspectice);
                }

                const PhysicsInterfaceReadLock lock = PhysicsInterfaceReadLock(id, interface);

                JPH::RVec3 jTranslation = body->GetPosition();
                JPH::Quat jRotation = body->GetRotation();

                const glm::vec4 diff = glm::vec4(jTranslation.GetX(), jTranslation.GetY(), jTranslation.GetZ(), 1.0f) + glm::vec4(iTranslation, 0.0f);

                buffer.Translation = (iRotation * diff).xyz();
                buffer.Rotation = glm::quat(jRotation.GetX(), jRotation.GetY(), jRotation.GetZ(), jRotation.GetW()) * iRotation;

                ObjectManager::SetTransformBuffer(binding.TransformAddr, buffer);
            }
        }
    }
}
