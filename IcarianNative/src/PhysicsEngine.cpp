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
#include "Core/DataTypes/Allocators/LeakAllocator.h"
#include "Core/DataTypes/Allocators/MallocAllocator.h"
#include "Core/DataTypes/Allocators/MultiSourceAllocator.h"
#include "Core/DataTypes/Allocators/OSAllocator.h"
#include "Core/DataTypes/Allocators/UberAllocator.h"
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

[[maybe_unused]] static bool AssertImpl(const char* a_expression, const char* a_message, const char* a_file, JPH::uint a_line)
{
    const IcarianCore::COWU8String str = ILAMBDA(
    {
        IcarianCore::COWU8String val = IcarianCore::COWU8String("Jolt Assert: ", IcarianCore::MallocAllocator::Instance) + a_expression;
        if (a_message != nullptr)
        {
            val = val + ": " + a_message;
        }
        val = val + "{" + a_file + ":" + IcarianCore::COWU8String::FromValue(a_line, 10, IcarianCore::MallocAllocator::Instance) + "}";

        ILRETURN val;
    });

    IERROR(str);

    return true;
}

static IcarianCore::ComplexAllocator* Alloc = nullptr;

constexpr static uint32_t PhysicsDefaultAllocationAlignment = 16;

static void* IcAlloc(size_t a_inSize)
{
	return Alloc->Allocate((uint64_t)a_inSize, PhysicsDefaultAllocationAlignment);
}
static void* IcRealloc(void* a_inBlock, size_t a_oldSize, size_t a_newSize)
{
    return Alloc->Realloc(a_inBlock, (uint64_t)a_newSize, PhysicsDefaultAllocationAlignment);
}
static void IcFree(void* a_inBlock)
{
	Alloc->Free(a_inBlock);
}

static void* IcAlignedAllocate(size_t a_inSize, size_t a_inAlignment)
{
    return Alloc->Allocate((uint64_t)a_inSize, (uint32_t)a_inAlignment);
}
static void IcAlignedFree(void* a_inBlock)
{
    Alloc->Free(a_inBlock);
}

