#include "Physics/PhysicsEngine.h"

#include <cstdarg>
#include <cstdio>
#include <Jolt/Core/Factory.h>
#include <Jolt/Core/Memory.h>
#include <Jolt/Core/IssueReporting.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Physics/PhysicsSettings.h>
#include <Jolt/RegisterTypes.h>

#include <thread>

#include "Flare/IcarianAssert.h"
#include "Trace.h"
 
static void TraceImpl(const char* inFMT, ...)
{
    va_list list;
    va_start(list, inFMT);
    char buffer[1024];
    vsnprintf(buffer, sizeof(buffer), inFMT, list);
    va_end(list);

    TRACE(buffer);
}

static constexpr JPH::BroadPhaseLayer BroadphaseNonMoving = JPH::BroadPhaseLayer(0);
static constexpr JPH::BroadPhaseLayer BroadphaseMoving = JPH::BroadPhaseLayer(1);
static constexpr JPH::ObjectLayer LayerNonMoving = JPH::ObjectLayer(0);
static constexpr JPH::ObjectLayer LayerMoving = JPH::ObjectLayer(1);

class IcBPLayerInterface : public JPH::BroadPhaseLayerInterface
{
private:
    JPH::BroadPhaseLayer m_layers[2];

protected:

public:
    IcBPLayerInterface()
    {
        m_layers[LayerNonMoving] = BroadphaseNonMoving;
        m_layers[LayerMoving] = BroadphaseMoving; 
    }

    virtual uint GetNumBroadPhaseLayers() const 
    {
        return 2;
    }
    virtual JPH::BroadPhaseLayer GetBroadPhaseLayer(JPH::ObjectLayer a_layer) const
    {
        ICARIAN_ASSERT(a_layer < 2);
        return m_layers[a_layer];
    }

#if defined(JPH_EXTERNAL_PROFILE) || defined(JPH_PROFILE_ENABLED)
	virtual const char* GetBroadPhaseLayerName(JPH::BroadPhaseLayer a_layer) const
    {
        switch ((JPH::BroadPhaseLayer::Type)a_layer)
        {
        case (JPH::BroadPhaseLayer::Type)BroadphaseNonMoving:
        {
            return "NON_MOVING";
        }
        case (JPH::BroadPhaseLayer::Type)BroadphaseMoving:
        {
            return "MOVING";
        }
        }

        ICARIAN_ASSERT(0);
    }
#endif 
};

class IcObjectVsBroadPhaseLayerFilter : public JPH::ObjectVsBroadPhaseLayerFilter
{
private:

protected:

public:
    IcObjectVsBroadPhaseLayerFilter()
    {

    }
    virtual ~IcObjectVsBroadPhaseLayerFilter() 
    {

    }

    virtual bool ShouldCollide(JPH::ObjectLayer a_lhs, JPH::ObjectLayer a_rhs)
    {
        switch (a_lhs)
        {
        case LayerNonMoving:
        {
            return a_rhs == LayerMoving;
        }
        case LayerMoving:
        {
            return true;
        }
        }

        ICARIAN_ASSERT(0);

        return false;
    }
};

class IcObjectLayerPairFilter : public JPH::ObjectLayerPairFilter
{
private:

protected:

public:
    IcObjectLayerPairFilter()
    {

    }
    virtual ~IcObjectLayerPairFilter()
    {

    }

    virtual bool ShouldCollide(JPH::ObjectLayer a_lhs, JPH::ObjectLayer a_rhs) const
    {
        switch (a_lhs)
        {
        case LayerNonMoving:
        {
            return a_rhs == LayerMoving;
        }
        case LayerMoving:
        {
            return true;
        }
        }

        ICARIAN_ASSERT(0);

        return false;
    }
};

PhysicsEngine::PhysicsEngine()
{
    JPH::RegisterDefaultAllocator();

    JPH::Trace = TraceImpl;

    JPH::Factory::sInstance = new JPH::Factory();

    JPH::RegisterTypes();

    JPH::JobSystemThreadPool pool = JPH::JobSystemThreadPool(JPH::cMaxPhysicsJobs, JPH::cMaxPhysicsBarriers, glm::max(1U, std::thread::hardware_concurrency() / 3));

    m_broadPhase = new IcBPLayerInterface();
    m_objectBroad = new IcObjectVsBroadPhaseLayerFilter();
    m_pairFilter = new IcObjectLayerPairFilter();

    m_physicsSystem.Init(MaxBodies, 0, MaxBodies, MaxContactConstraints, *m_broadPhase, *m_objectBroad, *m_pairFilter);
}
PhysicsEngine::~PhysicsEngine()
{
    delete m_pairFilter;
    delete m_objectBroad;
    delete m_broadPhase;

    JPH::UnregisterTypes();

    delete JPH::Factory::sInstance;
    JPH::Factory::sInstance = nullptr;
}
