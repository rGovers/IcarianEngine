#pragma once

#include <Jolt/Jolt.h>

#include <cstdint>
#include <Jolt/Core/Color.h>
#include <Jolt/Core/FixedSizeFreeList.h>
#include <Jolt/Core/JobSystemWithBarrier.h>

#include "ThreadPool.h"

class IcPhysicsJobSystem : public JPH::JobSystemWithBarrier
{
private:
    static constexpr uint32_t MaxJobs = 1024;

    JPH::FixedSizeFreeList<Job> m_jobs;

    class PhysicsJob : public ThreadJob
    {
    private:
        Job* m_job;

    protected:

    public:
        PhysicsJob(Job* a_job) : ThreadJob(JobPriority_EngineHigh)
        {
            m_job = a_job;

            m_job->AddRef();
        }
        virtual ~PhysicsJob()
        {
            m_job->Release();
        }

        virtual void Execute()
        {
            m_job->Execute();
        }
    };

protected:
    virtual void QueueJob(Job* a_job);
    virtual void QueueJobs(Job** a_jobs, uint a_numJobs);
    virtual void FreeJob(Job* a_job);

public:
    IcPhysicsJobSystem();
    virtual ~IcPhysicsJobSystem();

    virtual int GetMaxConcurrency() const;

    virtual JobHandle CreateJob(const char* a_name, JPH::ColorArg a_color, const JobFunction& a_jobFunction, uint32_t a_numDependencies);
};