PhysicsEngine::PhysicsEngine(Config* a_config)
{
    TRACE("Creating PhysicsEngine");

    IVERIFY(Alloc == nullptr);
    m_smallAllocator = IcarianCore::MallocAllocator::Instance->Create<IcarianCore::BlockAllocator>(SmallAllocatorSize, IcarianCore::UberAllocator::Instance);
    m_largeAllocator = IcarianCore::MallocAllocator::Instance->Create<IcarianCore::BlockAllocator>(LargeAllocatorSize, IcarianCore::UberAllocator::Instance);

    m_allocatorChain = m_smallAllocator->Create<IcarianCore::Array<IcarianCore::Allocator*>>(m_smallAllocator);

    const IcarianCore::AllocationSource allocatorSources[] =
    {
        {
            .Alloc = m_smallAllocator,
            .MaxSize = SmallAllocatorSize >> 1,
        },
        {
            .Alloc = m_largeAllocator,
            .MaxSize = LargeAllocatorSize >> 1,
        },
        {
            .Alloc = IcarianCore::OSAllocator::Instance,
            .MaxSize = uint64_t(-1),
        },
    };

    constexpr uint32_t AllocatorCount = sizeof(allocatorSources) / sizeof(*allocatorSources);

    Alloc = m_smallAllocator->Create<IcarianCore::MultiSourceAllocator>(m_smallAllocator, allocatorSources, AllocatorCount);
    m_allocatorChain->Push(Alloc);

    if (a_config->IsHeadless())
    {
        m_trackerAllocator = m_smallAllocator->Create<IcarianCore::TrackerAllocator>(Alloc);
        Alloc = m_trackerAllocator;
        m_allocatorChain->Push(Alloc);
    }

#ifdef DEBUG
    Alloc = m_smallAllocator->Create<IcarianCore::LeakAllocator>(Alloc);
    m_allocatorChain->Push(Alloc);
#endif

    m_data = Alloc->ZTAllocate<ClassData>();
    m_data->BodyMap = IcarianCore::Dictionary<JPH::uint32, uint32_t>(Alloc);

    for (uint32_t i = 0; i < 6; ++i)
    {
        for (uint32_t j = i; j < 6; ++j)
        {
            ISETBIT(m_data->ObjectLayerCollisions[i], j);
            ISETBIT(m_data->ObjectLayerCollisions[j], i);
        }

        ISETBIT(m_data->ObjectLayerCollisions[i], 6);
        ISETBIT(m_data->ObjectLayerCollisions[i], 7);

        ISETBIT(m_data->ObjectLayerCollisions[6], i);
        ISETBIT(m_data->ObjectLayerCollisions[7], i);
    }

    m_data->FixedUpdateFunction = RuntimeManager::GetFunction("IcarianEngine", "Program", ":FixedUpdate(double,double)");

    m_data->FixedTimeStep = a_config->GetFixedTimeStep();
    m_data->FixedTimeTimer = 0.0;
    m_data->FixedTimePassed = 0.0;

    JPH::Allocate = IcAlloc;
    JPH::Reallocate = IcRealloc;
	JPH::Free = IcFree;
	JPH::AlignedAllocate = IcAlignedAllocate;
	JPH::AlignedFree = IcAlignedFree;

    JPH::Trace = TraceImpl;
    JPH_IF_ENABLE_ASSERTS(JPH::AssertFailed = AssertImpl;)

    // Jolt is annoying as they override new and delete rather then leave it and use an allocator object
    // Because they override the new and delete we have to create them with new and delete as they no longer have the inplace new and delete
    // This is bad code smell to me as it can create side effects messing with the default allocation methods
    JPH::Factory::sInstance = new JPH::Factory();

    JPH::RegisterTypes();

    m_data->TempAllocator = new JPH::TempAllocatorImpl(TempAllocatorSize);

    m_data->JobSystem = new IcPhysicsJobSystem(JPH::cMaxPhysicsBarriers);

    m_data->BroadPhase = new IcBroadPhaseLayerInterface();
    m_data->ObjectBroad = new IcObjectVsBroadPhaseLayerFilter();
    m_data->PairFilter = new IcObjectLayerPairFilter(this);

    m_data->PhysicsSystem = new JPH::PhysicsSystem();
    m_data->PhysicsSystem->Init
    (
        (JPH::uint)MaxBodies,
        0,
        (JPH::uint)MaxBodies,
        (JPH::uint)MaxContactConstraints,
        *m_data->BroadPhase,
        *m_data->ObjectBroad,
        *m_data->PairFilter
    );

    m_data->ContactListener = new IcContactListener(this);
    m_data->ActivationListener = new IcBodyActivationListener();
    m_data->CharacterListener = new IcCharacterListener(this);

    m_data->PhysicsSystem->SetContactListener(m_data->ContactListener);
    m_data->PhysicsSystem->SetGravity(JPH::Vec3(0.0f, 9.807f, 0.0f));

    m_data->RuntimeBindings = Alloc->Create<PhysicsEngineBindings>(this);
}
PhysicsEngine::~PhysicsEngine()
{
    IVERIFY(Alloc != nullptr);

    IcarianCore::MallocAllocator::Instance->Destroy(m_data->FixedUpdateFunction);

    delete m_data->PhysicsSystem;

    delete m_data->ContactListener;
    delete m_data->ActivationListener;
    delete m_data->CharacterListener;

    delete m_data->BroadPhase;
    delete m_data->ObjectBroad;
    delete m_data->PairFilter;

    delete m_data->JobSystem;

    delete m_data->TempAllocator;

    Alloc->Destroy(m_data->RuntimeBindings);

    JPH::UnregisterTypes();

    Alloc->Destroy(m_data);

    delete JPH::Factory::sInstance;
    JPH::Factory::sInstance = nullptr;

    const uint32_t allocatorChainSize = m_allocatorChain->Size();
    for (uint32_t i = 0; i < allocatorChainSize; ++i)
    {
        IcarianCore::Allocator* alloc = (*m_allocatorChain)[allocatorChainSize - i - 1];
        m_smallAllocator->Destroy(alloc);
    }

    m_smallAllocator->Destroy(m_allocatorChain);

    IcarianCore::MallocAllocator::Instance->Destroy(m_largeAllocator);
    IcarianCore::MallocAllocator::Instance->Destroy(m_smallAllocator);
}

IcarianCore::Allocator* PhysicsEngine::GetAllocator() const
{
    return Alloc;
}

bool PhysicsEngine::CanObjectLayersCollide(uint32_t a_lhs, uint32_t a_rhs) const
{
    IVERIFY(a_lhs < 8);
    IVERIFY(a_rhs < 8);

    return IISBITSET(m_data->ObjectLayerCollisions[a_lhs], a_rhs);
}

uint32_t PhysicsEngine::GetBodyAddr(JPH::uint a_joltIndex)
{
    const IcarianCore::SharedThreadGuard g = IcarianCore::SharedThreadGuard(m_bodyMapLock);

    if (m_data->BodyMap.Exists(a_joltIndex))
    {
        return m_data->BodyMap[a_joltIndex];
    }

    return -1;
}

static void TransformObject(uint32_t a_transformAddr, const glm::vec3& a_translation, const glm::quat& a_rotation)
{
    TransformBuffer buffer = ObjectManager::GetTransformBuffer(a_transformAddr);

    glm::vec3 iTranslation = glm::vec3(0.0f);
    glm::quat iRotation = glm::identity<glm::quat>();

    if (buffer.ParentAddr != uint32_t(-1))
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

void PhysicsEngine::Update(double a_delta, float a_timeScale)
{
    if (m_trackerAllocator != nullptr)
    {
        const uint64_t size = m_trackerAllocator->GetMemoryUsage();

        Profiler::PushMemoryFrame(ProfilerMemoryFrame_Physics, size);
    }

    if (a_timeScale <= 0.0f)
    {
        return;
    }

    {
        PROFILESTACK("Physics Simulation");

        m_data->FixedTimeTimer += a_delta;

        // Done some digging and found a note about stability above 60hz needing to be done in steps
        constexpr double JoltStepMagicNumber = 1.0 / 60.0;

        const int steps = (int)((m_data->FixedTimeStep * a_timeScale) / JoltStepMagicNumber + 1);
        const float timeStep = (float)(m_data->FixedTimeStep * a_timeScale);

        while (m_data->FixedTimeTimer >= m_data->FixedTimeStep)
        {
            m_data->FixedTimeTimer -= m_data->FixedTimeStep;
            m_data->FixedTimePassed += m_data->FixedTimeStep;

            {
                PROFILESTACK("Fixed Update");

                void* args[] =
                {
                    &m_data->FixedTimeStep,
                    &m_data->FixedTimePassed
                };

                m_data->FixedUpdateFunction->Exec(args);
            }

            const JPH::Vec3 gravity = m_data->PhysicsSystem->GetGravity();

            const JPH::DefaultBroadPhaseLayerFilter broadFilter = m_data->PhysicsSystem->GetDefaultBroadPhaseLayerFilter(0);
            const JPH::DefaultObjectLayerFilter objectFilter = m_data->PhysicsSystem->GetDefaultLayerFilter(0);

            const IcarianCore::Array<JPH::CharacterVirtual*> characters = m_data->Characters.ToActiveArray(Alloc);
            for (JPH::CharacterVirtual* c : characters)
            {
                PROFILESTACK("Character Update");

                const JPH::Vec3 up = c->GetUp();

                const JPH::CharacterVirtual::ExtendedUpdateSettings updateSettings =
                {
                    .mStickToFloorStepDown = -up * 0.2f,
                    .mWalkStairsStepUp = up * 0.2f
                };

                c->ExtendedUpdate
                (
                    timeStep,
                    gravity,
                    updateSettings,
                    broadFilter,
                    objectFilter,
                    { },
                    { },
                    *m_data->TempAllocator
                );
            }

            PROFILESTACK("Physics Step");

            m_data->PhysicsSystem->Update(timeStep, steps, m_data->TempAllocator, m_data->JobSystem);
        }
    }

    {
        PROFILESTACK("Physics Sync");

        {
            PROFILESTACK("Physics Bodies");

            const IcarianCore::Array<JPH::BodyID> bodies = m_data->ActivationListener->ToBodies(Alloc);
            const IcarianCore::SharedThreadGuard g = IcarianCore::SharedThreadGuard(m_bodyMapLock);

            // Should not need but doing just incase for good practice as it multithreaded app
            // FFS something in WIN32 means that I can no longer call this interface without a compiler error bodyInterface it is
            const JPH::BodyLockInterfaceLocking& bodyinterface = m_data->PhysicsSystem->GetBodyLockInterface();

            // Need to sync the physics transform to the transform
            for (const JPH::BodyID id : bodies)
            {
                const PhysicsInterfaceReadLock lock = PhysicsInterfaceReadLock(id, bodyinterface);

                const JPH::Body* body = bodyinterface.TryGetBody(id);
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

                const JPH::uint32 bodyIndex = id.GetIndex();
                if (!m_data->BodyMap.Exists(bodyIndex))
                {
                    continue;
                }

                const BodyBinding binding = m_data->BodyBindings[bodyIndex];

                const bool valid = binding.TransformAddr != uint32_t(-1);
                if (!valid)
                {
                    continue;
                }

                // TODO: Should probably account for acceleration and apply the same to rotations
                const JPH::RVec3 jVelocity = body->GetLinearVelocity();
                const JPH::RVec3 jTranslation = jPos + jVelocity * m_data->FixedTimeTimer;
                const JPH::Quat jRotation = body->GetRotation();

                const glm::vec3 translation = glm::vec3(jTranslation.GetX(), jTranslation.GetY(), jTranslation.GetZ());
                const glm::quat rotation = glm::quat(jRotation.GetX(), jRotation.GetY(), jRotation.GetZ(), jRotation.GetW());

                TransformObject(binding.TransformAddr, translation, rotation);
            }
        }

        {
            PROFILESTACK("Characters");

            const IcarianCore::Array<JPH::CharacterVirtual*> characters = m_data->Characters.ToActiveArray(Alloc);
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

    {
        PROFILESTACK("Physics Memory");

        m_smallAllocator->TrimBlocks();
        m_largeAllocator->TrimBlocks();
    }
}

// MIT License
//
// Copyright (c) 2026 River Govers
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
